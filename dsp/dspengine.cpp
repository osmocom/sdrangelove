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

#include <stdio.h>
#include "dspengine.h"
#include "settings.h"
#include "channelizer.h"
#include "hardware/osmosdrinput.h"
#include "hardware/samplefifo.h"
#include "gui/glspectrum.h"

DSPEngine::DSPEngine(Settings* settings, QObject* parent) :
	QThread(parent),
	m_debugEvent(false),
	m_settings(settings),
	m_state(StNotStarted),
	m_nextState(StIdle),
	m_channelizerToAdd(NULL),
	m_channelizerToRemove(NULL),
	m_sampleFifo(),
	m_sampleSource(NULL)
{
	moveToThread(this);
}

DSPEngine::~DSPEngine()
{
	stop();
	wait();
}

void DSPEngine::setGLSpectrum(GLSpectrum* glSpectrum)
{
	m_spectrum.setGLSpectrum(glSpectrum);
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

bool DSPEngine::addChannelizer(Channelizer* channelizer)
{
	if(!isRunning())
		return false;

	m_stateWaitMutex.lock();
	m_channelizerToAdd = channelizer;
	while(m_channelizerToAdd != NULL)
		m_stateWaiter.wait(&m_stateWaitMutex, 100);
	m_stateWaitMutex.unlock();
	return true;
}

bool DSPEngine::removeChannelizer(Channelizer* channelizer)
{
	if(!isRunning())
		return false;

	m_stateWaitMutex.lock();
	m_channelizerToRemove = channelizer;
	while(m_channelizerToRemove != NULL)
		m_stateWaiter.wait(&m_stateWaitMutex, 100);
	m_stateWaitMutex.unlock();
	return true;
}

void DSPEngine::triggerDebug()
{
	m_debugEvent = true;
}

QString DSPEngine::errorMsg()
{
	QMutexLocker mutexLocker(&m_errorMsgMutex);
	QString res = m_errorMsg;
	res.detach();
	return res;
}

QString DSPEngine::deviceDesc()
{
	QMutexLocker mutexLocker(&m_deviceDescMutex);
	QString res = m_deviceDesc;
	res.detach();
	return res;
}

void DSPEngine::run()
{
	connect(&m_sampleFifo, SIGNAL(dataReady()), this, SLOT(handleData()), Qt::QueuedConnection);

	m_ready = createMembers();

	m_state = StIdle;
	m_stateWaiter.wakeAll();

	exec();
}

void DSPEngine::dcOffset(SampleVector::iterator begin, SampleVector::iterator end)
{
	int count = end - begin;
	int io = 0;
	int qo = 0;

	// sum all sample components
	for(SampleVector::iterator it = begin; it < end; it++) {
		io += it->real();
		qo += it->imag();
	}

	// build a sliding average (el cheapo style)
	m_iOffset = (m_iOffset * 3 + io / count) >> 2;
	m_qOffset = (m_qOffset * 3 + qo / count) >> 2;

	// correct samples
	Sample corr(m_iOffset, m_qOffset);
	for(SampleVector::iterator it = begin; it < end; it++)
		*it -= corr;
}

void DSPEngine::imbalance(SampleVector::iterator begin, SampleVector::iterator end)
{
	int iMin = 0;
	int iMax = 0;
	int qMin = 0;
	int qMax = 0;

	// find value ranges for both I and Q
	// both intervals should be same same size (for a perfect circle)
	for(SampleVector::iterator it = begin; it < end; it++) {
		if(it != begin) {
			if(it->real() < iMin)
				iMin = it->real();
			else if(it->real() > iMax)
				iMax = it->real();
			if(it->imag() < qMin)
				qMin = it->imag();
			else if(it->imag() > qMax)
				qMax = it->imag();

		} else {
			iMin = it->real();
			iMax = it->real();
			qMin = it->imag();
			qMax = it->imag();
		}
	}

	// sliding average (el cheapo again)
	m_iRange = (m_iRange * 15 + (iMax - iMin)) >> 4;
	m_qRange = (m_qRange * 15 + (qMax - qMin)) >> 4;

	// calculate imbalance as Q15.16
	if(m_qRange != 0)
		m_imbalance = ((uint)m_iRange << 16) / (uint)m_qRange;

	// correct imbalance and convert back to signed int 16
	for(SampleVector::iterator it = begin; it < end; it++)
		it->m_imag = (it->m_imag * m_imbalance) >> 16;
}

void DSPEngine::work()
{
	size_t wus;
	size_t maxWorkUnitSize = 0;
	size_t samplesDone = 0;

	wus = m_spectrum.workUnitSize();
	if(wus > maxWorkUnitSize)
		maxWorkUnitSize = wus;
	for(Channelizers::const_iterator it = m_channelizers.begin(); it != m_channelizers.end(); it++) {
		wus = (*it)->workUnitSize();
		if(wus > maxWorkUnitSize)
			maxWorkUnitSize = wus;
	}

	while((m_sampleFifo.fill() > maxWorkUnitSize) && (m_state == m_nextState) && (samplesDone < m_sampleRate)) {
		SampleVector::iterator part1begin;
		SampleVector::iterator part1end;
		SampleVector::iterator part2begin;
		SampleVector::iterator part2end;

		size_t count = m_sampleFifo.readBegin(m_sampleFifo.fill(), &part1begin, &part1end, &part2begin, &part2end);

		// first part of FIFO data
		if(part1begin != part1end) {
			// correct stuff
			if(m_settings.dcOffsetCorrection())
				dcOffset(part1begin, part1end);
			if(m_settings.iqImbalanceCorrection())
				imbalance(part1begin, part1end);
			// feed data to handlers
			m_spectrum.feed(part1begin, part1end);
			for(Channelizers::const_iterator it = m_channelizers.begin(); it != m_channelizers.end(); it++)
				(*it)->feed(part1begin, part1end);
		}
		// second part of FIFO data (used when block wraps around)
		if(part2begin != part2end) {
			// correct stuff
			if(m_settings.dcOffsetCorrection())
				dcOffset(part2begin, part2end);
			if(m_settings.iqImbalanceCorrection())
				imbalance(part2begin, part2end);
			// feed data to handlers
			m_spectrum.feed(part2begin, part2end);
			for(Channelizers::const_iterator it = m_channelizers.begin(); it != m_channelizers.end(); it++)
				(*it)->feed(part1begin, part1end);
		}

		// adjust FIFO pointers
		m_sampleFifo.readCommit(count);
		samplesDone += count;
	}

	// check if the center frequency has changed (has to be responsive)
	if(m_settings.isModifiedCenterFreq())
		m_sampleSource->setCenterFrequency(m_settings.centerFreq());
	// check if decimation has changed (needed to be done here, because to high a sample rate can clog the switch)
	if(m_settings.isModifiedDecimation()) {
		m_sampleSource->setDecimation(m_settings.decimation());
		m_sampleRate = 4000000 / (1 << m_settings.decimation());
		qDebug("New rate: %d", m_sampleRate);
	}
}

void DSPEngine::applyChannelizers()
{
	// check for channelizers to add or remove
	if(m_channelizerToAdd != NULL) {
		m_channelizers.push_back(m_channelizerToAdd);
		m_channelizerToAdd = NULL;
		m_stateWaiter.wakeAll();
	}
	if(m_channelizerToRemove != NULL) {
		m_channelizers.remove(m_channelizerToRemove);
		m_channelizerToRemove = NULL;
		m_stateWaiter.wakeAll();
	}
}

void DSPEngine::applyConfig()
{
	// apply changed configuration
	if(m_settings.isModifiedIQSwap())
		m_sampleSource->setIQSwap(m_settings.iqSwap());

	if(m_settings.isModifiedFFTSize() || m_settings.isModifiedFFTOverlap() || m_settings.isModifiedFFTWindow()) {
		m_spectrum.configure(m_settings.fftSize(), m_settings.fftOverlap(), (FFTWindow::Function)m_settings.fftWindow());
	}

	if(m_settings.isModifiedDCOffsetCorrection()) {
		m_iOffset = 0;
		m_qOffset = 0;
	}

	if(m_settings.isModifiedIQImbalanceCorrection()) {
		m_iRange = 1 << 16;
		m_qRange = 1 << 16;
		m_imbalance = 65536;
	}

	if(m_settings.isModifiedE4000LNAGain())
		((OsmoSDRInput*)m_sampleSource)->setE4000LNAGain(m_settings.e4000LNAGain());
	if(m_settings.isModifiedE4000MixerGain())
		((OsmoSDRInput*)m_sampleSource)->setE4000MixerGain(m_settings.e4000MixerGain());
	if(m_settings.isModifiedE4000MixerEnh())
		((OsmoSDRInput*)m_sampleSource)->setE4000MixerEnh(m_settings.e4000MixerEnh());
	if(m_settings.isModifiedE4000if1())
		((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(1, m_settings.e4000if1());
	if(m_settings.isModifiedE4000if2())
		((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(2, m_settings.e4000if2());
	if(m_settings.isModifiedE4000if3())
		((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(3, m_settings.e4000if3());
	if(m_settings.isModifiedE4000if4())
		((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(4, m_settings.e4000if4());
	if(m_settings.isModifiedE4000if5())
		((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(5, m_settings.e4000if5());
	if(m_settings.isModifiedE4000if6())
		((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(6, m_settings.e4000if6());

	if(m_settings.isModifiedFilterI1() || m_settings.isModifiedFilterI2() || m_settings.isModifiedFilterQ1() || m_settings.isModifiedFilterQ2()) {
		((OsmoSDRInput*)m_sampleSource)->setFilter(
			m_settings.filterI1(),
			m_settings.filterI2(),
			m_settings.filterQ1(),
			m_settings.filterQ2());
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
	m_deviceDescMutex.lock();
	m_deviceDesc.clear();
	m_deviceDescMutex.unlock();

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
		return gotoError("DSP engine is not ready to be started");

	m_settings.isModifiedFFTSize();
	m_settings.isModifiedFFTOverlap();
	m_settings.isModifiedFFTWindow();
	m_settings.isModifiedCenterFreq();
	m_settings.isModifiedIQSwap();
	m_settings.isModifiedDecimation();
	m_settings.isModifiedE4000LNAGain();
	m_settings.isModifiedE4000MixerGain();
	m_settings.isModifiedE4000MixerEnh();
	m_settings.isModifiedE4000if1();
	m_settings.isModifiedE4000if2();
	m_settings.isModifiedE4000if3();
	m_settings.isModifiedE4000if4();
	m_settings.isModifiedE4000if5();
	m_settings.isModifiedE4000if6();
	m_settings.isModifiedFilterI1();
	m_settings.isModifiedFilterI2();
	m_settings.isModifiedFilterQ1();
	m_settings.isModifiedFilterQ2();

	m_sampleRate = 4000000 / (1 << m_settings.decimation());
	qDebug("current rate: %d", m_sampleRate);

	m_spectrum.configure(m_settings.fftSize(), m_settings.fftOverlap(), (FFTWindow::Function)m_settings.fftWindow());

	m_iOffset = 0;
	m_qOffset = 0;
	m_iRange = 1 << 16;
	m_qRange = 1 << 16;

	if(!m_sampleFifo.setSize(2 * 500000))
	   return gotoError("Could not allocate SampleFifo");

	if(!m_sampleSource->startInput(0, 4000000))
		return gotoError("Could not start OsmoSDR");

	m_deviceDescMutex.lock();
	m_deviceDesc = m_sampleSource->deviceDesc();
	m_deviceDescMutex.unlock();

	m_sampleSource->setCenterFrequency(m_settings.centerFreq());
	m_sampleSource->setIQSwap(m_settings.iqSwap());
	m_sampleSource->setDecimation(m_settings.decimation());
	((OsmoSDRInput*)m_sampleSource)->setE4000LNAGain(m_settings.e4000LNAGain());
	((OsmoSDRInput*)m_sampleSource)->setE4000MixerGain(m_settings.e4000MixerGain());
	((OsmoSDRInput*)m_sampleSource)->setE4000MixerEnh(m_settings.e4000MixerEnh());
	((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(1, m_settings.e4000if1());
	((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(2, m_settings.e4000if2());
	((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(3, m_settings.e4000if3());
	((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(4, m_settings.e4000if4());
	((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(5, m_settings.e4000if5());
	((OsmoSDRInput*)m_sampleSource)->setE4000ifStageGain(6, m_settings.e4000if6());
	((OsmoSDRInput*)m_sampleSource)->setFilter(
		m_settings.filterI1(),
		m_settings.filterI2(),
		m_settings.filterQ1(),
		m_settings.filterQ2());

	return StRunning;
}

DSPEngine::State DSPEngine::gotoError(const QString& errorMsg)
{
	QMutexLocker mutexLocker(&m_errorMsgMutex);
	m_errorMsg = errorMsg;
	m_state = StError;
	m_deviceDescMutex.lock();
	m_deviceDesc.clear();
	m_deviceDescMutex.unlock();
	return StError;
}

bool DSPEngine::createMembers()
{
	if((m_timer = new QTimer(this)) == NULL)
		return false;
	connect(m_timer, SIGNAL(timeout()), this, SLOT(tick()));
	m_timer->start(250);

	if((m_sampleSource = new OsmoSDRInput(&m_sampleFifo)) == NULL)
		return false;

	return true;
}

void DSPEngine::destroyMembers()
{
	if(m_sampleSource != NULL) {
		delete m_sampleSource;
		m_sampleSource = NULL;
	}
	m_deviceDescMutex.lock();
	m_deviceDesc.clear();
	m_deviceDescMutex.unlock();
}

void DSPEngine::handleData()
{
	if(m_state == StRunning)
		work();
}

void DSPEngine::tick()
{
	static const char* stateNames[4] = {
		"StNotRunning", "StIdle", "StRunning", "StError"
	};

	if(m_state != m_nextState) {
		changeState();
		m_nextState = m_state;
		m_stateWaiter.wakeAll();

		qDebug("New state: %d: %s", m_state, stateNames[m_state]);
	}

	applyChannelizers();

	switch(m_state) {
		case StNotStarted:
			exit();
			break;

		case StRunning:
			applyConfig();
			break;

		default:
			break;
	}
}
