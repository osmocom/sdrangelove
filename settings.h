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

	qint64 centerFreq() const;
	void setCenterFreq(qint64 v);
	bool isModifiedCenterFreq();

private:
	bool m_changed;
	const Settings* m_reference;

	int m_fftSize;
	int m_fftOverlap;
	int m_fftWindow;
	qint64 m_centerFreq;
};

#endif // INCLUDE_SETTINGS_H
