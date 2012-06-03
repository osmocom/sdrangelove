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
#include <QMutex>
#include <QWaitCondition>
#include "dsptypes.h"
#include "kissfft.h"
#include "fftwindow.h"
#include "settings.h"

class SampleSource;
class SampleFifo;
class Waterfall;
class SpectroHistogram;
class GLSpectrum;

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

	State state() const { return m_state; }
	QString errorMsg();

private:
	typedef kissfft<Real, Complex> KissFFT;

	Settings m_settings;

	State m_state;
	State m_nextState;
	bool m_ready;

	QMutex m_stateWaitMutex;
	QWaitCondition m_stateWaiter;

	QMutex m_errorMsgMutex;
	QString m_errorMsg;

	SampleFifo* m_sampleFifo;
	SampleSource* m_sampleSource;

	int m_sampleRate;
	int m_fftSize;
	int m_fftOverlap;
	int m_fftOverlapSize;
	int m_fftRefillSize;

	Real m_iOfs;
	Real m_qOfs;

	Waterfall* m_waterfall;
	SpectroHistogram* m_spectroHistogram;
	GLSpectrum* m_glSpectrum;

	KissFFT m_fft;
	std::vector<qint16> m_fftSamples;
	std::vector<Complex> m_fftPreWindow;
	std::vector<Complex> m_fftIn;
	std::vector<Complex> m_fftOut;
	std::vector<Real> m_logPowerSpectrum;
	FFTWindow m_fftWindow;

	void run();

	void work();
	void changeState();

	State gotoIdle();
	State gotoRunning();
	State gotoError(const QString& errorMsg);

	bool createMembers();
	void destroyMembers();
};

#endif // INCLUDE_DSPENGINE_H
