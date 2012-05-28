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

#include "samplefifo.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void SampleFifo::create(int s)
{
	if(m_data != NULL) {
		delete[] m_data;
		m_data = NULL;
	}

	m_size = 0;
	m_fill = 0;
	m_head = 0;
	m_tail = 0;

	if((m_data = new qint16[s]) == NULL) {
		qCritical("SampleFifo: out of memory");
		return;
	}

	m_size = s;
}

SampleFifo::SampleFifo()
{
	m_suppressed = -1;
	m_data = NULL;
	m_size = 0;
	m_fill = 0;
	m_head = 0;
	m_tail = 0;
}

SampleFifo::SampleFifo(int size)
{
	m_suppressed = -1;
	m_data = NULL;

	create(size);
}

SampleFifo::~SampleFifo()
{
	QMutexLocker mutexLocker(&m_mutex);

	if(m_data != NULL) {
		delete[] m_data;
		m_data = NULL;
	}

	m_size = 0;
}

bool SampleFifo::setSize(int size)
{
	create(size);

	return m_data != NULL;
}

int SampleFifo::write(const qint16* samples, int count)
{
	QMutexLocker mutexLocker(&m_mutex);
	int total;
	int remaining;
	int len;

	if(m_data == NULL)
		return 0;

	total = MIN(count, m_size - m_fill);
	if(total < count) {
		if(m_suppressed < 0) {
			m_suppressed = 0;
			m_msgRateTimer.start();
			qCritical("SampleFifo: overflow - dropping %d samples", count - total);
		} else {
			if(m_msgRateTimer.elapsed() > 2500) {
				qCritical("SampleFifo: %d messages dropped", m_suppressed);
				qCritical("SampleFifo: overflow - dropping %d samples", count - total);
				m_suppressed = -1;
			} else {
				m_suppressed++;
			}
		}
	}

	remaining = total;
	while(remaining > 0) {
		len = MIN(remaining, m_size - m_tail);
		memcpy(m_data + m_tail, samples, len * sizeof(qint16));
		m_tail += len;
		m_tail %= m_size;
		m_fill += len;
		samples += len;
		remaining -= len;
	}

	return total;
}

int SampleFifo::read(qint16* samples, int count)
{
	QMutexLocker mutexLocker(&m_mutex);
	int total;
	int remaining;
	int len;

	if(m_data == NULL)
		return 0;

	total = MIN(count, m_fill);
	if(total < count)
		qCritical("SampleFifo: underflow - missing %d samples", count - total);

	remaining = total;
	while(remaining > 0) {
		len = MIN(remaining, m_size - m_head);
		memcpy(samples, m_data + m_head, len * sizeof(qint16));
		m_head += len;
		m_head %= m_size;
		m_fill -= len;
		samples += len;
		remaining -= len;
	}

	return total;
}

int SampleFifo::drain(int count)
{
	QMutexLocker mutexLocker(&m_mutex);

	if(count > m_fill)
		count = m_fill;
	m_head = (m_head + count) & m_size;
	m_fill -= count;

	return count;
}
