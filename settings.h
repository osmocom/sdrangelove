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

#ifndef INCLUDE_SETTINGS_H
#define INCLUDE_SETTINGS_H

#include <QtGlobal>

class Settings {
public:
	Settings();
	Settings(Settings* reference);

	void defaults();
	void load();
	void save();

	int fftSize() const;
	void setFFTSize(int v);
	bool isModifiedFFTSize();

	int fftOverlap() const;
	void setFFTOverlap(int v);
	bool isModifiedFFTOverlap();

	int fftWindow() const;
	void setFFTWindow(int v);
	bool isModifiedFFTWindow();

	bool displayWaterfall() const;
	void setDisplayWaterfall(bool v);
	bool isModifiedDisplayWaterfall();

	bool invertedWaterfall() const;
	void setInvertedWaterfall(bool v);
	bool isModifiedInvertedWaterfall();

	bool displayLiveSpectrum() const;
	void setDisplayLiveSpectrum(bool v);
	bool isModifiedDisplayLiveSpectrum();

	bool displayHistogram() const;
	void setDisplayHistogram(bool v);
	bool isModifiedDisplayHistogram();

	qint64 centerFreq() const;
	void setCenterFreq(qint64 v);
	bool isModifiedCenterFreq();

	bool iqSwap() const;
	void setIQSwap(bool v);
	bool isModifiedIQSwap();

	int decimation() const;
	void setDecimation(int v);
	bool isModifiedDecimation();

	bool dcOffsetCorrection() const;
	void setDCOffsetCorrection(bool v);
	bool isModifiedDCOffsetCorrection();

	bool iqImbalanceCorrection() const;
	void setIQImbalanceCorrection(bool v);
	bool isModifiedIQImbalanceCorrection();

	int e4000LNAGain() const;
	void setE4000LNAGain(int v);
	bool isModifiedE4000LNAGain();

	int e4000MixerGain() const;
	void setE4000MixerGain(int v);
	bool isModifiedE4000MixerGain();

	int e4000MixerEnh() const;
	void setE4000MixerEnh(int v);
	bool isModifiedE4000MixerEnh();

	int e4000if1() const;
	void setE4000if1(int v);
	bool isModifiedE4000if1();

	int e4000if2() const;
	void setE4000if2(int v);
	bool isModifiedE4000if2();

	int e4000if3() const;
	void setE4000if3(int v);
	bool isModifiedE4000if3();

	int e4000if4() const;
	void setE4000if4(int v);
	bool isModifiedE4000if4();

	int e4000if5() const;
	void setE4000if5(int v);
	bool isModifiedE4000if5();

	int e4000if6() const;
	void setE4000if6(int v);
	bool isModifiedE4000if6();

	quint8 filterI1() const;
	void setFilterI1(quint8 v);
	bool isModifiedFilterI1();

	quint8 filterI2() const;
	void setFilterI2(quint8 v);
	bool isModifiedFilterI2();

	quint8 filterQ1() const;
	void setFilterQ1(quint8 v);
	bool isModifiedFilterQ1();

	quint8 filterQ2() const;
	void setFilterQ2(quint8 v);
	bool isModifiedFilterQ2();

private:
	bool m_changed;
	const Settings* m_reference;

	int m_fftSize;
	int m_fftOverlap;
	int m_fftWindow;
	bool m_displayWaterfall;
	bool m_invertedWaterfall;
	bool m_displayLiveSpectrum;
	bool m_displayHistogram;
	qint64 m_centerFreq;
	bool m_iqSwap;
	int m_decimation;
	bool m_dcOffsetCorrection;
	bool m_iqImbalanceCorrection;

	int m_e4000LNAGain;
	int m_e4000MixerGain;
	int m_e4000MixerEnh;
	int m_e4000if1;
	int m_e4000if2;
	int m_e4000if3;
	int m_e4000if4;
	int m_e4000if5;
	int m_e4000if6;

	quint8 m_filterI1;
	quint8 m_filterI2;
	quint8 m_filterQ1;
	quint8 m_filterQ2;
};

#endif // INCLUDE_SETTINGS_H
