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

OsmoSDRInput::OsmoSDRInput(SampleFifo* sampleFifo) :
	SampleSource(sampleFifo),
	m_dev(NULL),
	m_osmoSDRThread(NULL)
{
}

OsmoSDRInput::~OsmoSDRInput()
{
	stopInput();
}

bool OsmoSDRInput::startInput(int device, int rate)
{
	if(m_dev != NULL)
		stopInput();

	char vendor[256];
	char product[256];
	char serial[256];
	int res;

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

	if((res = osmosdr_set_sample_rate(m_dev, rate)) < 0) {
		qCritical("error setting sample rate");
		goto failed;
	}
	if((res = osmosdr_set_tuner_gain_mode(m_dev, 1)) < 0) {
		qCritical("error setting tuner gain mode");
		goto failed;
	}

	if((res = osmosdr_reset_buffer(m_dev)) < 0) {
		qCritical("could not reset USB EP buffers: %s", strerror(errno));
		goto failed;
	}

	if((m_osmoSDRThread = new OsmoSDRThread(m_dev, m_sampleFifo)) == NULL) {
		qFatal("out of memory");
		goto failed;
	}
	m_osmoSDRThread->start();

	qDebug("OsmoSDRInput: start");

	return true;

failed:
	stopInput();
	return false;
}

void OsmoSDRInput::stopInput()
{
	if(m_osmoSDRThread != NULL) {
		m_osmoSDRThread->stop();
		delete m_osmoSDRThread;
		m_osmoSDRThread = NULL;
	}
	if(m_dev != NULL) {
		osmosdr_close(m_dev);
		m_dev = NULL;
	}
}

bool OsmoSDRInput::setCenterFrequency(qint64 freq)
{
	if(m_dev == NULL)
		return false;
	return osmosdr_set_center_freq(m_dev, freq) != 0;
}

bool OsmoSDRInput::setIQSwap(bool sw)
{
	if(m_dev == NULL)
		return false;
	return osmosdr_set_fpga_iq_swap(m_dev, sw ? 1 : 0);
}

bool OsmoSDRInput::setDecimation(int dec)
{
	if(m_dev == NULL)
		return false;
	return osmosdr_set_fpga_decimation(m_dev, dec);
}

bool OsmoSDRInput::setE4000LNAGain(int gain)
{
	if(m_dev == NULL)
		return false;
	return osmosdr_set_tuner_lna_gain(m_dev, gain);
}

bool OsmoSDRInput::setE4000MixerGain(int gain)
{
	if(m_dev == NULL)
		return false;
	return osmosdr_set_tuner_mixer_gain(m_dev, gain);
}

bool OsmoSDRInput::setE4000MixerEnh(int gain)
{
	if(m_dev == NULL)
		return false;
	return osmosdr_set_tuner_mixer_enh(m_dev, gain);
}

bool OsmoSDRInput::setE4000ifStageGain(int stage, int gain)
{
	if(m_dev == NULL)
		return false;
	return osmosdr_set_tuner_if_gain(m_dev, stage, gain);
}
