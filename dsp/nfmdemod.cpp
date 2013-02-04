#if 0
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

#include <QTime>
#include <stdio.h>
#include "nfmdemod.h"
#include "hardware/audiooutput.h"

NFMDemod::NFMDemod()
{
	m_nco.setFreq(-100000, 500000);
	m_interpolator.create(51, 32, 32 * 500000, 25000 / 2);
	m_distance = 500000.0 / 44100.0;

	m_audioFifo.setSize(4, 44100 / 4);
	m_audioOutput = new AudioOutput;
	m_audioOutput->start(0, 44100, &m_audioFifo);

	m_audioBuffer.resize(256);
	m_audioBufferFill = 0;
}

NFMDemod::~NFMDemod()
{
	m_audioOutput->stop();
	delete m_audioOutput;
}

size_t NFMDemod::workUnitSize()
{
	return 1024;
}

size_t NFMDemod::work(SampleVector::const_iterator begin, SampleVector::const_iterator end)
{
	size_t count = end - begin;

	Complex ci;
	bool consumed;

	for(SampleVector::const_iterator it = begin; it < end; ++it) {
		Complex c(it->real() / 32768.0, it->imag() / 32768.0);
		c *= m_nco.nextIQ();

		consumed = false;
		if(m_interpolator.interpolate(&m_distance, c, &consumed, &ci)) {

			Complex d = ci * conj(m_lastSample);
			m_lastSample = ci;
			//Complex demod(atan2(d.imag(), d.real()) * 0.5, 0);
			Real demod = atan2(d.imag(), d.real()) / M_PI;
			qint16 sample = demod * 32767 * 3;

			m_audioBuffer[m_audioBufferFill].l = sample;
			m_audioBuffer[m_audioBufferFill].r = sample;
			++m_audioBufferFill;
			if(m_audioBufferFill >= m_audioBuffer.size()) {
				if(m_audioFifo.write((const quint8*)&m_audioBuffer[0], m_audioBufferFill) != m_audioBufferFill)
					qDebug("lost samples");
				m_audioBufferFill = 0;
			}

			m_distance += 500000 / 44100.0 * m_audioOutput->rateCorrection();
		}
	}
	if(m_audioFifo.write((const quint8*)&m_audioBuffer[0], m_audioBufferFill) != m_audioBufferFill)
		qDebug("lost samples");
	m_audioBufferFill = 0;

	return count;
}
#endif
