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

#ifndef INCLUDE_RTLSDRINPUT_H
#define INCLUDE_RTLSDRINPUT_H

#include "../samplesource.h"
#include <rtl-sdr.h>
#include <QString>

class RTLSDRThread;

class RTLSDRInput : public SampleSource {
public:
	struct Settings {
		quint64 centerFrequency;
		int gain;
		int decimation;

		Settings();
		QString serialize() const;
		bool deserialize(const QString& settings);
	};

	RTLSDRInput(MessageQueue* msgQueueToGUI);
	~RTLSDRInput();

	bool startInput(int device);
	void stopInput();

	const QString& getDeviceDescription() const;
	int getSampleRate() const;
	quint64 getCenterFrequency() const;

	SampleSourceGUI* createGUI(MessageQueue* msgQueueToEngine, QWidget* parent = NULL) const;

	void handleGUIMessage(DSPCmdGUIToSource* cmd);

private:
	QMutex m_mutex;
	Settings m_settings;
	rtlsdr_dev_t* m_dev;
	RTLSDRThread* m_rtlSDRThread;
	QString m_deviceDescription;
	std::vector<int> m_gains;

	bool applySettings(const Settings& settings, bool force);
};

class DSPCmdConfigureSourceRTLSDR : public DSPCmdGUIToSource {
public:
	enum {
		SourceType = 2
	};

	int sourceType() const;
	const RTLSDRInput::Settings& getSettings() const { return m_settings; }

	static DSPCmdConfigureSourceRTLSDR* create(const RTLSDRInput::Settings& settings)
	{
		return new DSPCmdConfigureSourceRTLSDR(settings);
	}

protected:
	RTLSDRInput::Settings m_settings;

	DSPCmdConfigureSourceRTLSDR(const RTLSDRInput::Settings& settings) :
		m_settings(settings)
	{ }
};

class DSPCmdGUIInfoRTLSDR : public DSPCmdSourceToGUI {
public:
	enum {
		SourceType = 2
	};
	int sourceType() const;
	const std::vector<int>& getGains() const { return m_gains; }

	static DSPCmdGUIInfoRTLSDR* create(const std::vector<int>& gains)
	{
		return new DSPCmdGUIInfoRTLSDR(gains);
	}

protected:
	std::vector<int> m_gains;

	DSPCmdGUIInfoRTLSDR(const std::vector<int>& gains) :
		m_gains(gains)
	{ }
};

#endif // INCLUDE_RTLSDRINPUT_H
