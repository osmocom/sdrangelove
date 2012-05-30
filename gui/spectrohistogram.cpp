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
#include <GL/glu.h>
#include "spectrohistogram.h"

SpectroHistogram::SpectroHistogram(QWidget* parent) :
	QGLWidget(parent),
	m_fftSize(512),
	m_image(NULL),
	m_histo(NULL),
	m_histoHoldoff(NULL),
	m_textureAllocated(false)
{
	// no background painting
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setMinimumHeight(100);

	m_palette[0] = 0;
	for(int i = 1; i < 240; i++) {
		 QColor c;
		 c.setHsv(239 - i, 255 - ((i < 200) ? 0 : (i - 200) * 3), 150 + ((i < 100) ? i : 100));
		 ((quint8*)&m_palette[i])[0] = c.red();
		 ((quint8*)&m_palette[i])[1] = c.green();
		 ((quint8*)&m_palette[i])[2] = c.blue();
		 ((quint8*)&m_palette[i])[3] = c.alpha();
	}
	for(int i = 1; i < 16; i++) {
		QColor c;
		c.setHsv(270, 128, 48 + i * 4);
		((quint8*)&m_palette[i])[0] = c.red();
		((quint8*)&m_palette[i])[1] = c.green();
		((quint8*)&m_palette[i])[2] = c.blue();
		((quint8*)&m_palette[i])[3] = c.alpha();
	}

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(refresh()));
	m_timer.start(50);

	createImage();
	m_holdOff = 4;
	m_holdOffCount = m_holdOff;
	m_lateHoldOff = 20;
	m_resizeTexture = true;
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
	if(m_textureAllocated) {
		glDeleteTextures(1, &m_texture);
		m_textureAllocated = false;
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
		Real vr = spectrum[i];
		int v = (int)vr;

		if((v >= 0) && (v <= 99)) {
			if(*(b + v) < 220)
				(*(b + v)) += 4;
			else if(*(b + v) < 239)
				(*(b + v)) += 1;
			//*h = m_lateHoldOff;
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
	if(m_histo != NULL) {
		delete[] m_histo;
		m_histo = NULL;
	}
	if(m_histoHoldoff != NULL) {
		delete[] m_histoHoldoff;
		m_histoHoldoff = NULL;
	}

	m_image = new QImage(m_fftSize, 100, QImage::Format_RGB32);
	if(m_image != NULL)
		m_image->fill(qRgb(0x00, 0x00, 0x00));

	m_histo = new quint8[100 * m_fftSize];
	memset(m_histo, 0x00, 100 * m_fftSize);
	m_histoHoldoff = new quint8[100 * m_fftSize];
	memset(m_histoHoldoff, 0x07, 100 * m_fftSize);

	m_resizeTexture = true;
}

void SpectroHistogram::initializeGL()
{
	glGenTextures(1, &m_texture);
	m_textureAllocated = true;
}

void SpectroHistogram::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	m_resizeTexture = true;
}

void SpectroHistogram::paintGL()
{
	if(!m_mutex.tryLock())
		return;

	if(m_image == NULL) {
		m_mutex.unlock();
		return;
	}

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

	resizeTexture();
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_fftSize, 100, GL_RGBA, GL_UNSIGNED_BYTE, m_image->scanLine(0));
	m_mutex.unlock();
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glScalef(width(), height(), 1.0);
	glBegin(GL_QUADS);
	float prop_y = 0.0;
	float prop_x = 1.0;
	float off = 1.0 / (100 - 1);
	glTexCoord2f(0, prop_y + 1 - off);
	glVertex2f(0, 1);
	glTexCoord2f(prop_x, prop_y + 1 - off);
	glVertex2f(1, 1);
	glTexCoord2f(prop_x, prop_y);
	glVertex2f(1, 0);
	glTexCoord2f(0, prop_y);
	glVertex2f(0, 0);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void SpectroHistogram::resizeTexture()
{
	if(!m_resizeTexture)
		return;
	m_resizeTexture = false;

	if(!m_textureAllocated) {
		glGenTextures(1, &m_texture);
		m_textureAllocated = true;
	}

	quint8* data = new quint8[m_fftSize * 100 * 4];
	memset(data, 0x00, m_fftSize * 100 * 4);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fftSize, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	delete[] data;
}

void SpectroHistogram::refresh()
{
	if(m_fftSize != m_image->width())
	   createImage();

	update();
}
