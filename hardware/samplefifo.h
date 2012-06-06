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

#ifndef INCLUDE_SAMPLEFIFO_H
#define INCLUDE_SAMPLEFIFO_H

#include <QObject>
#include <QMutex>
#include <QTime>

class SampleFifo : public QObject {
	Q_OBJECT

private:
	QMutex m_mutex;
	QTime m_msgRateTimer;
	int m_suppressed;

	qint16* m_data;

	int m_size;
	int m_fill;
	int m_head;
	int m_tail;

	void create(int s);

public:
	SampleFifo(QObject* parent = NULL);
	SampleFifo(int size, QObject* parent = NULL);
	~SampleFifo();

	bool setSize(int size);
	inline int fill() { return m_fill; }

	int write(const qint16* samples, int count);
	int read(qint16* samples, int count);

	int drain(int count);

signals:
	void dataReady();
};

#endif // INCLUDE_SAMPLEFIFO_H
