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

#include <QPainter>
#include "spectrohistogram.h"

SpectroHistogram::SpectroHistogram(QWidget* parent) :
	QWidget(parent),
	m_fftSize(512),
	m_image(NULL),
	m_histo(NULL),
	m_histoHoldoff(NULL)
{
	// no background painting
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setMinimumHeight(100);
	setMaximumHeight(100);

	m_palette[0] = 0;
	for(int i = 1; i < 240; i++) {
		 QColor c;
		 c.setHsv(239 - i, 255 - ((i < 200) ? 0 : (i - 200) * 3), 150 + ((i < 100) ? i : 100));
		 m_palette[i] = c.rgb();
	}
	for(int i = 1; i < 16; i++) {
		QColor c;
		c.setHsv(180, 64, 48 + i * 4);
		m_palette[i] = c.rgb();
	}

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(refresh()));
	m_timer.start(50);

	createImage();
	m_holdOff = 4;
	m_holdOffCount = m_holdOff;
	m_lateHoldOff = 20;
}

SpectroHistogram::~SpectroHistogram()
{
	QMutexLocker mutexLocker(&m_mutex);

	if(m_image != NULL) {
		delete m_image;
		m_image = NULL;
	}
	if(m_histo != NULL) {
		delete[] m_histo;
		m_histo = NULL;
	}
	if(m_histoHoldoff != NULL) {
		delete[] m_histoHoldoff;
		m_histoHoldoff = NULL;
	}
}

void SpectroHistogram::newSpectrum(const std::vector<Real>& spectrum)
{
	if(!m_mutex.tryLock())
		return;

	if(m_image == NULL) {
		m_mutex.unlock();
		return;
	}
	if(m_image->width() != (int)spectrum.size()) {
		m_fftSize = spectrum.size();
		m_mutex.unlock();
		return;
	}

	quint8* b = m_histo;
	quint8* h = m_histoHoldoff;

	m_holdOffCount--;
	if(m_holdOffCount <= 0) {
		for(int i = 0; i < 100 * m_fftSize; i++) {
			if(*b > 20) {
				*b = *b - 1;
			} else if(*b > 0) {
				if(*h > 0) {
					*h = *h - 1;
				} else {
					*h = m_lateHoldOff;
					*b = *b - 1;
				}
			}
			b++;
			h++;
		}
		m_holdOffCount = m_holdOff;
	}

	b = m_histo;
	h = m_histoHoldoff;
	for(size_t i = 0; i < spectrum.size(); i++) {
		Real vr = spectrum[i] + 72.0;
		int v = (int)vr;

		if((v >= 0) && (v <= 99)) {
			if(*(b + v) < 220)
				(*(b + v)) += 4;
			else if(*(b + v) < 239)
				(*(b + v)) += 1;
			*h = m_lateHoldOff;
		}

		b += 100;
		h += 100;
	}

	m_mutex.unlock();
}

void SpectroHistogram::createImage()
{
	if(m_image != NULL) {
		delete m_image;
		m_image = NULL;
	}

	m_image = new QImage(m_fftSize, 100, QImage::Format_RGB32);
	if(m_image != NULL) {
		m_image->fill(qRgb(0x00, 0x00, 0x00));
/*
		for(int i = 0; i < 240; i++) {
			m_image->setPixel(10 + i, 10, m_palette[i]);
			m_image->setPixel(10 + i, 11, m_palette[i]);
			m_image->setPixel(10 + i, 12, m_palette[i]);
		}
*/
	}
	m_histo = new quint8[100 * m_fftSize];
	memset(m_histo, 0x00, 100 * m_fftSize);
	m_histoHoldoff = new quint8[100 * m_fftSize];
	memset(m_histoHoldoff, 0x07, 100 * m_fftSize);
}

void SpectroHistogram::paintEvent(QPaintEvent*)
{
	QMutexLocker mutexLocker(&m_mutex);
	QPainter painter(this);

	if(m_image == NULL) {
		painter.setPen(Qt::NoPen);
		painter.setBackground(palette().window());
		painter.drawRect(rect());
		return;
	}

	if(m_image->width() != m_fftSize)
		createImage();

#if 1
	quint32* pix;
	quint8* bs = m_histo;
	for(int y = 0; y < 100; y++) {
		quint8* b = bs;
		pix = (quint32*)m_image->scanLine(99 - y);
		for(int x = 0; x < m_fftSize; x++) {
			*pix = m_palette[*b];
			pix++;
			b += 100;
		}
		bs++;
	}
#endif
	painter.drawImage(0, 0, *m_image);

	if(m_fftSize < width()) {
		painter.setPen(Qt::NoPen);
		painter.setBrush(palette().window());
		painter.drawRect(QRect(QPoint(m_fftSize, 0), QPoint(width(), height())));
	}
}

void SpectroHistogram::refresh()
{
	update();
}
