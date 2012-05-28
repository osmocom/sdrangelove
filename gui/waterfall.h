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

#ifndef INCLUDE_WATERFALL_H
#define INCLUDE_WATERFALL_H

#if 0
#include <QWidget>
#include <QMutex>
#include <QTimer>
#include "dsp/dsptypes.h"

class Waterfall : public QWidget {
	Q_OBJECT

public:
	Waterfall(QWidget* parent = NULL);
	~Waterfall();

	void newSpectrum(const std::vector<Real>& spectrum);

private:
	QTimer m_timer;
	QMutex m_mutex;
	int m_fftSize;
	QImage* m_image;
	QRgb m_palette[240];
	int m_pos;

	void createImage();

	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent*);

private slots:
	void refresh();
};
#endif

#include <QGLWidget>
#include <QMutex>
#include <QTimer>
#include "dsp/dsptypes.h"

class Waterfall : public QGLWidget {
	Q_OBJECT

public:
	Waterfall(QWidget* parent = NULL);
	~Waterfall();

	void newSpectrum(const std::vector<Real>& spectrum);

private:
	QTimer m_timer;
	QMutex m_mutex;
	int m_fftSize;
	QImage* m_image;
	QRgb m_palette[240];
	int m_pos;

	bool m_textureAllocated;
	GLuint m_texture;
	int m_textureHeight;
	bool m_resizeTexture;
	int m_texPos;

	void createImage();

//	void paintEvent(QPaintEvent*);
//	void resizeEvent(QResizeEvent*);

	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
	void resizeTexture();

private slots:
	void refresh();
};

#endif // INCLUDE_WATERFALL_H
