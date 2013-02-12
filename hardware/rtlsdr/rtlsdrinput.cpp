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
#include "rtlsdrinput.h"
#include "rtlsdrthread.h"
#include "rtlsdrgui.h"

RTLSDRInput::Settings::Settings() :
	centerFrequency(100000000),
	gain(0),
	decimation(0)
{
}

QString RTLSDRInput::Settings::serialize() const
{
	return QString("rtlsdr:a:%1:%2:%3")
		.arg(centerFrequency)
		.arg(gain)
		.arg(decimation);
}

bool RTLSDRInput::Settings::deserialize(const QString& settings)
{
	QStringList list = settings.split(":");
	if(list.size() < 2)
		return false;
	if(list[0] != "rtlsdr")
		return false;

	if(list[1] == "a") {
		bool ok;
		if(list.size() != 5)
			return false;
		centerFrequency = list[2].toLongLong(&ok);
		if(!ok)
			return false;
		gain = list[3].toInt(&ok);
		if(!ok)
			return false;
		decimation = list[4].toInt(&ok);
		if(!ok)
			return false;
		return true;
	} else {
		return false;
	}
}

RTLSDRInput::RTLSDRInput(MessageQueue* msgQueueToGUI) :
	SampleSource(msgQueueToGUI),
	m_settings(),
	m_dev(NULL),
	m_rtlSDRThread(NULL),
	m_deviceDescription()
{
}

RTLSDRInput::~RTLSDRInput()
{
	stopInput();
}

bool RTLSDRInput::startInput(int device)
{
	QMutexLocker mutexLocker(&m_mutex);

	if(m_dev != NULL)
		stopInput();

	char vendor[256];
	char product[256];
	char serial[256];
	int res;
	int numberOfGains;

	if(!m_sampleFifo.setSize(524288)) {
		qCritical("Could not allocate SampleFifo");
		return false;
	}

	if((res = rtlsdr_open(&m_dev, device)) < 0) {
		qCritical("could not open RTLSDR #%d: %s", device, strerror(errno));
		return false;
	}

	vendor[0] = '\0';
	product[0] = '\0';
	serial[0] = '\0';
	if((res = rtlsdr_get_usb_strings(m_dev, vendor, product, serial)) < 0) {
		qCritical("error accessing USB device");
		goto failed;
	}
	qDebug("RTLSDRInput open: %s %s, SN: %s", vendor, product, serial);
	m_deviceDescription = QString("%1 (SN %2)").arg(product).arg(serial);

	if((res = rtlsdr_set_sample_rate(m_dev, 2000000)) < 0) {
		qCritical("could not set sample rate: %s", strerror(errno));
		goto failed;
	}

	if((res = rtlsdr_set_tuner_gain_mode(m_dev, 1)) < 0) {
		qCritical("error setting tuner gain mode");
		goto failed;
	}
	if((res = rtlsdr_set_agc_mode(m_dev, 0)) < 0) {
		qCritical("error setting agc mode");
		goto failed;
	}

	numberOfGains = rtlsdr_get_tuner_gains(m_dev, NULL);
	if(numberOfGains < 0) {
		qCritical("error getting number of gain values supported");
		goto failed;
	}
	m_gains.resize(numberOfGains);
	if(rtlsdr_get_tuner_gains(m_dev, &m_gains[0]) < 0) {
		qCritical("error getting gain values");
		goto failed;
	}
	if((res = rtlsdr_reset_buffer(m_dev)) < 0) {
		qCritical("could not reset USB EP buffers: %s", strerror(errno));
		goto failed;
	}

	if((m_rtlSDRThread = new RTLSDRThread(m_dev, &m_sampleFifo)) == NULL) {
		qFatal("out of memory");
		goto failed;
	}
	m_rtlSDRThread->startWork();

	mutexLocker.unlock();
	applySettings(m_settings, true);

	qDebug("RTLSDRInput: start");
	DSPCmdGUIInfoRTLSDR::create(m_gains)->submit(m_guiMessageQueue);

	return true;

failed:
	stopInput();
	return false;
}

void RTLSDRInput::stopInput()
{
	QMutexLocker mutexLocker(&m_mutex);

	if(m_rtlSDRThread != NULL) {
		m_rtlSDRThread->stopWork();
		delete m_rtlSDRThread;
		m_rtlSDRThread = NULL;
	}
	if(m_dev != NULL) {
		rtlsdr_close(m_dev);
		m_dev = NULL;
	}
	m_deviceDescription.clear();
}

const QString& RTLSDRInput::getDeviceDescription() const
{
	return m_deviceDescription;
}

int RTLSDRInput::getSampleRate() const
{
	return 2000000 / (1 << m_settings.decimation);
}

quint64 RTLSDRInput::getCenterFrequency() const
{
	return m_settings.centerFrequency;
}

SampleSourceGUI* RTLSDRInput::createGUI(MessageQueue* msgQueueToEngine, QWidget* parent) const
{
	return new RTLSDRGui(msgQueueToEngine, parent);
}

void RTLSDRInput::handleGUIMessage(DSPCmdGUIToSource* cmd)
{
	if(cmd->sourceType() != DSPCmdConfigureSourceRTLSDR::SourceType)
		return;
	if(!applySettings(((DSPCmdConfigureSourceRTLSDR*)cmd)->getSettings(), false))
		qDebug("RTLSDR config error");
}

bool RTLSDRInput::applySettings(const Settings& settings, bool force)
{
	QMutexLocker mutexLocker(&m_mutex);

	if((m_settings.centerFrequency != settings.centerFrequency) || force) {
		m_settings.centerFrequency = settings.centerFrequency;
		if(m_dev != NULL) {
			if(rtlsdr_set_center_freq(m_dev, m_settings.centerFrequency) != 0)
				qDebug("osmosdr_set_center_freq(%lld) failed", m_settings.centerFrequency);
		}
	}
	if((m_settings.gain != settings.gain) || force) {
		m_settings.gain = settings.gain;
		if(m_dev != NULL) {
			if(rtlsdr_set_tuner_gain(m_dev, m_settings.gain) != 0)
				qDebug("rtlsdr_set_tuner_gain() failed");
		}
	}
	if((m_settings.decimation != settings.decimation) || force) {
		m_settings.decimation = settings.decimation;
		if(m_dev != NULL)
			m_rtlSDRThread->setDecimation(m_settings.decimation);
	}
	return true;
}

int DSPCmdConfigureSourceRTLSDR::sourceType() const
{
	return SourceType;
}

int DSPCmdGUIInfoRTLSDR::sourceType() const
{
	return SourceType;
}
