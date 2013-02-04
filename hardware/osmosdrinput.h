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

#ifndef INCLUDE_OSMOSDRINPUT_H
#define INCLUDE_OSMOSDRINPUT_H

#include "samplesource.h"
#include <osmosdr.h>
#include <QString>

class OsmoSDRThread;

class OsmoSDRInput : public SampleSource {
public:
	struct Settings {
		quint64 centerFrequency;
		bool swapIQ;
		int decimation;
		int lnaGain;
		int mixerGain;
		int mixerEnhancement;
		int if1gain;
		int if2gain;
		int if3gain;
		int if4gain;
		int if5gain;
		int if6gain;
		int opAmpI1;
		int opAmpI2;
		int opAmpQ1;
		int opAmpQ2;
		int dcOfsI;
		int dcOfsQ;

		Settings();
		QString serialize() const;
		bool deserialize(const QString& settings);
	};

	OsmoSDRInput();
	~OsmoSDRInput();

	bool startInput(int device);
	void stopInput();

	const QString& getDeviceDescription() const;
	int getSampleRate() const;
	quint64 getCenterFrequency() const;

	SampleSourceGUI* createGUI(MessageQueue* msgQueue, QWidget* parent = NULL) const;

	void handleConfiguration(DSPCmdConfigureSource* cmd);

private:
	QMutex m_mutex;
	Settings m_settings;
	osmosdr_dev_t* m_dev;
	OsmoSDRThread* m_osmoSDRThread;
	QString m_deviceDescription;

	bool applySettings(const Settings& settings, bool force);
};

class DSPCmdConfigureSourceOsmoSDR : public DSPCmdConfigureSource {
public:
	enum {
		SourceType = 1
	};

	int sourceType() const;
	const OsmoSDRInput::Settings& getSettings() const { return m_settings; }

	static DSPCmdConfigureSourceOsmoSDR* create(const OsmoSDRInput::Settings& settings)
	{
		return new DSPCmdConfigureSourceOsmoSDR(settings);
	}

protected:
	OsmoSDRInput::Settings m_settings;

	DSPCmdConfigureSourceOsmoSDR(const OsmoSDRInput::Settings& settings) : m_settings(settings) { }
};

#endif // INCLUDE_OSMOSDRINPUT_H
