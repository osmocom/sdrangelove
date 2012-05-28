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

#if 0

#include <stdio.h>
#include <QPainter>
#include "waterfall.h"

Waterfall::Waterfall(QWidget* parent) :
	QWidget(parent),
	m_fftSize(512),
	m_image(NULL)
{
	// no background painting
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setMinimumHeight(128);
/*
	m_palette[0] = 0;
	for(int i = 1; i < 240; i++) {
		 QColor c;
		 c.setHsv(239 - i, 255 - ((i < 200) ? 0 : (i - 200) * 3), 150 + ((i < 100) ? i : 100));
		 m_palette[i] = c.rgb();
	}
*/
	for(int i = 0; i <= 239; i++) {
		 QColor c;
		 c.setHsv(239 - i, 255, 15 + i);
		 m_palette[i] = c.rgb();
	 }

	 createImage();

	 connect(&m_timer, SIGNAL(timeout()), this, SLOT(refresh()));
	 m_timer.start(50);
}

Waterfall::~Waterfall()
{
	QMutexLocker mutexLocker(&m_mutex);
	if(m_image != NULL) {
		delete m_image;
		m_image = NULL;
	}
}

void Waterfall::newSpectrum(const std::vector<Real>& spectrum)
{
	if(!m_mutex.tryLock())
		return;

	if(m_image == NULL) {
		m_mutex.unlock();
		return;
	}
	if(m_image->height() == 0) {
		m_mutex.unlock();
		return;
	}

	m_fftSize = spectrum.size();

	int w = spectrum.size();
	if(w > m_image->width())
		w = m_image->width();

	quint32* pix = (quint32*)m_image->scanLine(m_pos);

	for(int i = 0; i < w; i++) {
		Real vr = 2.4 * (spectrum[i] + 72.0);
		int v = (int)vr;

		if(v > 239)
			v = 239;
		else if(v < 0)
			v = 0;

		*pix++ = m_palette[(int)v];
	}

	m_pos++;
	if(m_pos >= m_image->height())
		m_pos = 0;

	m_mutex.unlock();
}

void Waterfall::createImage()
{
	if(m_image != NULL) {
		if((m_image->width() == m_fftSize) && (m_image->height() == height()))
			return;
		delete m_image;
		m_image = NULL;
	}

	m_image = new QImage(m_fftSize, height(), QImage::Format_RGB32);
	if(m_image != NULL) {
		m_image->fill(qRgb(0x00, 0x00, 0x00));
/*
		for(int i = 0; i < 240; i++)
			m_image->setPixel(10 + i, 10, m_palette[i]);
*/
		m_pos = 0;
	}
}

void Waterfall::paintEvent(QPaintEvent*)
{
	QMutexLocker mutexLocker(&m_mutex);
	QPainter painter(this);

	if(m_image == NULL) {
		painter.setPen(Qt::NoPen);
		painter.setBackground(palette().window());
		painter.drawRect(rect());
		return;
	}

	if(m_fftSize != m_image->width())
	   createImage();

	painter.drawImage(0, 0, *m_image, 0, m_pos, m_image->width(), m_image->height() - m_pos);
	painter.drawImage(0, m_image->height() - m_pos, *m_image, 0, 0, m_image->width(), m_pos);

	if(m_fftSize < width()) {
		painter.setPen(Qt::NoPen);
		painter.setBrush(palette().window());
		painter.drawRect(QRect(QPoint(m_fftSize, 0), QPoint(width(), height())));
	}
/*
	for(int i = 0; i < 240; i++) {
		painter.setPen(m_palette[i]);
		painter.drawPoint(10 + i, 10);
	}
*/
}

void Waterfall::resizeEvent(QResizeEvent*)
{
	QMutexLocker mutexLocker(&m_mutex);
	createImage();
}

void Waterfall::refresh()
{
	update();
}

#endif

#include <stdio.h>
#include <QPainter>
#include <GL/glu.h>
#include "waterfall.h"

Waterfall::Waterfall(QWidget* parent) :
	QGLWidget(parent),
	m_fftSize(1),
	m_image(NULL),
	m_textureAllocated(false)
{
	// no background painting

	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setMinimumHeight(128);
/*
	m_palette[0] = 0;
	for(int i = 1; i < 240; i++) {
		 QColor c;
		 c.setHsv(239 - i, 255 - ((i < 200) ? 0 : (i - 200) * 3), 150 + ((i < 100) ? i : 100));
		 m_palette[i] = c.rgb();
	}
*/
	for(int i = 0; i <= 239; i++) {
		 QColor c;
		 c.setHsv(239 - i, 255, 15 + i);
		 ((quint8*)&m_palette[i])[0] = c.red();
		 ((quint8*)&m_palette[i])[1] = c.green();
		 ((quint8*)&m_palette[i])[2] = c.blue();
		 ((quint8*)&m_palette[i])[3] = c.alpha();
	 }

	 createImage();

	 connect(&m_timer, SIGNAL(timeout()), this, SLOT(refresh()));
	 m_timer.start(50);
}

Waterfall::~Waterfall()
{
	QMutexLocker mutexLocker(&m_mutex);
	if(m_image != NULL) {
		delete m_image;
		m_image = NULL;
	}
	if(m_textureAllocated) {
		glDeleteTextures(1, &m_texture);
		m_textureAllocated = false;
	}
}

void Waterfall::newSpectrum(const std::vector<Real>& spectrum)
{
	if(!m_mutex.tryLock())
		return;

	if(m_image == NULL) {
		m_mutex.unlock();
		return;
	}
	if(m_image->height() == 0) {
		m_mutex.unlock();
		return;
	}
	if(m_pos >= m_image->height()) {
		m_mutex.unlock();
		return;
	}

	m_fftSize = spectrum.size();

	int w = spectrum.size();
	if(w > m_image->width())
		w = m_image->width();

	quint32* pix = (quint32*)m_image->scanLine(m_pos);

	for(int i = 0; i < w; i++) {
		Real vr = 2.4 * (spectrum[i] + 72.0);
		int v = (int)vr;

		if(v > 239)
			v = 239;
		else if(v < 0)
			v = 0;

		*pix++ = m_palette[(int)v];
	}

	m_pos++;

	m_mutex.unlock();
}

void Waterfall::createImage()
{
	if(m_image != NULL) {
		if(m_image->width() == m_fftSize)
			return;
		delete m_image;
		m_image = NULL;
	}

	m_image = new QImage(m_fftSize, 1024, QImage::Format_ARGB32);
	if(m_image != NULL) {
		m_image->fill(qRgb(0x00, 0x00, 0x00));
		m_pos = 0;
	}
	m_resizeTexture = true;
}

void Waterfall::initializeGL()
{
	qDebug("initializeGL()");
	glGenTextures(1, &m_texture);
	m_textureAllocated = true;
}

void Waterfall::resizeGL(int width, int height)
{
	qDebug("resizeGL()");

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0); // set origin to bottom left corner
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	m_resizeTexture = true;
}

void Waterfall::paintGL()
{
	QMutexLocker mutexLocker(&m_mutex);
	if(m_image == NULL)
		return;

	resizeTexture();
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	for(int i = 0; i < m_pos; i++) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_texPos, m_fftSize, 1, GL_RGBA, GL_UNSIGNED_BYTE, m_image->scanLine(i));
		m_texPos = (m_texPos + 1) % m_textureHeight;
	}
	m_pos = 0;

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glScalef(width(), height(), 1.0);
	glBegin(GL_QUADS);
	float prop_y = m_texPos / (m_textureHeight - 1.0);
	float prop_x = 1.0;
	float off = 1.0 / (m_textureHeight - 1);
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

void Waterfall::resizeTexture()
{
	if(!m_resizeTexture)
		return;
	m_resizeTexture = false;

	m_textureHeight = height();
	quint8* data = new quint8[m_fftSize * m_textureHeight * 4];
	memset(data, 0x00, m_fftSize * m_textureHeight * 4);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fftSize, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	delete[] data;
	m_texPos = 0;
}

void Waterfall::refresh()
{
	if(m_fftSize != m_image->width())
	   createImage();
	update();
}
