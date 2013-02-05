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

#ifndef INCLUDE_GLSCOPE_H
#define INCLUDE_GLSCOPE_H

#include <QGLWidget>
#include <QPen>
#include <QTimer>
#include <QMutex>
#include "dsp/dsptypes.h"
#include "../dsp/scopevis.h"

class DSPEngine;
class ScopeVis;

class GLScope: public QGLWidget {
	Q_OBJECT

public:
	GLScope(QWidget* parent = NULL);
	~GLScope();

	void setDSPEngine(DSPEngine* dspEngine);
	void setAmp(Real amp);
	void setTimeStep(int timeStep);
	void setTimeOfsProMill(int timeOfsProMill);

	void newTrace(const std::vector<Complex>& trace, int sampleRate);

	int getTraceSize() const { return m_trace.size(); }

signals:
	void traceSizeChanged(int);

private:
	// state
	QTimer m_timer;
	QMutex m_mutex;
	bool m_changed;
	bool m_changesPending;

	// traces
	std::vector<Complex> m_trace;
	int m_oldTraceSize;
	int m_sampleRate;

	// sample sink
	DSPEngine* m_dspEngine;
	ScopeVis* m_scopeVis;

	// config
	Real m_amp;
	int m_timeStep;
	int m_timeOfsProMill;
	ScopeVis::TriggerChannel m_triggerChannel;
	Real m_triggerLevelHigh;
	Real m_triggerLevelLow;

	// graphics stuff
	QRectF m_glScopeRectI;
	QRectF m_glScopeRectQ;

	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

	void mousePressEvent(QMouseEvent*);

	void applyChanges();

protected slots:
	void tick();
};

#endif // INCLUDE_GLSCOPE_H
