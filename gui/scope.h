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

#ifndef INCLUDE_SCOPE_H
#define INCLUDE_SCOPE_H

#include <QWidget>
#include <QPen>
#include <QTimer>
#include <QMutex>
#include "dsp/dsptypes.h"
#include "scaleengine.h"

class Scope: public QWidget {
	Q_OBJECT

public:
	Scope(QWidget* parent = NULL);
	~Scope();

	void newTrace(const std::vector<Real>& trace);

protected:
	// state
	QTimer m_timer;
	QMutex m_mutex;
	bool m_changed;

	// traces
	std::vector<Real> m_trace;

	void paintEvent(QPaintEvent* event);

protected slots:
	void tick();
};

#endif // INCLUDE_SCOPE_H
