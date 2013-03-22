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

#include "gnuradiogui.h"
#include "ui_gnuradiogui.h"

#include <osmosdr_device.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <plugin/pluginapi.h>

GNURadioGui::GNURadioGui(PluginAPI* pluginAPI, QWidget* parent) :
	PluginGUI(parent),
	ui(new Ui::GNURadioGui),
	m_pluginAPI(pluginAPI),
	m_settings(),
	m_sampleSource(NULL)
{
	ui->setupUi(this);
	ui->centerFrequency->setValueRange(7, 20000U, 2200000U);
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateHardware()));
	displaySettings();

	m_sampleSource = new GNURadioInput(m_pluginAPI->getMainWindowMessageQueue());
	m_pluginAPI->setSampleSource(m_sampleSource);
}

GNURadioGui::~GNURadioGui()
{
	delete ui;
}

void GNURadioGui::destroy()
{
	delete this;
}

void GNURadioGui::resetToDefaults()
{
	m_generalSettings.resetToDefaults();
	m_settings.resetToDefaults();
	displaySettings();
	sendSettings();
}

QByteArray GNURadioGui::serializeGeneral() const
{
	return m_generalSettings.serialize();
}

bool GNURadioGui::deserializeGeneral(const QByteArray&data)
{
	if(m_generalSettings.deserialize(data)) {
		displaySettings();
		sendSettings();
		return true;
	} else {
		resetToDefaults();
		return false;
	}
}

quint64 GNURadioGui::getCenterFrequency() const
{
	return m_generalSettings.m_centerFrequency;
}

QByteArray GNURadioGui::serialize() const
{
	return m_settings.serialize();
}

bool GNURadioGui::deserialize(const QByteArray& data)
{
	if(m_settings.deserialize(data)) {
		displaySettings();
		sendSettings();
		return true;
	} else {
		resetToDefaults();
		return false;
	}
}

bool GNURadioGui::handleMessage(Message* message)
{
	if(message->id() == GNURadioInput::MsgReportGNURadio::ID()) {
		GNURadioInput::MsgReportGNURadio* rep = (GNURadioInput::MsgReportGNURadio*)message;
		m_rfGains = rep->getRfGains();
		m_ifGains = rep->getIfGains();
		m_sampRates = rep->getSampRates();
		m_antennas = rep->getAntennas();
		displaySettings();
		return true;
	} else {
		return false;
	}
}

void GNURadioGui::displaySettings()
{
	int oldIndex = 0;

	oldIndex = ui->gnuradioDevices->currentIndex();
	ui->gnuradioDevices->clear();

	BOOST_FOREACH(osmosdr::device_t dev, osmosdr::device::find())
	{
		QString label;

		if ( dev.count( "label" ) ) {
			label = QString(dev[ "label" ].c_str());
			dev.erase("label");
		}

		QPair< QString, QString > pair(label, dev.to_string().c_str());
		m_devs.append(pair);

		ui->gnuradioDevices->addItem(label);
	}

	if ( ui->gnuradioDevices->count() && oldIndex >= 0 )
	{
		ui->gnuradioDevices->setCurrentIndex(oldIndex);
		ui->deviceArguments->setText(m_devs[oldIndex].second);
	}

	ui->centerFrequency->setValue(m_generalSettings.m_centerFrequency / 1000);

	if ( m_rfGains.size() ) {
		oldIndex = ui->sldRfGain->value();
		ui->sldRfGain->setMinimum(0);
		ui->sldRfGain->setMaximum(m_rfGains.size() - 1);
		ui->sldRfGain->setValue(oldIndex == 0 ? m_rfGains.size() / 2 : oldIndex);
		ui->sldRfGain->setEnabled(true);
	} else {
		ui->sldRfGain->setEnabled(false);
	}

	if ( m_ifGains.size() ) {
		oldIndex = ui->sldIfGain->value();
		ui->sldIfGain->setMinimum(0);
		ui->sldIfGain->setMaximum(m_ifGains.size() - 1);
		ui->sldIfGain->setValue(oldIndex == 0 ? m_ifGains.size() / 2 : oldIndex);
		ui->sldIfGain->setEnabled(true);
	} else {
		ui->sldIfGain->setEnabled(false);
	}

	oldIndex = ui->cboSampleRate->currentIndex();
	ui->cboSampleRate->clear();

	for ( int i = 0; i < m_sampRates.size(); i++ )
		ui->cboSampleRate->addItem(QString("%1").arg( m_sampRates[i] ));

	if ( ui->cboSampleRate->count() && oldIndex >= 0 )
		ui->cboSampleRate->setCurrentIndex(oldIndex);

	oldIndex = ui->cboAntennas->currentIndex();
	ui->cboAntennas->clear();

	if ( m_antennas.size() ) {
		for ( int i = 0; i < m_antennas.size(); i++ )
			ui->cboAntennas->addItem(QString("%1").arg( m_antennas[i] ));

		if ( ui->cboAntennas->count() && oldIndex >= 0 )
			ui->cboAntennas->setCurrentIndex(oldIndex);

		ui->cboAntennas->setEnabled(true);
	} else {
		ui->cboAntennas->setEnabled(false);
	}
}

void GNURadioGui::sendSettings()
{
	if(!m_updateTimer.isActive())
		m_updateTimer.start(100);
}

void GNURadioGui::updateHardware()
{
	m_updateTimer.stop();
	GNURadioInput::MsgConfigureGNURadio* msg = GNURadioInput::MsgConfigureGNURadio::create(m_generalSettings, m_settings);
	msg->submit(m_pluginAPI->getDSPEngineMessageQueue());
}

void GNURadioGui::on_gnuradioDevices_currentIndexChanged(int index)
{
	if ( index < 0 || index >= m_devs.count() )
		return;

	ui->deviceArguments->setText(m_devs[index].second);
}

void GNURadioGui::on_centerFrequency_changed(quint64 value)
{
	m_generalSettings.m_centerFrequency = value * 1000;
	sendSettings();
}

void GNURadioGui::on_sldFreqCorr_valueChanged(int value)
{
	ui->lblFreqCorrValue->setText(tr("%1").arg(value));
	m_settings.m_freqCorr = value;
	sendSettings();
}

void GNURadioGui::on_sldRfGain_valueChanged(int value)
{
	if ( value >= m_rfGains.size() )
		return;

	double gain = m_rfGains[value];
	ui->lblRfGainValue->setText(tr("%1").arg(gain));
	m_settings.m_rfGain = gain;
	sendSettings();
}

void GNURadioGui::on_sldIfGain_valueChanged(int value)
{
	if ( value >= m_ifGains.size() )
		return;

	double gain = m_ifGains[value];
	ui->lblIfGainValue->setText(tr("%1").arg(gain));
	m_settings.m_ifGain = gain;
	sendSettings();
}

void GNURadioGui::on_cboSampleRate_currentIndexChanged(int index)
{
	if ( index < 0 || index >= m_sampRates.size() )
		return;

	m_settings.m_sampleRate = m_sampRates[index];
	sendSettings();
}

void GNURadioGui::on_deviceArguments_textChanged(const QString &arg1)
{
	m_settings.m_args = arg1;
	sendSettings();
}

void GNURadioGui::on_cboAntennas_currentIndexChanged(const QString &arg1)
{
	m_settings.m_antenna = arg1;
	std::cout << arg1.toStdString() << std::endl;
	sendSettings();
}
