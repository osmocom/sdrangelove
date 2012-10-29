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

#ifndef INCLUDE_DSPENGINE_H
#define INCLUDE_DSPENGINE_H

#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include "dsptypes.h"
#include "kissfft.h"
#include "fftwindow.h"
#include "settings.h"
#include "hardware/samplefifo.h"
#include "spectrum.h"

class SampleSource;
class SampleFifo;
class GLSpectrum;
class Channelizer;

class DSPEngine : public QThread {
	Q_OBJECT

public:
	enum State {
		StNotStarted,
		StIdle,
		StRunning,
		StError
	};

	DSPEngine(Settings* settings, QObject* parent = NULL);
	~DSPEngine();

	void setGLSpectrum(GLSpectrum* glSpectrum);

	void start();
	void stop();

	bool startAcquisition();
	void stopAcquistion();

	bool addChannelizer(Channelizer* channelizer);
	bool removeChannelizer(Channelizer* channelizer);

	void triggerDebug();

	State state() const { return m_state; }
	QString errorMsg();

	QString deviceDesc();

private:
	bool m_debugEvent;

	Settings m_settings;

	State m_state;
	State m_nextState;
	bool m_ready;

	QMutex m_stateWaitMutex;
	QWaitCondition m_stateWaiter;

	QMutex m_errorMsgMutex;
	QString m_errorMsg;

	QMutex m_deviceDescMutex;
	QString m_deviceDesc;

	Channelizer* m_channelizerToAdd;
	Channelizer* m_channelizerToRemove;
	typedef std::list<Channelizer*> Channelizers;
	SampleFifo m_sampleFifo;
	Channelizers m_channelizers;

	QTimer* m_timer;
	SampleSource* m_sampleSource;

	int m_sampleRate;

	qint32 m_iOffset;
	qint32 m_qOffset;
	qint32 m_iRange;
	qint32 m_qRange;
	qint32 m_imbalance;

	Spectrum m_spectrum;

	void run();

	void dcOffset(SampleVector::iterator begin, SampleVector::iterator end);
	void imbalance(SampleVector::iterator begin, SampleVector::iterator end);
	void work();

	void applyChannelizers();
	void applyConfig();
	void changeState();

	State gotoIdle();
	State gotoRunning();
	State gotoError(const QString& errorMsg);

	bool createMembers();
	void destroyMembers();

private slots:
	void handleData();
	void tick();
};

#endif // INCLUDE_DSPENGINE_H
