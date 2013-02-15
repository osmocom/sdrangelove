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
#include <QSettings>

class Settings {
public:
	Settings();
	Settings(const Settings& other);
	Settings& operator=(const Settings& other);

	void defaults();
	void load(QSettings* settings, const QString& path);
	void save(QSettings* settings, const QString& path) const;

	int fftSize() const { return m_fftSize; }
	void setFFTSize(int v) { m_fftSize = v; }

	int fftOverlap() const { return m_fftOverlap; }
	void setFFTOverlap(int v) { m_fftOverlap = v; }

	int fftWindow() const { return m_fftWindow; }
	void setFFTWindow(int v) { m_fftWindow = v; }

	bool displayWaterfall() const { return m_displayWaterfall; }
	void setDisplayWaterfall(bool v) { m_displayWaterfall = v; }

	bool invertedWaterfall() const { return m_invertedWaterfall; }
	void setInvertedWaterfall(bool v) { m_invertedWaterfall = v; }

	bool displayLiveSpectrum() const { return m_displayLiveSpectrum; }
	void setDisplayLiveSpectrum(bool v) { m_displayLiveSpectrum = v; }

	bool displayHistogram() const { return m_displayHistogram; }
	void setDisplayHistogram(bool v) { m_displayHistogram = v; }

	bool dcOffsetCorrection() const { return m_dcOffsetCorrection; }
	void setDCOffsetCorrection(bool v) { m_dcOffsetCorrection = v; }

	bool iqImbalanceCorrection() const { return m_iqImbalanceCorrection; }
	void setIQImbalanceCorrection(bool v) { m_iqImbalanceCorrection = v; }

	bool displayScope() const { return m_displayScope; }
	void setDisplayScope(bool v) { m_displayScope = v; }

	quint64 centerFrequency() const { return m_centerFrequency; }
	void setCenterFrequency(quint64 v) { m_centerFrequency = v; }

	const QString& sourceSettings() const { return m_sourceSettings; }
	void setSourceSettings(const QString& v) { m_sourceSettings = v; }

private:
	int m_fftSize;
	int m_fftOverlap;
	int m_fftWindow;
	bool m_displayWaterfall;
	bool m_invertedWaterfall;
	bool m_displayLiveSpectrum;
	bool m_displayHistogram;
	bool m_dcOffsetCorrection;
	bool m_iqImbalanceCorrection;
	bool m_displayScope;
	quint64 m_centerFrequency;
	QString m_sourceSettings;
};

#endif // INCLUDE_SETTINGS_H
