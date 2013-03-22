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

#ifndef INCLUDE_GNURADIOINPUT_H
#define INCLUDE_GNURADIOINPUT_H

#include "dsp/samplesource/samplesource.h"
#include <QString>

class GnuradioThread;

class GNURadioInput : public SampleSource {
public:
	struct Settings {
		QString m_args;
		double m_sampleRate;
		double m_freqCorr;
		double m_rfGain;
		double m_ifGain;
		QString m_antenna;

		Settings();
		void resetToDefaults();
		QByteArray serialize() const;
		bool deserialize(const QByteArray& data);
	};

	class MsgConfigureGNURadio : public Message {
	public:
		static MessageRegistrator ID;

		const GeneralSettings& getGeneralSettings() const { return m_generalSettings; }
		const Settings& getSettings() const { return m_settings; }

		static MsgConfigureGNURadio* create(const GeneralSettings& generalSettings, const Settings& settings)
		{
			return new MsgConfigureGNURadio(generalSettings, settings);
		}

	protected:
		GeneralSettings m_generalSettings;
		Settings m_settings;

		MsgConfigureGNURadio(const GeneralSettings& generalSettings, const Settings& settings) :
			Message(ID()),
			m_generalSettings(generalSettings),
			m_settings(settings)
		{ }
	};

	class MsgReportGNURadio : public Message {
	public:
		static MessageRegistrator ID;

		const std::vector<double>& getRfGains() const { return m_rfGains; }
		const std::vector<double>& getIfGains() const { return m_ifGains; }
		const std::vector<double>& getSampRates() const { return m_sampRates; }
		const std::vector<QString>& getAntennas() const { return m_antennas; }

		static MsgReportGNURadio* create(const std::vector<double>& rfGains,
										 const std::vector<double>& ifGains,
										 const std::vector<double>& sampRates,
										 const std::vector<QString>& antennas)
		{
			return new MsgReportGNURadio(rfGains, ifGains, sampRates, antennas);
		}

	protected:
		std::vector<double> m_rfGains;
		std::vector<double> m_ifGains;
		std::vector<double> m_sampRates;
		std::vector<QString> m_antennas;

		MsgReportGNURadio(const std::vector<double>& rfGains,
						  const std::vector<double>& ifGains,
						  const std::vector<double>& sampRates,
						  const std::vector<QString>& antennas) :
			Message(ID()),
			m_rfGains(rfGains),
			m_ifGains(ifGains),
			m_sampRates(sampRates),
			m_antennas(antennas)
		{ }
	};

	GNURadioInput(MessageQueue* msgQueueToGUI);
	~GNURadioInput();

	bool startInput(int device);
	void stopInput();

	const QString& getDeviceDescription() const;
	int getSampleRate() const;
	quint64 getCenterFrequency() const;

	bool handleMessage(Message* message);

private:
	QMutex m_mutex;
	Settings m_settings;
	GnuradioThread* m_GnuradioThread;
	QString m_deviceDescription;
	std::vector<double> m_rfGains;
	std::vector<double> m_ifGains;
	std::vector<double> m_sampRates;
	std::vector<QString> m_antennas;

	bool applySettings(const GeneralSettings& generalSettings, const Settings& settings, bool force);
};
#if 0
class DSPCmdConfigureSourceGnuradio : public DSPCmdGUIToSource {
public:
	enum {
		SourceType = 3
	};

	int sourceType() const;
	const GnuradioInput::Settings& getSettings() const { return m_settings; }

	static DSPCmdConfigureSourceGnuradio* create(const GnuradioInput::Settings& settings)
	{
		return new DSPCmdConfigureSourceGnuradio(settings);
	}

protected:
	GnuradioInput::Settings m_settings;

	DSPCmdConfigureSourceGnuradio(const GnuradioInput::Settings& settings) : m_settings(settings) { }
};
#endif
#if 0
class DSPCmdGUIInfoGnuradio : public DSPCmdSourceToGUI {
public:
	enum {
		SourceType = 3
	};

	int sourceType() const;
	const std::vector<double>& getRfGains() const { return m_rfGains; }
	const std::vector<double>& getIfGains() const { return m_ifGains; }
	const std::vector<double>& getSampRates() const { return m_sampRates; }
	const std::vector<QString>& getAntennas() const { return m_antennas; }

	static DSPCmdGUIInfoGnuradio* create(const std::vector<double>& rfGains,
										 const std::vector<double>& ifGains,
										 const std::vector<double>& sampRates,
										 const std::vector<QString>& antennas)
	{
		return new DSPCmdGUIInfoGnuradio(rfGains, ifGains, sampRates, antennas);
	}

protected:
	std::vector<double> m_rfGains;
	std::vector<double> m_ifGains;
	std::vector<double> m_sampRates;
	std::vector<QString> m_antennas;

	DSPCmdGUIInfoGnuradio(const std::vector<double>& rfGains,
						  const std::vector<double>& ifGains,
						  const std::vector<double>& sampRates,
						  const std::vector<QString>& antennas) :
		m_rfGains(rfGains),
		m_ifGains(ifGains),
		m_sampRates(sampRates),
		m_antennas(antennas)
	{ }
};
#endif
#endif // INCLUDE_GNURADIOINPUT_H
