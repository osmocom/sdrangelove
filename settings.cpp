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

Settings::Settings() :
	m_changed(false),
	m_fftSize(512),
	m_fftOverlap(30),
	m_centerFreq(144500000)
{
}

Settings::~Settings()
{
	if(m_changed)
		save();
}

void Settings::load()
{
	m_changed = false;
	QSettings s("osmocom", "SDRangelove");

	if(s.value("version", 0).toInt() != 1)
		return;

	m_fftSize = s.value("fftsize", 512).toInt();
	m_fftOverlap = s.value("fftoverlap", 30).toInt();
	m_centerFreq = s.value("centerfreq", 144500000).toLongLong();;
}

void Settings::save()
{
	m_changed = false;
	QSettings s("osmocom", "SDRangelove");

	s.setValue("version", 1);
	s.setValue("fftsize", m_fftSize);
	s.setValue("fftoverlap", m_fftOverlap);
	s.setValue("centerfreq", m_centerFreq);
}


int Settings::getFFTSize() const
{
	return m_fftSize;
}

void Settings::setFFTSize(int v)
{
	m_fftSize = v;
	m_changed = true;
}

int Settings::getFFTOverlap() const
{
	return m_fftOverlap;
}

void Settings::setFFTOverlap(int v)
{
	m_fftOverlap = v;
	m_changed = true;
}

qint64 Settings::getCenterFreq() const
{
	return m_centerFreq;
}

void Settings::setCenterFreq(qint64 v)
{
	m_centerFreq = v;
	m_changed = true;
}
