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

#ifndef INCLUDE_GNURADIOGUI_H
#define INCLUDE_GNURADIOGUI_H

#include <QTimer>
#include <QPair>
#include <QList>
#include <QString>
#include "plugin/plugingui.h"
#include "gnuradioinput.h"

namespace Ui {
	class GNURadioGui;
}

class PluginAPI;

class GNURadioGui : public PluginGUI {
	Q_OBJECT

public:
	explicit GNURadioGui(PluginAPI* pluginAPI, QWidget* parent = NULL);
	~GNURadioGui();
	void destroy();

	void resetToDefaults();
	QByteArray serializeGeneral() const;
	bool deserializeGeneral(const QByteArray&data);
	quint64 getCenterFrequency() const;
	QByteArray serialize() const;
	bool deserialize(const QByteArray& data);
	bool handleMessage(Message* message);

private:
	Ui::GNURadioGui* ui;
	PluginAPI* m_pluginAPI;
	SampleSource* m_sampleSource;
	QList< QPair<QString, QString> > m_devs;
	std::vector<double> m_rfGains;
	std::vector<double> m_ifGains;
	std::vector<double> m_sampRates;
	std::vector<QString> m_antennas;
	std::vector<QString> m_iqbals;

	SampleSource::GeneralSettings m_generalSettings;
	GNURadioInput::Settings m_settings;
	QTimer m_updateTimer;

	void displaySettings();
	void sendSettings();

private slots:
	void updateHardware();

	void on_gnuradioDevices_currentIndexChanged(int index);
	void on_centerFrequency_changed(quint64 value);
	void on_sldFreqCorr_valueChanged(int value);
	void on_sldRfGain_valueChanged(int value);
	void on_sldIfGain_valueChanged(int value);
	void on_cboSampleRate_currentIndexChanged(int index);
	void on_deviceArguments_textChanged(const QString &arg1);
	void on_cboAntennas_currentIndexChanged(const QString &arg1);
	void on_cboIQBalance_currentIndexChanged(const QString &arg1);
};

#endif // INCLUDE_GNURADIOGUI_H
