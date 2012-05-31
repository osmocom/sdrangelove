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
	m_reference(NULL)
{
	defaults();
}

Settings::Settings(Settings* reference) :
	m_changed(false),
	m_reference(reference)
{
	defaults();
}

void Settings::defaults()
{
	m_fftSize = 1024;
	m_fftOverlap = 30;
	m_fftWindow = 3;
	m_centerFreq = 144500000;
	m_iqSwap = false;
	m_decimation = 3;
	m_e4000LNAGain = 300;
	m_e4000MixerGain = 120;
	m_e4000MixerEnh = 70;
	m_e4000if1 = 60;
	m_e4000if2 = 90;
	m_e4000if3 = 90;
	m_e4000if4 = 20;
	m_e4000if5 = 150;
	m_e4000if6 = 150;
}

void Settings::load()
{
	m_changed = false;
	QSettings s("osmocom", "SDRangelove");

	if(s.value("version", 0).toInt() != 1)
		return;

	m_fftSize = s.value("fftsize", 512).toInt();
	m_fftOverlap = s.value("fftoverlap", 30).toInt();
	m_fftWindow = s.value("fftwindow", 3).toInt();
	m_centerFreq = s.value("centerfreq", 144500000).toLongLong();;
	m_iqSwap = s.value("iqswap", false).toBool();
	m_decimation = s.value("decimation", 3).toInt();
	m_e4000LNAGain = s.value("e4000_lnagain", 300).toInt();
	m_e4000MixerGain = s.value("e4000_mixergain", 120).toInt();
	m_e4000MixerEnh = s.value("e4000_mixerenh", 70).toInt();
	m_e4000if1 = s.value("e4000_if1", 60).toInt();
	m_e4000if2 = s.value("e4000_if2", 90).toInt();
	m_e4000if3 = s.value("e4000_if3", 90).toInt();
	m_e4000if4 = s.value("e4000_if4", 20).toInt();
	m_e4000if5 = s.value("e4000_if5", 150).toInt();
	m_e4000if6 = s.value("e4000_if6", 150).toInt();
}

void Settings::save()
{
	m_changed = false;
	QSettings s("osmocom", "SDRangelove");

	s.setValue("version", 1);
	s.setValue("fftsize", m_fftSize);
	s.setValue("fftoverlap", m_fftOverlap);
	s.setValue("fftwindow", m_fftWindow);
	s.setValue("centerfreq", m_centerFreq);
	s.setValue("iqswap", m_iqSwap);
	s.setValue("e4000_lnagain", m_e4000LNAGain);
	s.setValue("e4000_mixergain", m_e4000MixerGain);
	s.setValue("e4000_mixerenh", m_e4000MixerEnh);
	s.setValue("e4000_if1", m_e4000if1);
	s.setValue("e4000_if2", m_e4000if2);
	s.setValue("e4000_if3", m_e4000if3);
	s.setValue("e4000_if4", m_e4000if4);
	s.setValue("e4000_if5", m_e4000if5);
	s.setValue("e4000_if6", m_e4000if6);
}

int Settings::fftSize() const
{
	return m_fftSize;
}

void Settings::setFFTSize(int v)
{
	m_fftSize = v;
	m_changed = true;
}

bool Settings::isModifiedFFTSize()
{
	if(m_reference->m_fftSize != m_fftSize) {
		m_fftSize = m_reference->m_fftSize;
		return true;
	} else {
		return false;
	}
}

int Settings::fftOverlap() const
{
	return m_fftOverlap;
}

void Settings::setFFTOverlap(int v)
{
	m_fftOverlap = v;
	m_changed = true;
}

bool Settings::isModifiedFFTOverlap()
{
	if(m_reference->m_fftOverlap != m_fftOverlap) {
		m_fftOverlap = m_reference->m_fftOverlap;
		return true;
	} else {
		return false;
	}
}

int Settings::fftWindow() const
{
	return m_fftWindow;
}

void Settings::setFFTWindow(int v)
{
	m_fftWindow = v;
	m_changed = true;
}

bool Settings::isModifiedFFTWindow()
{
	if(m_reference->m_fftWindow != m_fftWindow) {
		m_fftWindow = m_reference->m_fftWindow;
		return true;
	} else {
		return false;
	}
}

qint64 Settings::centerFreq() const
{
	return m_centerFreq;
}

void Settings::setCenterFreq(qint64 v)
{
	m_centerFreq = v;
	m_changed = true;
}

bool Settings::isModifiedCenterFreq()
{
	if(m_reference->m_centerFreq != m_centerFreq) {
		m_centerFreq = m_reference->m_centerFreq;
		return true;
	} else {
		return false;
	}
}

bool Settings::iqSwap() const
{
	return m_iqSwap;
}

void Settings::setIQSwap(bool v)
{
	m_iqSwap = v;
	m_changed = true;
}

bool Settings::isModifiedIQSwap()
{
	if(m_reference->m_iqSwap != m_iqSwap) {
		m_iqSwap = m_reference->m_iqSwap;
		return true;
	} else {
		return false;
	}
}

int Settings::decimation() const
{
	return m_decimation;
}

void Settings::setDecimation(int v)
{
	m_decimation = v;
	m_changed = true;
}

bool Settings::isModifiedDecimation()
{
	if(m_reference->m_decimation != m_decimation) {
		m_decimation = m_reference->m_decimation;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000LNAGain() const
{
	return m_e4000LNAGain;
}

void Settings::setE4000LNAGain(int v)
{
	m_e4000LNAGain = v;
	m_changed = true;
}

bool Settings::isModifiedE4000LNAGain()
{
	if(m_reference->m_e4000LNAGain != m_e4000LNAGain) {
		m_e4000LNAGain = m_reference->m_e4000LNAGain;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000MixerGain() const
{
	return m_e4000MixerGain;
}

void Settings::setE4000MixerGain(int v)
{
	m_e4000MixerGain = v;
	m_changed = true;
}

bool Settings::isModifiedE4000MixerGain()
{
	if(m_reference->m_e4000MixerGain != m_e4000MixerGain) {
		m_e4000MixerGain = m_reference->m_e4000MixerGain;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000MixerEnh() const
{
	return m_e4000MixerEnh;
}

void Settings::setE4000MixerEnh(int v)
{
	m_e4000MixerEnh = v;
	m_changed = true;
}

bool Settings::isModifiedE4000MixerEnh()
{
	if(m_reference->m_e4000MixerEnh != m_e4000MixerEnh) {
		m_e4000MixerEnh = m_reference->m_e4000MixerEnh;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000if1() const
{
	return m_e4000if1;
}

void Settings::setE4000if1(int v)
{
	m_e4000if1 = v;
	m_changed = true;
}

bool Settings::isModifiedE4000if1()
{
	if(m_reference->m_e4000if1 != m_e4000if1) {
		m_e4000if1 = m_reference->m_e4000if1;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000if2() const
{
	return m_e4000if2;
}

void Settings::setE4000if2(int v)
{
	m_e4000if2 = v;
	m_changed = true;
}

bool Settings::isModifiedE4000if2()
{
	if(m_reference->m_e4000if2 != m_e4000if2) {
		m_e4000if2 = m_reference->m_e4000if2;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000if3() const
{
	return m_e4000if3;
}

void Settings::setE4000if3(int v)
{
	m_e4000if3 = v;
	m_changed = true;
}

bool Settings::isModifiedE4000if3()
{
	if(m_reference->m_e4000if3 != m_e4000if3) {
		m_e4000if3 = m_reference->m_e4000if3;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000if4() const
{
	return m_e4000if4;
}

void Settings::setE4000if4(int v)
{
	m_e4000if4 = v;
	m_changed = true;
}

bool Settings::isModifiedE4000if4()
{
	if(m_reference->m_e4000if4 != m_e4000if4) {
		m_e4000if4 = m_reference->m_e4000if4;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000if5() const
{
	return m_e4000if5;
}

void Settings::setE4000if5(int v)
{
	m_e4000if5 = v;
	m_changed = true;
}

bool Settings::isModifiedE4000if5()
{
	if(m_reference->m_e4000if5 != m_e4000if5) {
		m_e4000if5 = m_reference->m_e4000if5;
		return true;
	} else {
		return false;
	}
}

int Settings::e4000if6() const
{
	return m_e4000if6;
}

void Settings::setE4000if6(int v)
{
	m_e4000if6 = v;
	m_changed = true;
}

bool Settings::isModifiedE4000if6()
{
	if(m_reference->m_e4000if6 != m_e4000if6) {
		m_e4000if6 = m_reference->m_e4000if6;
		return true;
	} else {
		return false;
	}
}
