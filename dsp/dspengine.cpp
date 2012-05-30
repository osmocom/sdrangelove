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

#include "dspengine.h"
#include "settings.h"
#include "hardware/osmosdrinput.h"
#include "hardware/samplefifo.h"
#include "gui/waterfall.h"
#include "gui/spectrohistogram.h"

DSPEngine::DSPEngine(Settings* settings, QObject* parent) :
	QThread(parent),
	m_settings(settings),
	m_state(StNotStarted),
	m_nextState(StIdle),
	m_sampleFifo(NULL),
	m_sampleSource(NULL),
	m_fftSize(512),
	m_fftOverlap(30),
	m_waterfall(NULL),
	m_spectroHistogram(NULL),
	m_curCenterFreq(-1)
{
	moveToThread(this);
}

DSPEngine::~DSPEngine()
{
	stop();
	wait();
}

void DSPEngine::setWaterfall(Waterfall* waterfall)
{
	m_waterfall = waterfall;
}

void DSPEngine::setSpectroHistogram(SpectroHistogram* spectroHistogram)
{
	m_spectroHistogram = spectroHistogram;
}

void DSPEngine::start()
{
	m_stateWaitMutex.lock();
	QThread::start();
	while(m_state != StNotStarted)
		m_stateWaiter.wait(&m_stateWaitMutex, 100);
	m_stateWaitMutex.unlock();
}

void DSPEngine::stop()
{
	m_stateWaitMutex.lock();
	m_nextState = StNotStarted;
	while(m_state != StNotStarted)
		m_stateWaiter.wait(&m_stateWaitMutex, 100);
	m_stateWaitMutex.unlock();
}

bool DSPEngine::startAcquisition()
{
	if((m_state == StRunning) || (m_state == StNotStarted))
		return false;

	m_stateWaitMutex.lock();

	m_fftSize = m_settings->getFFTSize();
	m_fftOverlap = m_settings->getFFTOverlap();

	m_nextState = StRunning;
	while((m_state != StRunning) && (m_state != StError))
		m_stateWaiter.wait(&m_stateWaitMutex, 100);
	m_stateWaitMutex.unlock();

	return m_state == StRunning;
}

void DSPEngine::stopAcquistion()
{
	m_stateWaitMutex.lock();
	m_nextState = StIdle;
	while(m_state == StRunning)
		m_stateWaiter.wait(&m_stateWaitMutex, 100);
	m_stateWaitMutex.unlock();
}

QString DSPEngine::errorMsg()
{
	QMutexLocker mutexLocker(&m_errorMsgMutex);
	QString res = m_errorMsg;
	res.detach();
	return res;
}

void DSPEngine::run()
{
	static const char* stateNames[4] = {
		"StNotRunning", "StIdle", "StRunning", "StError"
	};

	m_ready = createMembers();

	m_state = StIdle;
	m_stateWaiter.wakeAll();

	while(m_state != StNotStarted) {
		if(m_state != m_nextState) {
			changeState();
			m_nextState = m_state;
			m_stateWaiter.wakeAll();

			qDebug("new state: %d: %s", m_state, stateNames[m_state]);
		}

		switch(m_state) {
			case StNotStarted:
				return;

			case StIdle:
				msleep(250);
				break;

			case StRunning:
				work();
				msleep(20);
				break;

			case StError:
				msleep(250);
				break;
		}
	}
}

void DSPEngine::work()
{
	while((m_sampleFifo->fill() > 2 * m_fftRefillSize) && (m_state == m_nextState)) {
		// advance buffer respecting the fft overlap factor
		memmove(&m_fftPreWindow[0], &m_fftPreWindow[m_fftRefillSize], m_fftOverlapSize * sizeof(Complex));
		// read new samples
		m_sampleFifo->read(&m_fftSamples[0], 2 * m_fftRefillSize);
		// convert new samples to Complex()
		qint16* s = &m_fftSamples[0];
		for(int i = m_fftOverlapSize; i < m_fftSize; i++) {
			float q = *s++;
			float j = *s++;
			m_fftPreWindow[i] = Complex(j / 32768.0f, q / 32768.0f);
		}
		// apply fft window
		m_fftWindow.apply(m_fftPreWindow, &m_fftIn);
		// calculate FFT
		m_fft.transform(&m_fftIn[0], &m_fftOut[0]);

		// extract power spectrum and reorder buckets
		for(int i = 0; i < m_fftSize; i++) {
			Complex c = m_fftOut[((i + m_fftOut.size() / 2) % m_fftOut.size())];
			Real v = sqrt((c.real() * c.real() + c.imag() * c.imag()) / (Real)m_fftSize);
			v = 20.0 * log10(v);
			m_logPowerSpectrum[i] = v;
		}

		// send new data to visualisation
		if(m_waterfall != NULL)
			m_waterfall->newSpectrum(m_logPowerSpectrum);
		if(m_spectroHistogram != NULL)
			m_spectroHistogram->newSpectrum(m_logPowerSpectrum);
	}

	if(m_settings->getCenterFreq() != m_curCenterFreq) {
		m_curCenterFreq = m_settings->getCenterFreq();
		m_sampleSource->setCenterFrequency(m_curCenterFreq);
	}
}

void DSPEngine::changeState()
{
	switch(m_nextState) {
		case StNotStarted:
			gotoIdle();
			destroyMembers();
			m_state = StNotStarted;
			break;

		case StIdle:
			m_state = gotoIdle();
			break;

		case StRunning:
			m_state = gotoIdle();
			if(m_state == StIdle)
				m_state = gotoRunning();
			break;

		case StError:
			m_state = StError;
			break;
	}
}

DSPEngine::State DSPEngine::gotoIdle()
{
	switch(m_state) {
		case StNotStarted:
			return StNotStarted;

		case StIdle:
		case StError:
			return StIdle;

		case StRunning:
			break;
	}

	if(!m_ready)
		return StIdle;

	m_sampleSource->stopInput();

	return StIdle;
}

DSPEngine::State DSPEngine::gotoRunning()
{
	switch(m_state) {
		case StNotStarted:
			return StNotStarted;

		case StRunning:
			return StRunning;

		case StIdle:
		case StError:
			break;
	}

	if(!m_ready)
		return gotoError("DSP engine is not ready");

	try {
		m_fft.configure(m_fftSize, false);
		m_fftSamples.resize(2 * m_fftSize);
		m_fftPreWindow.resize(m_fftSize);
		m_fftIn.resize(m_fftSize);
		m_fftOut.resize(m_fftSize);
		m_logPowerSpectrum.resize(m_fftSize);
	} catch(...) {
		return gotoError("out of memory error");
	}
	m_fftOverlapSize = (m_fftSize * m_fftOverlap) / 100;
	m_fftRefillSize = m_fftSize - m_fftOverlapSize;

	if(!m_sampleFifo->setSize(2 * 250 * 500000 / 1000))
	   return gotoError("could not allocate SampleFifo");
	m_fftWindow.create(FFTWindow::Hamming, m_fftSize);

	m_curCenterFreq = -1;

	if(!m_sampleSource->startInput(0, 500000))
		return gotoError("could not start OsmoSDR");

	return StRunning;
}

DSPEngine::State DSPEngine::gotoError(const QString& errorMsg)
{
	QMutexLocker mutexLocker(&m_errorMsgMutex);
	m_errorMsg = errorMsg;
	m_state = StError;
	return StError;
}

bool DSPEngine::createMembers()
{
	if((m_sampleFifo = new SampleFifo()) == NULL)
		return false;
	if((m_sampleSource = new OsmoSDRInput(m_sampleFifo)) == NULL)
		return false;

	return true;
}

void DSPEngine::destroyMembers()
{
	if(m_sampleSource != NULL) {
		delete m_sampleSource;
		m_sampleSource = NULL;
	}
	if(m_sampleFifo != NULL) {
		delete m_sampleFifo;
		m_sampleFifo = NULL;
	}
}
