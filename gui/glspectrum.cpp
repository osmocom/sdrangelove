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

#include <QMouseEvent>
#include "glspectrum.h"

GLSpectrum::GLSpectrum(QWidget* parent) :
	QGLWidget(parent),
	m_cursorState(CSNormal),
	m_changesPending(true),
	m_centerFrequency(100000000),
	m_referenceLevel(0),
	m_powerRange(100),
	m_sampleRate(500000),
	m_fftSize(512),
	m_invertedWaterfall(false),
	m_displayLiveSpectrum(false),
	m_liveSpectrumChanged(false),
	m_leftMarginTextureAllocated(false),
	m_frequencyTextureAllocated(false),
	m_waterfallBuffer(NULL),
	m_waterfallTextureAllocated(false),
	m_waterfallTextureHeight(-1),
	m_displayWaterfall(true),
	m_histogramBuffer(NULL),
	m_histogram(NULL),
	m_histogramHoldoff(NULL),
	m_histogramTextureAllocated(false),
	m_displayHistogram(true),
	m_displayChanged(false)
{
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setMouseTracking(true);

	setMinimumSize(200, 200);

	m_waterfallShare = 0.5;

	for(int i = 0; i <= 239; i++) {
		 QColor c;
		 c.setHsv(239 - i, 255, 15 + i);
		 ((quint8*)&m_waterfallPalette[i])[0] = c.red();
		 ((quint8*)&m_waterfallPalette[i])[1] = c.green();
		 ((quint8*)&m_waterfallPalette[i])[2] = c.blue();
		 ((quint8*)&m_waterfallPalette[i])[3] = c.alpha();
	}
	m_waterfallPalette[239] = 0xffffffff;

	m_histogramPalette[0] = m_waterfallPalette[0];
	for(int i = 1; i < 240; i++) {
		 QColor c;
		 c.setHsv(239 - i, 255 - ((i < 200) ? 0 : (i - 200) * 3), 150 + ((i < 100) ? i : 100));
		 ((quint8*)&m_histogramPalette[i])[0] = c.red();
		 ((quint8*)&m_histogramPalette[i])[1] = c.green();
		 ((quint8*)&m_histogramPalette[i])[2] = c.blue();
		 ((quint8*)&m_histogramPalette[i])[3] = c.alpha();
	}
	for(int i = 1; i < 16; i++) {
		QColor c;
		c.setHsv(270, 128, 48 + i * 4);
		((quint8*)&m_histogramPalette[i])[0] = c.red();
		((quint8*)&m_histogramPalette[i])[1] = c.green();
		((quint8*)&m_histogramPalette[i])[2] = c.blue();
		((quint8*)&m_histogramPalette[i])[3] = c.alpha();
	}
	m_histogramHoldoffBase = 4;
	m_histogramHoldoffCount = m_histogramHoldoffBase;
	m_histogramLateHoldoff = 20;

	m_timeScale.setFont(font());
	m_timeScale.setOrientation(Qt::Vertical);
	m_powerScale.setFont(font());
	m_powerScale.setOrientation(Qt::Vertical);
	m_frequencyScale.setFont(font());
	m_frequencyScale.setOrientation(Qt::Horizontal);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
	m_timer.start(50);
}

GLSpectrum::~GLSpectrum()
{
	QMutexLocker mutexLocker(&m_mutex);

	m_changesPending = true;

	if(m_waterfallBuffer != NULL) {
		delete m_waterfallBuffer;
		m_waterfallBuffer = NULL;
	}
	if(m_waterfallTextureAllocated) {
		glDeleteTextures(1, &m_waterfallTexture);
		m_waterfallTextureAllocated = false;
	}
	if(m_histogramBuffer != NULL) {
		delete m_histogramBuffer;
		m_histogramBuffer = NULL;
	}
	if(m_histogram != NULL) {
		delete[] m_histogram;
		m_histogram = NULL;
	}
	if(m_histogramHoldoff != NULL) {
		delete[] m_histogramHoldoff;
		m_histogramHoldoff = NULL;
	}
	if(m_histogramTextureAllocated) {
		glDeleteTextures(1, &m_histogramTexture);
		m_histogramTextureAllocated = false;
	}
	if(m_leftMarginTextureAllocated) {
		deleteTexture(m_leftMarginTexture);
		m_leftMarginTextureAllocated = false;
	}
	if(m_frequencyTextureAllocated) {
		deleteTexture(m_frequencyTexture);
		m_frequencyTextureAllocated = false;
	}
}

void GLSpectrum::setCenterFrequency(quint64 frequency)
{
	m_centerFrequency = frequency;
	m_changesPending = true;
	update();
}

void GLSpectrum::setReferenceLevel(Real referenceLevel)
{
	m_referenceLevel = referenceLevel;
	m_changesPending = true;
	update();
}

void GLSpectrum::setPowerRange(Real powerRange)
{
	m_powerRange = powerRange;
	m_changesPending = true;
	update();
}

void GLSpectrum::setSampleRate(qint32 sampleRate)
{
	m_sampleRate = sampleRate;
	m_changesPending = true;
	update();
}

void GLSpectrum::setDisplayWaterfall(bool display)
{
	m_displayWaterfall = display;
	m_changesPending = true;
	stopSplitterMove();
	update();
}

void GLSpectrum::setInvertedWaterfall(bool inv)
{
	m_invertedWaterfall = inv;
	m_changesPending = true;
	stopSplitterMove();
	update();
}

void GLSpectrum::setDisplayLiveSpectrum(bool display)
{
	m_displayLiveSpectrum = display;
	m_changesPending = true;
	stopSplitterMove();
	update();
}

void GLSpectrum::setDisplayHistogram(bool display)
{
	m_displayHistogram = display;
	m_changesPending = true;
	stopSplitterMove();
	update();
}

void GLSpectrum::newSpectrum(const std::vector<Real>& spectrum, int fftSize)
{
	QMutexLocker mutexLocker(&m_mutex);

	m_displayChanged = true;

	if(m_changesPending) {
		m_fftSize = fftSize;
		return;
	}

	if(fftSize != m_fftSize) {
		m_fftSize = fftSize;
		m_changesPending = true;
		return;
	}

	updateWaterfall(spectrum);
	updateHistogram(spectrum);

	if(!m_liveSpectrumChanged) {
		std::copy(spectrum.begin(), spectrum.begin() + m_fftSize, m_liveSpectrum.begin());
		m_liveSpectrumChanged = true;
	}
}

void GLSpectrum::updateWaterfall(const std::vector<Real>& spectrum)
{
	if(m_waterfallBufferPos < m_waterfallBuffer->height()) {
		quint32* pix = (quint32*)m_waterfallBuffer->scanLine(m_waterfallBufferPos);

		for(int i = 0; i < m_fftSize; i++) {
			Real vr = (int)((spectrum[i] - m_referenceLevel) * 2.4 * 100.0 / m_powerRange + 240.0);
			int v = (int)vr;

			if(v > 239)
				v = 239;
			else if(v < 0)
				v = 0;

			*pix++ = m_waterfallPalette[(int)v];
		}

		m_waterfallBufferPos++;
	}
}

void GLSpectrum::updateHistogram(const std::vector<Real>& spectrum)
{
	quint8* b = m_histogram;
	quint8* h = m_histogramHoldoff;

	m_histogramHoldoffCount--;
	if(m_histogramHoldoffCount <= 0) {
		for(int i = 0; i < 100 * m_fftSize; i++) {
			if(*b > 20) {
				*b = *b - 1;
			} else if(*b > 0) {
				if(*h > 0) {
					*h = *h - 1;
				} else {
					*b = *b - 1;
					*h = m_histogramLateHoldoff;
				}
			}
			b++;
			h++;
		}
		m_histogramHoldoffCount = m_histogramHoldoffBase;
	}

	for(int i = 0; i < m_fftSize; i++) {
		int v = (int)((spectrum[i] - m_referenceLevel) * 100.0 / m_powerRange + 100.0);

		if((v >= 0) && (v <= 99)) {
			b = m_histogram + i * 100 + v;
			if(*b < 220)
				*b += 4;
			else if(*b < 239)
				*b += 1;
		}
	}
}

void GLSpectrum::initializeGL()
{
}

void GLSpectrum::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);

	m_changesPending = true;
}

void GLSpectrum::paintGL()
{
	if(!m_mutex.tryLock(2))
		return;

	if(m_changesPending)
		applyChanges();

	if(m_fftSize <= 0) {
		m_mutex.unlock();
		return;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glScalef(2.0, -2.0, 1.0);
	glTranslatef(-0.50, -0.5, 0);

	// paint waterfall
	if(m_displayWaterfall) {
		glPushMatrix();
		glTranslatef(m_glWaterfallRect.x(), m_glWaterfallRect.y(), 0);
		glScalef(m_glWaterfallRect.width(), m_glWaterfallRect.height(), 1);

		glBindTexture(GL_TEXTURE_2D, m_waterfallTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		for(int i = 0; i < m_waterfallBufferPos; i++) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_waterfallTexturePos, m_fftSize, 1, GL_RGBA, GL_UNSIGNED_BYTE, m_waterfallBuffer->scanLine(i));
			m_waterfallTexturePos = (m_waterfallTexturePos + 1) % m_waterfallTextureHeight;
		}
		m_waterfallBufferPos = 0;
		float prop_y = m_waterfallTexturePos / (m_waterfallTextureHeight - 1.0);
		float off = 1.0 / (m_waterfallTextureHeight - 1.0);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glTexCoord2f(0, prop_y + 1 - off);
		glVertex2f(0, m_invertedWaterfall ? 0 : 1);
		glTexCoord2f(1, prop_y + 1 - off);
		glVertex2f(1, m_invertedWaterfall ? 0 : 1);
		glTexCoord2f(1, prop_y);
		glVertex2f(1, m_invertedWaterfall ? 1 : 0);
		glTexCoord2f(0, prop_y);
		glVertex2f(0, m_invertedWaterfall ? 1 : 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		// draw rect around
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(1.0f);
		glColor4f(1, 1, 1, 0.5);
		glBegin(GL_LINE_LOOP);
		glVertex2f(1, 1);
		glVertex2f(0, 1);
		glVertex2f(0, 0);
		glVertex2f(1, 0);
		glEnd();
		glDisable(GL_BLEND);

		glPopMatrix();
	}

	// paint histogram
	if(m_displayHistogram || m_displayLiveSpectrum) {
		glPushMatrix();
		glTranslatef(m_glHistogramRect.x(), m_glHistogramRect.y(), 0);
		glScalef(m_glHistogramRect.width(), m_glHistogramRect.height(), 1);
		if(m_displayHistogram) {
			// import new lines into the texture
			quint32* pix;
			quint8* bs = m_histogram;
			for(int y = 0; y < 100; y++) {
				quint8* b = bs;
				pix = (quint32*)m_histogramBuffer->scanLine(99 - y);
				for(int x = 0; x < m_fftSize; x++) {
					*pix = m_histogramPalette[*b];
					pix++;
					b += 100;
				}
				bs++;
			}

			// draw texture
			glBindTexture(GL_TEXTURE_2D, m_histogramTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_fftSize, 100, GL_RGBA, GL_UNSIGNED_BYTE, m_histogramBuffer->scanLine(0));
			glEnable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2f(0, 0);
			glTexCoord2f(1, 0);
			glVertex2f(1, 0);
			glTexCoord2f(1, 1);
			glVertex2f(1, 1);
			glTexCoord2f(0, 1);
			glVertex2f(0, 1);
			glEnd();
			glDisable(GL_TEXTURE_2D);
		}
		// draw rect around
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(1.0f);
		glColor4f(1, 1, 1, 0.5);
		glBegin(GL_LINE_LOOP);
		glVertex2f(1, 1);
		glVertex2f(0, 1);
		glVertex2f(0, 0);
		glVertex2f(1, 0);
		glEnd();
		glDisable(GL_BLEND);
		glPopMatrix();
	}

	// paint left scales (time and power)
	if(m_displayWaterfall || m_displayLiveSpectrum || m_displayHistogram ) {
		glBindTexture(GL_TEXTURE_2D, m_leftMarginTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glPushMatrix();
		glTranslatef(m_glLeftScaleRect.x(), m_glLeftScaleRect.y(), 0);
		glScalef(m_glLeftScaleRect.width(), m_glLeftScaleRect.height(), 1);

		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0, 1);
		glTexCoord2f(1, 0);
		glVertex2f(1, 1);
		glTexCoord2f(1, 1);
		glVertex2f(1, 0);
		glTexCoord2f(0, 1);
		glVertex2f(0, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	// paint frequency scale
	if(m_displayWaterfall || m_displayLiveSpectrum || m_displayHistogram ) {
		glBindTexture(GL_TEXTURE_2D, m_frequencyTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glPushMatrix();
		glTranslatef(m_glFrequencyScaleRect.x(), m_glFrequencyScaleRect.y(), 0);
		glScalef(m_glFrequencyScaleRect.width(), m_glFrequencyScaleRect.height(), 1);

		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0, 1);
		glTexCoord2f(1, 0);
		glVertex2f(1, 1);
		glTexCoord2f(1, 1);
		glVertex2f(1, 0);
		glTexCoord2f(0, 1);
		glVertex2f(0, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	// paint live spectrum lines on top of histogram
	if(m_displayLiveSpectrum) {
		glPushMatrix();
		glTranslatef(m_glHistogramRect.x(), m_glHistogramRect.y(), 0);
		glScalef(m_glHistogramRect.width() / (float)(m_fftSize - 1), -m_glHistogramRect.height() / m_powerRange, 1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);
		glColor4f(1, 1, 1, 0.5f);
		for(int i = 1; i < m_fftSize; i++) {
			glBegin(GL_LINE_LOOP);
			glVertex2f(i - 1, m_liveSpectrum[i - 1] - m_referenceLevel);
			glVertex2f(i, m_liveSpectrum[i] - m_referenceLevel);
			glEnd();
		}
		glDisable(GL_LINE_SMOOTH);
		glPopMatrix();
	}
	m_liveSpectrumChanged = false;

	// paint waterfall grid
	if(m_displayWaterfall) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(1.0f);
		glColor4f(1, 1, 1, 0.05);

		glPushMatrix();
		glTranslatef(m_glWaterfallRect.x(), m_glWaterfallRect.y(), 0);
		glScalef(m_glWaterfallRect.width(), m_glWaterfallRect.height(), 1);

		const ScaleEngine::TickList* tickList;
		const ScaleEngine::Tick* tick;

		tickList = &m_timeScale.getTickList();
		for(int i= 0; i < tickList->count(); i++) {
			tick = &(*tickList)[i];
			if(tick->major) {
				if(tick->textSize > 0) {
					float y = tick->pos / m_timeScale.getSize();
					glBegin(GL_LINE_LOOP);
					glVertex2f(0, y);
					glVertex2f(1, y);
					glEnd();
				}
			}
		}

		tickList = &m_frequencyScale.getTickList();
		for(int i= 0; i < tickList->count(); i++) {
			tick = &(*tickList)[i];
			if(tick->major) {
				if(tick->textSize > 0) {
					float x = tick->pos / m_frequencyScale.getSize();
					glBegin(GL_LINE_LOOP);
					glVertex2f(x, 0);
					glVertex2f(x, 1);
					glEnd();
				}
			}
		}

		glPopMatrix();
	}

	// paint histogram grid
	if(m_displayHistogram || m_displayLiveSpectrum) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(1.0f);
		glColor4f(1, 1, 1, 0.05);

		glPushMatrix();
		glTranslatef(m_glHistogramRect.x(), m_glHistogramRect.y(), 0);
		glScalef(m_glHistogramRect.width(), m_glHistogramRect.height(), 1);

		const ScaleEngine::TickList* tickList;
		const ScaleEngine::Tick* tick;

		tickList = &m_powerScale.getTickList();
		for(int i= 0; i < tickList->count(); i++) {
			tick = &(*tickList)[i];
			if(tick->major) {
				if(tick->textSize > 0) {
					float y = tick->pos / m_powerScale.getSize();
					glBegin(GL_LINE_LOOP);
					glVertex2f(0, y);
					glVertex2f(1, y);
					glEnd();
				}
			}
		}

		tickList = &m_frequencyScale.getTickList();
		for(int i= 0; i < tickList->count(); i++) {
			tick = &(*tickList)[i];
			if(tick->major) {
				if(tick->textSize > 0) {
					float x = tick->pos / m_frequencyScale.getSize();
					glBegin(GL_LINE_LOOP);
					glVertex2f(x, 0);
					glVertex2f(x, 1);
					glEnd();
				}
			}
		}

		glPopMatrix();
	}

	glPopMatrix();

	m_mutex.unlock();
}

void GLSpectrum::stopSplitterMove()
{
	if(m_cursorState != CSNormal) {
		if(m_cursorState == CSSplitterMoving)
			releaseMouse();
		setCursor(Qt::ArrowCursor);
		m_cursorState = CSNormal;
	}
}

void GLSpectrum::applyChanges()
{
	if(m_fftSize <= 0)
		return;

	QFontMetrics fm(font());
	int M = fm.width("-");

	m_liveSpectrum.resize(m_fftSize);

	int topMargin = fm.ascent() * 1.5;
	int bottomMargin = fm.ascent() * 1.5;

	int waterfallHeight;
	int waterfallTop;
	int frequencyScaleHeight = fm.height() * 2;
	int frequencyScaleTop;
	int histogramTop;
	int histogramHeight;
	int leftMargin;
	int rightMargin = fm.width("000");

	if(m_displayWaterfall && (m_displayHistogram | m_displayLiveSpectrum)) {
		waterfallHeight = height() * m_waterfallShare - 1;
		if(waterfallHeight < 0)
			waterfallHeight = 0;
		if(!m_invertedWaterfall) {
			waterfallTop = topMargin;
			frequencyScaleTop = waterfallTop + waterfallHeight + 1;
			histogramTop = waterfallTop + waterfallHeight + frequencyScaleHeight + 1;
			histogramHeight = height() - topMargin - waterfallHeight - frequencyScaleHeight - bottomMargin;
		} else {
			histogramTop = topMargin;
			histogramHeight = height() - topMargin - waterfallHeight - frequencyScaleHeight - bottomMargin;
			waterfallTop = histogramTop + histogramHeight + frequencyScaleHeight + 1;
			frequencyScaleTop = histogramTop + histogramHeight + 1;
		}

		m_timeScale.setSize(waterfallHeight);
		if(!m_invertedWaterfall)
			m_timeScale.setRange(Unit::Time, (waterfallHeight * m_fftSize) / (float)m_sampleRate, 0);
		else m_timeScale.setRange(Unit::Time, 0, (waterfallHeight * m_fftSize) / (float)m_sampleRate);
		m_powerScale.setSize(histogramHeight);
		m_powerScale.setRange(Unit::Decibel, m_referenceLevel - m_powerRange, m_referenceLevel);
		leftMargin = m_timeScale.getScaleWidth();
		if(m_powerScale.getScaleWidth() > leftMargin)
			leftMargin = m_powerScale.getScaleWidth();
		leftMargin += 2 * M;

		m_frequencyScale.setSize(width() - leftMargin - rightMargin);
		m_frequencyScale.setRange(Unit::Frequency, m_centerFrequency - m_sampleRate / 2, m_centerFrequency + m_sampleRate / 2);

		m_glWaterfallRect = QRectF(
			(float)leftMargin / (float)width(),
			(float)waterfallTop / (float)height(),
			(float)(width() - leftMargin - rightMargin) / (float)width(),
			(float)waterfallHeight / (float)height()
		);

		m_glHistogramRect = QRectF(
			(float)leftMargin / (float)width(),
			(float)histogramTop / (float)height(),
			(float)(width() - leftMargin - rightMargin) / (float)width(),
			(float)histogramHeight / (float)height()
		);

		m_frequencyScaleRect = QRect(
			0,
			frequencyScaleTop,
			width(),
			frequencyScaleHeight
		);
		m_glFrequencyScaleRect = QRectF(
			(float)0,
			(float)frequencyScaleTop / (float)height(),
			(float)1,
			(float)frequencyScaleHeight / (float)height()
		);

		m_glLeftScaleRect = QRectF(
			(float)0,
			(float)0,
			(float)(leftMargin - 1) / (float)width(),
			(float)1
		);
	} else if(m_displayWaterfall) {
		bottomMargin = frequencyScaleHeight;
		waterfallTop = topMargin;
		waterfallHeight = height() - topMargin - frequencyScaleHeight;
		frequencyScaleTop = topMargin + waterfallHeight + 1;
		histogramTop = 0;

		m_timeScale.setSize(waterfallHeight);
		if(!m_invertedWaterfall)
			m_timeScale.setRange(Unit::Time, (waterfallHeight * m_fftSize) / (float)m_sampleRate, 0);
		else m_timeScale.setRange(Unit::Time, 0, (waterfallHeight * m_fftSize) / (float)m_sampleRate);
		leftMargin = m_timeScale.getScaleWidth();
		leftMargin += 2 * M;

		m_frequencyScale.setSize(width() - leftMargin - rightMargin);
		m_frequencyScale.setRange(Unit::Frequency, m_centerFrequency - m_sampleRate / 2.0, m_centerFrequency + m_sampleRate / 2.0);

		m_glWaterfallRect = QRectF(
			(float)leftMargin / (float)width(),
			(float)topMargin / (float)height(),
			(float)(width() - leftMargin - rightMargin) / (float)width(),
			(float)waterfallHeight / (float)height()
		);

		m_frequencyScaleRect = QRect(
			0,
			frequencyScaleTop,
			width(),
			frequencyScaleHeight
		);
		m_glFrequencyScaleRect = QRectF(
			(float)0,
			(float)frequencyScaleTop / (float)height(),
			(float)1,
			(float)frequencyScaleHeight / (float)height()
		);

		m_glLeftScaleRect = QRectF(
			(float)0,
			(float)0,
			(float)(leftMargin - 1) / (float)width(),
			(float)1
		);
	} else if(m_displayHistogram | m_displayLiveSpectrum) {
		bottomMargin = frequencyScaleHeight;
		frequencyScaleTop = height() - bottomMargin;
		histogramTop = topMargin - 1;
		waterfallHeight = 0;
		histogramHeight = height() - topMargin - frequencyScaleHeight;

		m_powerScale.setSize(histogramHeight);
		m_powerScale.setRange(Unit::Decibel, -100, 0);
		leftMargin = m_powerScale.getScaleWidth();
		leftMargin += 2 * M;

		m_frequencyScale.setSize(width() - leftMargin - rightMargin);
		m_frequencyScale.setRange(Unit::Frequency, m_centerFrequency - m_sampleRate / 2, m_centerFrequency + m_sampleRate / 2);

		m_glHistogramRect = QRectF(
			(float)leftMargin / (float)width(),
			(float)histogramTop / (float)height(),
			(float)(width() - leftMargin - rightMargin) / (float)width(),
			(float)(height() - topMargin - frequencyScaleHeight) / (float)height()
		);

		m_frequencyScaleRect = QRect(
			0,
			frequencyScaleTop,
			width(),
			frequencyScaleHeight
		);
		m_glFrequencyScaleRect = QRectF(
			(float)0,
			(float)frequencyScaleTop / (float)height(),
			(float)1,
			(float)frequencyScaleHeight / (float)height()
		);

		m_glLeftScaleRect = QRectF(
			(float)0,
			(float)0,
			(float)(leftMargin - 1) / (float)width(),
			(float)1
		);
	} else {
		leftMargin = 2;
		waterfallHeight = 0;
	}

	// prepare left scales (time and power)
	{
		m_leftMarginPixmap = QPixmap(leftMargin - 1, height());
		m_leftMarginPixmap.fill(Qt::black);
		{
			QPainter painter(&m_leftMarginPixmap);
			painter.setPen(QColor(0xf0, 0xf0, 0xff));
			const ScaleEngine::TickList* tickList;
			const ScaleEngine::Tick* tick;
			if(m_displayWaterfall) {
				tickList = &m_timeScale.getTickList();
				for(int i = 0; i < tickList->count(); i++) {
					tick = &(*tickList)[i];
					if(tick->major) {
						if(tick->textSize > 0)
							painter.drawText(QPointF(leftMargin - M - tick->textSize, waterfallTop + fm.ascent() + tick->textPos), tick->text);
					}
				}
			}
			if(m_displayHistogram || m_displayLiveSpectrum) {
				tickList = &m_powerScale.getTickList();
				for(int i = 0; i < tickList->count(); i++) {
					tick = &(*tickList)[i];
					if(tick->major) {
						if(tick->textSize > 0)
							painter.drawText(QPointF(leftMargin - M - tick->textSize, histogramTop + histogramHeight - tick->textPos - 1), tick->text);
					}
				}
			}
		}
		if(m_leftMarginTextureAllocated)
			deleteTexture(m_leftMarginTexture);
		m_leftMarginTexture = bindTexture(m_leftMarginPixmap);
		m_leftMarginTextureAllocated = true;
	}
	// prepare frequency scale
	if(m_displayWaterfall || m_displayHistogram || m_displayLiveSpectrum){
		m_frequencyPixmap = QPixmap(width(), frequencyScaleHeight);
		m_frequencyPixmap.fill(Qt::transparent);
		{
			QPainter painter(&m_frequencyPixmap);
			painter.setPen(Qt::NoPen);
			painter.setBrush(Qt::black);
			painter.drawRect(leftMargin, 0, width() - leftMargin, frequencyScaleHeight);
			painter.setPen(QColor(0xf0, 0xf0, 0xff));
			const ScaleEngine::TickList* tickList = &m_frequencyScale.getTickList();
			const ScaleEngine::Tick* tick;
			for(int i = 0; i < tickList->count(); i++) {
				tick = &(*tickList)[i];
				if(tick->major) {
					if(tick->textSize > 0)
						painter.drawText(QPointF(leftMargin + tick->textPos, fm.height() + fm.ascent() / 2 - 1), tick->text);
				}
			}

		}
		if(m_frequencyTextureAllocated)
			deleteTexture(m_frequencyTexture);
		m_frequencyTexture = bindTexture(m_frequencyPixmap);
		m_frequencyTextureAllocated = true;
	}

	if(!m_waterfallTextureAllocated) {
		glGenTextures(1, &m_waterfallTexture);
		m_waterfallTextureAllocated = true;
	}
	if(!m_histogramTextureAllocated) {
		glGenTextures(1, &m_histogramTexture);
		m_histogramTextureAllocated = true;
	}

	bool fftSizeChanged = true;
	if(m_waterfallBuffer != NULL)
		fftSizeChanged = m_waterfallBuffer->width() != m_fftSize;
	bool windowSizeChanged = m_waterfallTextureHeight != waterfallHeight;

	if(fftSizeChanged) {
		if(m_waterfallBuffer != NULL) {
			delete m_waterfallBuffer;
			m_waterfallBuffer = NULL;
		}
		m_waterfallBuffer = new QImage(m_fftSize, 256, QImage::Format_ARGB32);
		if(m_waterfallBuffer != NULL) {
			m_waterfallBuffer->fill(qRgb(0x00, 0x00, 0x00));
			m_waterfallBufferPos = 0;
		} else {
			m_fftSize = 0;
			m_changesPending = true;
			return;
		}

		if(m_histogramBuffer != NULL) {
			delete m_histogramBuffer;
			m_histogramBuffer = NULL;
		}
		if(m_histogram != NULL) {
			delete[] m_histogram;
			m_histogram = NULL;
		}
		if(m_histogramHoldoff != NULL) {
			delete[] m_histogramHoldoff;
			m_histogramHoldoff = NULL;
		}

		m_histogramBuffer = new QImage(m_fftSize, 100, QImage::Format_RGB32);
		if(m_histogramBuffer != NULL) {
			m_histogramBuffer->fill(qRgb(0x00, 0x00, 0x00));
		} else {
			m_fftSize = 0;
			m_changesPending = true;
			return;
		}

		m_histogram = new quint8[100 * m_fftSize];
		memset(m_histogram, 0x00, 100 * m_fftSize);
		m_histogramHoldoff = new quint8[100 * m_fftSize];
		memset(m_histogramHoldoff, 0x07, 100 * m_fftSize);

		quint8* data = new quint8[m_fftSize * 100 * 4];
		memset(data, 0x00, m_fftSize * 100 * 4);
		glBindTexture(GL_TEXTURE_2D, m_histogramTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fftSize, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		delete[] data;
	}

	if(fftSizeChanged || windowSizeChanged) {
		m_waterfallTextureHeight = waterfallHeight;
		quint8* data = new quint8[m_fftSize * m_waterfallTextureHeight * 4];
		memset(data, 0x00, m_fftSize * m_waterfallTextureHeight * 4);
		glBindTexture(GL_TEXTURE_2D, m_waterfallTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fftSize, m_waterfallTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		delete[] data;
		m_waterfallTexturePos = 0;
	}

	m_changesPending = false;
}

void GLSpectrum::mouseMoveEvent(QMouseEvent* event)
{
	if(m_displayWaterfall && (m_displayHistogram || m_displayLiveSpectrum)) {
		if(m_frequencyScaleRect.contains(event->pos())) {
			if(m_cursorState == CSNormal) {
				setCursor(Qt::SizeVerCursor);
				m_cursorState = CSSplitter;
			}
		} else {
			if(m_cursorState == CSSplitter) {
				setCursor(Qt::ArrowCursor);
				m_cursorState = CSNormal;
			}
		}
	}

	if(m_cursorState == CSSplitterMoving) {
		float newShare;
		if(!m_invertedWaterfall)
			newShare = (float)(event->y() - m_frequencyScaleRect.height()) / (float)height();
		else newShare = 1.0 - (float)(event->y() + m_frequencyScaleRect.height()) / (float)height();
		if(newShare < 0.1)
			newShare = 0.1;
		else if(newShare > 0.8)
			newShare = 0.8;
		m_waterfallShare = newShare;
		m_changesPending = true;
		update();
		return;
	}
}

void GLSpectrum::mousePressEvent(QMouseEvent* event)
{
	if(event->button() != 1)
		return;

	if(m_cursorState == CSSplitter) {
		grabMouse();
		m_cursorState = CSSplitterMoving;
	}
}

void GLSpectrum::mouseReleaseEvent(QMouseEvent*)
{
	if(m_cursorState == CSSplitterMoving) {
		releaseMouse();
		m_cursorState = CSSplitter;
	}
}

void GLSpectrum::tick()
{
	if(m_displayChanged) {
		m_displayChanged = false;
		update();
	}
}
