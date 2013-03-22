///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
// Copyright (C) 2013 by Dimitri Stolnikov <horiz0n@gmx.net>                     //
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
#include <boost/foreach.hpp>
#include "util/simpleserializer.h"
#include "gnuradioinput.h"
#include "gnuradiothread.h"
#include "gnuradiogui.h"

MessageRegistrator GNURadioInput::MsgConfigureGNURadio::ID("MsgConfigureGNURadio");
MessageRegistrator GNURadioInput::MsgReportGNURadio::ID("MsgReportGNURadio");

GNURadioInput::Settings::Settings() :
	m_args(""),
	m_sampleRate(2e6),
	m_freqCorr(0),
	m_rfGain(10),
	m_ifGain(15),
	m_antenna("")
{
}

void GNURadioInput::Settings::resetToDefaults()
{
	m_args = "";
	m_sampleRate = 2e6;
	m_freqCorr = 0;
	m_rfGain = 10;
	m_ifGain = 15;
	m_antenna = "";
}

QByteArray GNURadioInput::Settings::serialize() const
{
	SimpleSerializer s(1);
	s.writeString(1, m_args);
	s.writeDouble(2, m_sampleRate);
	s.writeDouble(3, m_freqCorr);
	s.writeDouble(4, m_rfGain);
	s.writeDouble(5, m_ifGain);
	s.writeString(6, m_antenna);
	return s.final();
}

bool GNURadioInput::Settings::deserialize(const QByteArray& data)
{
	SimpleDeserializer d(data);

	if(!d.isValid()) {
		resetToDefaults();
		return false;
	}

	if(d.getVersion() == 1) {
		d.readString(1, &m_args, "");
		d.readDouble(2, &m_sampleRate, 2e6);
		d.readDouble(3, &m_freqCorr, 0);
		d.readDouble(4, &m_rfGain, 10);
		d.readDouble(5, &m_ifGain, 15);
		d.readString(6, &m_antenna, "");
		return true;
	} else {
		resetToDefaults();
		return false;
	}
}

GNURadioInput::GNURadioInput(MessageQueue* msgQueueToGUI) :
	SampleSource(msgQueueToGUI),
	m_settings(),
	m_GnuradioThread(NULL),
	m_deviceDescription()
{
}

GNURadioInput::~GNURadioInput()
{
	stopInput();
}

bool GNURadioInput::startInput(int device)
{
	QMutexLocker mutexLocker(&m_mutex);

	if(m_GnuradioThread != NULL)
		stopInput();

	if(!m_sampleFifo.setSize(524288)) {
		qCritical("Could not allocate SampleFifo");
		return false;
	}

	m_deviceDescription = m_settings.m_args;

	// pass device arguments from the gui
	m_GnuradioThread = new GnuradioThread(m_settings.m_args, &m_sampleFifo);
	if(m_GnuradioThread == NULL) {
		qFatal("out of memory");
		goto failed;
	}
	m_GnuradioThread->startWork();

	mutexLocker.unlock();
	applySettings(m_generalSettings, m_settings, true);

	if(m_GnuradioThread != NULL) {
		osmosdr_source_c_sptr radio = m_GnuradioThread->radio();

		m_sampRates = radio->get_sample_rates().values();
		m_rfGains = radio->get_gain_range().values();
		m_ifGains = radio->get_gain_range("IF").values();

		m_antennas.clear();
		BOOST_FOREACH( std::string antenna, radio->get_antennas() )
			m_antennas.push_back( QString( antenna.c_str() ) );
	}

	qDebug("GnuradioInput: start");
	MsgReportGNURadio::create(m_rfGains, m_ifGains, m_sampRates, m_antennas)->submit(m_guiMessageQueue);

	return true;

failed:
	stopInput();
	return false;
}

void GNURadioInput::stopInput()
{
	QMutexLocker mutexLocker(&m_mutex);

	if(m_GnuradioThread != NULL) {
		m_GnuradioThread->stopWork();
		delete m_GnuradioThread;
		m_GnuradioThread = NULL;
	}

	m_deviceDescription.clear();
}

const QString& GNURadioInput::getDeviceDescription() const
{
	return m_deviceDescription;
}

int GNURadioInput::getSampleRate() const
{
	return m_settings.m_sampleRate;
}

quint64 GNURadioInput::getCenterFrequency() const
{
	return m_generalSettings.m_centerFrequency;
}

bool GNURadioInput::handleMessage(Message* message)
{
	if(message->id() == MsgConfigureGNURadio::ID()) {
		MsgConfigureGNURadio* conf = (MsgConfigureGNURadio*)message;
		if(!applySettings(conf->getGeneralSettings(), conf->getSettings(), false))
			qDebug("Gnuradio config error");
		return true;
	} else {
		return false;
	}
}

bool GNURadioInput::applySettings(const GeneralSettings& generalSettings, const Settings& settings, bool force)
{
	QMutexLocker mutexLocker(&m_mutex);

	m_settings.m_args = settings.m_args;

	if((m_settings.m_freqCorr != settings.m_freqCorr) || force) {
		m_settings.m_freqCorr = settings.m_freqCorr;
		if(m_GnuradioThread != NULL) {
			m_GnuradioThread->radio()->set_freq_corr( m_settings.m_freqCorr );
		}
	}

	if((m_generalSettings.m_centerFrequency != generalSettings.m_centerFrequency) || force) {
		m_generalSettings.m_centerFrequency = generalSettings.m_centerFrequency;
		if(m_GnuradioThread != NULL) {
			m_GnuradioThread->radio()->set_center_freq( m_generalSettings.m_centerFrequency );
		}
	}

	if((m_settings.m_rfGain != settings.m_rfGain) || force) {
		m_settings.m_rfGain = settings.m_rfGain;
		if(m_GnuradioThread != NULL) {
			m_GnuradioThread->radio()->set_gain( m_settings.m_rfGain );
		}
	}

	if((m_settings.m_ifGain != settings.m_ifGain) || force) {
		m_settings.m_ifGain = settings.m_ifGain;
		if(m_GnuradioThread != NULL) {
			m_GnuradioThread->radio()->set_gain( m_settings.m_ifGain, "IF" );
		}
	}

	if((m_settings.m_sampleRate != settings.m_sampleRate) || force) {
		m_settings.m_sampleRate = settings.m_sampleRate;
		if(m_GnuradioThread != NULL) {
			m_GnuradioThread->radio()->set_sample_rate( m_settings.m_sampleRate );
		}
	}

	if((m_settings.m_antenna != settings.m_antenna) || force) {
		m_settings.m_antenna = settings.m_antenna;
		if(m_GnuradioThread != NULL) {
			m_GnuradioThread->radio()->set_antenna( m_settings.m_antenna.toStdString() );
		}
	}

	return true;
}
