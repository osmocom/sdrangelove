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

static FILE* logFile = NULL;
static int logLine = 0;

NFMDemod::NFMDemod()
{
	m_squelchLevel = pow(10.0, -50.0 / 20.0);
	m_sampleRate = 500000;
	m_frequency = -50000;
	m_squelchLevel *= m_squelchLevel;

	m_nco.setFreq(m_frequency, m_sampleRate);
	m_interpolator.create(51, 32, 32 * m_sampleRate, 12500 / 2);
	m_sampleDistanceRemain = (Real)m_sampleRate / 44100.0;

	m_lowpass.create(21, 44100, 3000);
	m_audioFifo.setSize(4, 44100 / 4);
	m_audioOutput = new AudioOutput;

	m_audioBuffer.resize(256);
	m_audioBufferFill = 0;

	qDebug("- [%g] -", m_squelchLevel);


	//logFile = fopen("/tmp/log.txt", "wb");
}

NFMDemod::~NFMDemod()
{
	m_audioOutput->stop();
	delete m_audioOutput;
}

void NFMDemod::feed(SampleVector::const_iterator begin, SampleVector::const_iterator end, bool firstOfBurst)
{
	size_t count = end - begin;

	Complex ci;
	bool consumed;

	if((firstOfBurst) && (logFile != NULL)) {
		uint a = m_audioFifo.fill();
		uint b = m_audioOutput->bufferedSamples();
		fprintf(logFile, "%u %u\n", logLine++, a + b);
		fflush(logFile);
	}

	for(SampleVector::const_iterator it = begin; it < end; ++it) {
		Complex c(it->real() / 32768.0, it->imag() / 32768.0);
		c *= m_nco.nextIQ();

		consumed = false;
		if(m_interpolator.interpolate(&m_sampleDistanceRemain, c, &consumed, &ci)) {
			if((ci.real() * ci.real() + ci.imag() * ci.imag()) >= m_squelchLevel)
				m_squelchState = m_sampleRate / 100;

			if(m_squelchState > 0) {
				m_squelchState--;
				Complex d = ci * conj(m_lastSample);
				m_lastSample = ci;
				Real demod = atan2(d.imag(), d.real()) / M_PI;
				demod = m_lowpass.filter(demod);
				qint16 sample = demod * 32767 * 4;

				m_audioBuffer[m_audioBufferFill].l = sample;
				m_audioBuffer[m_audioBufferFill].r = sample;
				++m_audioBufferFill;
				if(m_audioBufferFill >= m_audioBuffer.size()) {
					if(m_audioFifo.write((const quint8*)&m_audioBuffer[0], m_audioBufferFill) != m_audioBufferFill)
						qDebug("lost samples");
					m_audioBufferFill = 0;
				}
			}

			m_sampleDistanceRemain += (Real)m_sampleRate / 44100.0;
		}
	}
	if(m_audioFifo.write((const quint8*)&m_audioBuffer[0], m_audioBufferFill) != m_audioBufferFill)
		qDebug("lost samples");
	m_audioBufferFill = 0;
}

void NFMDemod::start()
{
	m_audioOutput->start(0, 44100, &m_audioFifo);
	m_squelchState = 0;
}

void NFMDemod::stop()
{
	m_audioOutput->stop();
}

void NFMDemod::setSampleRate(int sampleRate)
{
	m_sampleRate = sampleRate;
	m_nco.setFreq(m_frequency, m_sampleRate);
	m_interpolator.create(51, 32, 32 * m_sampleRate, 12500 / 2);
	m_sampleDistanceRemain = m_sampleRate / 44100.0;
	m_squelchState = 0;
}

void NFMDemod::handleMessage(Message* cmd)
{
}
