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

#ifndef INCLUDE_SPECTROHISTOGRAM_H
#define INCLUDE_SPECTROHISTOGRAM_H

#include <QWidget>
#include <QMutex>
#include <QTimer>
#include "dsp/dsptypes.h"

class SpectroHistogram : public QWidget {
	Q_OBJECT

public:
	SpectroHistogram(QWidget* parent = NULL);
	~SpectroHistogram();

	void newSpectrum(const std::vector<Real>& spectrum);

private:
	QTimer m_timer;
	QMutex m_mutex;
	int m_fftSize;
	QRgb m_palette[240];
	QImage* m_image;
	quint8* m_histo;
	quint8* m_histoHoldoff;
	int m_holdOff;
	int m_lateHoldOff;
	int m_holdOffCount;

	void createImage();

	void paintEvent(QPaintEvent*);

private slots:
	void refresh();
};

#endif // INCLUDE_SPECTROHISTOGRAM_H
