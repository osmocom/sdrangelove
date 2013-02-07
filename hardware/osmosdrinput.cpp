///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <errno.h>
#include "osmosdrinput.h"
#include "osmosdrthread.h"
#include "osmosdrgui.h"

OsmoSDRInput::Settings::Settings() :
	centerFrequency(100000000),
	swapIQ(false),
	decimation(3),
	lnaGain(-50),
	mixerGain(40),
	mixerEnhancement(0),
	if1gain(-30),
	if2gain(0),
	if3gain(0),
	if4gain(0),
	if5gain(30),
	if6gain(30),
	opAmpI1(0),
	opAmpI2(0),
	opAmpQ1(0),
	opAmpQ2(0),
	dcOfsI(0),
	dcOfsQ(0)
{
}

QString OsmoSDRInput::Settings::serialize() const
{
	return QString("osmosdr:a:%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12:%13:%14:%15:%16:%17:%18")
		.arg(centerFrequency)
		.arg(swapIQ ? 1 : 0)
		.arg(decimation)
		.arg(lnaGain)
		.arg(mixerGain)
		.arg(mixerEnhancement)
		.arg(if1gain)
		.arg(if2gain)
		.arg(if3gain)
		.arg(if4gain)
		.arg(if5gain)
		.arg(if6gain)
		.arg(opAmpI1)
		.arg(opAmpI2)
		.arg(opAmpQ1)
		.arg(opAmpQ2)
		.arg(dcOfsI)
		.arg(dcOfsQ);
}

bool OsmoSDRInput::Settings::deserialize(const QString& settings)
{
	QStringList list = settings.split(":");
	if(list.size() < 2)
		return false;
	if(list[0] != "osmosdr")
		return false;

	if(list[1] == "a") {
		bool ok;
		if(list.size() != 20)
			return false;
		centerFrequency = list[2].toLongLong(&ok);
		if(!ok)
			return false;
		swapIQ = (list[3].toInt(&ok) != 0) ? true : false;
		if(!ok)
			return false;
		decimation = list[4].toInt(&ok);
		if(!ok)
			return false;
		lnaGain = list[5].toInt(&ok);
		if(!ok)
			return false;
		mixerGain = list[6].toInt(&ok);
		if(!ok)
			return false;
		mixerEnhancement = list[7].toInt(&ok);
		if(!ok)
			return false;
		if1gain = list[8].toInt(&ok);
		if(!ok)
			return false;
		if2gain = list[9].toInt(&ok);
		if(!ok)
			return false;
		if3gain = list[10].toInt(&ok);
		if(!ok)
			return false;
		if4gain = list[11].toInt(&ok);
		if(!ok)
			return false;
		if5gain = list[12].toInt(&ok);
		if(!ok)
			return false;
		if6gain = list[13].toInt(&ok);
		if(!ok)
			return false;
		opAmpI1 = list[14].toInt(&ok);
		if(!ok)
			return false;
		opAmpI2 = list[15].toInt(&ok);
		if(!ok)
			return false;
		opAmpQ1 = list[16].toInt(&ok);
		if(!ok)
			return false;
		opAmpQ2 = list[17].toInt(&ok);
		if(!ok)
			return false;
		dcOfsI = list[18].toInt(&ok);
		if(!ok)
			return false;
		dcOfsQ = list[19].toInt(&ok);
		if(!ok)
			return false;
		return true;
	} else {
		return false;
	}
}

OsmoSDRInput::OsmoSDRInput() :
	SampleSource(),
	m_settings(),
	m_dev(NULL),
	m_osmoSDRThread(NULL),
	m_deviceDescription()
{
}

OsmoSDRInput::~OsmoSDRInput()
{
	QMutexLocker mutexLocker(&m_mutex);
	stopInput();
}

bool OsmoSDRInput::startInput(int device)
{
	QMutexLocker mutexLocker(&m_mutex);

	if(m_dev != NULL)
		stopInput();

	char vendor[256];
	char product[256];
	char serial[256];
	int res;

	if(!m_sampleFifo.setSize(524288)) {
		qCritical("Could not allocate SampleFifo");
		return false;
	}

	if((res = osmosdr_open(&m_dev, device)) < 0) {
		qCritical("could not open OsmoSDR #%d: %s", device, strerror(errno));
		return false;
	}

	vendor[0] = '\0';
	product[0] = '\0';
	serial[0] = '\0';
	if((res = osmosdr_get_usb_strings(m_dev, vendor, product, serial)) < 0) {
		qCritical("error accessing USB device");
		goto failed;
	}
	qDebug("OsmoSDRInput open: %s %s, SN: %s", vendor, product, serial);
	m_deviceDescription = QString("%1 (SN %2)").arg(product).arg(serial);

	if((res = osmosdr_set_tuner_gain_mode(m_dev, 1)) < 0) {
		qCritical("error setting tuner gain mode");
		goto failed;
	}

	if((res = osmosdr_reset_buffer(m_dev)) < 0) {
		qCritical("could not reset USB EP buffers: %s", strerror(errno));
		goto failed;
	}

	if((m_osmoSDRThread = new OsmoSDRThread(m_dev, &m_sampleFifo)) == NULL) {
		qFatal("out of memory");
		goto failed;
	}
	m_osmoSDRThread->start();

	mutexLocker.unlock();
	applySettings(m_settings, true);

	qDebug("OsmoSDRInput: start");

	return true;

failed:
	stopInput();
	return false;
}

void OsmoSDRInput::stopInput()
{
	QMutexLocker mutexLocker(&m_mutex);

	if(m_osmoSDRThread != NULL) {
		m_osmoSDRThread->stop();
		delete m_osmoSDRThread;
		m_osmoSDRThread = NULL;
	}
	if(m_dev != NULL) {
		osmosdr_close(m_dev);
		m_dev = NULL;
	}
	m_deviceDescription.clear();
}

const QString& OsmoSDRInput::getDeviceDescription() const
{
	return m_deviceDescription;
}

int OsmoSDRInput::getSampleRate() const
{
	return 4000000 / (1 << m_settings.decimation);
}

quint64 OsmoSDRInput::getCenterFrequency() const
{
	return m_settings.centerFrequency;
}

SampleSourceGUI* OsmoSDRInput::createGUI(MessageQueue* msgQueue, QWidget* parent) const
{
	return new OsmoSDRGui(msgQueue, parent);
}

void OsmoSDRInput::handleConfiguration(DSPCmdConfigureSource* cmd)
{
	if(cmd->sourceType() != DSPCmdConfigureSourceOsmoSDR::SourceType)
		return;
	if(!applySettings(((DSPCmdConfigureSourceOsmoSDR*)cmd)->getSettings(), false))
		qDebug("OsmoSDR config error");
}

bool OsmoSDRInput::applySettings(const Settings& settings, bool force)
{
	QMutexLocker mutexLocker(&m_mutex);

	if((m_settings.centerFrequency != settings.centerFrequency) || force) {
		m_settings.centerFrequency = settings.centerFrequency;
		if(m_dev != NULL) {
			if(osmosdr_set_center_freq(m_dev, m_settings.centerFrequency) != 0)
				qDebug("osmosdr_set_center_freq(%lld) failed", m_settings.centerFrequency);
		}
	}

	if((m_settings.swapIQ != settings.swapIQ) || force) {
		m_settings.swapIQ = settings.swapIQ;
		if(m_dev != NULL) {
			if(osmosdr_set_fpga_iq_swap(m_dev, m_settings.swapIQ ? 1 : 0) == 0)
				qDebug("osmosdr_set_fpga_iq_swap() failed");
		}
	}

	if((m_settings.decimation != settings.decimation) || force) {
		m_settings.decimation = settings.decimation;
		if(m_dev != NULL) {
			if(!osmosdr_set_fpga_decimation(m_dev, m_settings.decimation))
				qDebug("osmosdr_set_fpga_decimation() failed");
		}
	}

	if((m_settings.lnaGain != settings.lnaGain) || force) {
		m_settings.lnaGain = settings.lnaGain;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_lna_gain(m_dev, m_settings.lnaGain))
				qDebug("osmosdr_set_tuner_lna_gain() failed");
		}
	}

	if((m_settings.mixerGain != settings.mixerGain) || force) {
		m_settings.mixerGain = settings.mixerGain;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_mixer_gain(m_dev, m_settings.mixerGain))
				qDebug("osmosdr_set_tuner_mixer_gain() failed");
		}
	}

	if((m_settings.mixerEnhancement != settings.mixerEnhancement) || force) {
		m_settings.mixerEnhancement = settings.mixerEnhancement;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_mixer_enh(m_dev, m_settings.mixerEnhancement))
				qDebug("osmosdr_set_tuner_mixer_enh() failed");
		}
	}

	if((m_settings.if1gain != settings.if1gain) || force) {
		m_settings.if1gain = settings.if1gain;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_if_gain(m_dev, 1, m_settings.if1gain))
				qDebug("osmosdr_set_tuner_if_gain(1) failed");
		}
	}

	if((m_settings.if2gain != settings.if2gain) || force) {
		m_settings.if2gain = settings.if2gain;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_if_gain(m_dev, 2, m_settings.if2gain))
				qDebug("osmosdr_set_tuner_if_gain(2) failed");
		}
	}

	if((m_settings.if3gain != settings.if3gain) || force) {
		m_settings.if3gain = settings.if3gain;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_if_gain(m_dev, 3, m_settings.if3gain))
				qDebug("osmosdr_set_tuner_if_gain(3) failed");
		}
	}

	if((m_settings.if4gain != settings.if4gain) || force) {
		m_settings.if4gain = settings.if4gain;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_if_gain(m_dev, 4, m_settings.if4gain))
				qDebug("osmosdr_set_tuner_if_gain(4) failed");
		}
	}

	if((m_settings.if5gain != settings.if5gain) || force) {
		m_settings.if5gain = settings.if5gain;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_if_gain(m_dev, 5, m_settings.if5gain))
				qDebug("osmosdr_set_tuner_if_gain(5) failed");
		}
	}

	if((m_settings.if6gain != settings.if6gain) || force) {
		m_settings.if6gain = settings.if6gain;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_if_gain(m_dev, 6, m_settings.if6gain))
				qDebug("osmosdr_set_tuner_if_gain(6) failed");
		}
	}

	if((m_settings.opAmpI1 != settings.opAmpI1) || (m_settings.opAmpI2 != settings.opAmpI2) ||
	   (m_settings.opAmpQ1 != settings.opAmpQ1) || (m_settings.opAmpQ2 != settings.opAmpQ2) ||
	   force) {
		m_settings.opAmpI1 = settings.opAmpI1;
		m_settings.opAmpI2 = settings.opAmpI2;
		m_settings.opAmpQ1 = settings.opAmpQ1;
		m_settings.opAmpQ2 = settings.opAmpQ2;
		if(m_dev != NULL) {
			if(!osmosdr_set_iq_amp(m_dev, m_settings.opAmpI1, m_settings.opAmpI2, m_settings.opAmpQ1, m_settings.opAmpQ2))
				qDebug("osmosdr_set_iq_amp(1) failed");
		}
	}

	if((m_settings.dcOfsI != settings.dcOfsI) || (m_settings.dcOfsQ != settings.dcOfsQ) ||
	   force) {
		m_settings.dcOfsI = settings.dcOfsI;
		m_settings.dcOfsQ = settings.dcOfsQ;
		if(m_dev != NULL) {
			if(!osmosdr_set_tuner_dc_offset(m_dev, m_settings.dcOfsI, m_settings.dcOfsQ))
				qDebug("osmosdr_set_tuner_dc_offset() failed");
		}
	}

	return true;
}

int DSPCmdConfigureSourceOsmoSDR::sourceType() const
{
	return SourceType;
}
