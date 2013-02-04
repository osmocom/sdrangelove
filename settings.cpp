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

#include <QSettings>
#include "settings.h"

Settings::Settings()
{
	defaults();
}

Settings& Settings::operator=(const Settings& other)
{
	m_fftSize = other.m_fftSize;
	m_fftOverlap = other.m_fftOverlap;
	m_fftWindow = other.m_fftWindow;
	m_displayWaterfall = other.m_displayWaterfall;
	m_invertedWaterfall = other.m_invertedWaterfall;
	m_displayLiveSpectrum = other.m_displayLiveSpectrum;
	m_displayHistogram = other.displayHistogram();
	m_dcOffsetCorrection = other.m_dcOffsetCorrection;
	m_iqImbalanceCorrection = other.m_iqImbalanceCorrection;
	m_centerFrequency = other.m_centerFrequency;
	m_sourceSettings = other.m_sourceSettings;
}

void Settings::defaults()
{
	m_fftSize = 1024;
	m_fftOverlap = 10;
	m_fftWindow = 3;
	m_displayWaterfall = true;
	m_invertedWaterfall = false;
	m_displayLiveSpectrum = true;
	m_displayHistogram = true;
	m_dcOffsetCorrection = true;
	m_iqImbalanceCorrection = true;
	m_centerFrequency = 0;
	m_sourceSettings.clear();
}

void Settings::load(QSettings* settings, const QString& path)
{
	settings->beginGroup(path);
	m_fftSize = settings->value("fftsize", 512).toInt();
	m_fftOverlap = settings->value("fftoverlap", 10).toInt();
	m_fftWindow = settings->value("fftwindow", 3).toInt();
	m_displayWaterfall = settings->value("displaywaterfall", true).toBool();
	m_invertedWaterfall = settings->value("invertedwaterfall", false).toBool();
	m_displayLiveSpectrum = settings->value("displaylivespectrum", true).toBool();
	m_displayHistogram = settings->value("displayhistogram", true).toBool();
	m_dcOffsetCorrection = settings->value("dcoffsetcorrection", true).toBool();
	m_iqImbalanceCorrection = settings->value("iqimbalancecorrection", true).toBool();
	m_centerFrequency = settings->value("centerfrequency", 0).toLongLong();
	m_sourceSettings = settings->value("sourcesettings", "").toString();
	settings->endGroup();
}

void Settings::save(QSettings* settings, const QString& path) const
{
	settings->beginGroup(path);
	settings->setValue("fftsize", m_fftSize);
	settings->setValue("fftoverlap", m_fftOverlap);
	settings->setValue("fftwindow", m_fftWindow);
	settings->setValue("displaywaterfall", m_displayWaterfall);
	settings->setValue("invertedwaterfall", m_invertedWaterfall);
	settings->setValue("displaylivespectrum", m_displayLiveSpectrum);
	settings->setValue("displayhistogram", m_displayHistogram);
	settings->setValue("dcoffsetcorrection", m_dcOffsetCorrection);
	settings->setValue("iqimbalancecorrection", m_iqImbalanceCorrection);
	settings->setValue("centerfrequency", m_centerFrequency);
	settings->setValue("sourcesettings", m_sourceSettings);
	settings->endGroup();
}
