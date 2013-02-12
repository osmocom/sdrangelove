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

#ifndef INCLUDE_NFMDEMOD_H
#define INCLUDE_NFMDEMOD_H

#include <vector>
#include "samplesink.h"
#include "nco.h"
#include "interpolator.h"
#include "lowpass.h"
#include "pidcontroller.h"
#include "hardware/audiofifo.h"

class AudioOutput;

class NFMDemod : public SampleSink {
public:
	NFMDemod();
	~NFMDemod();

	void feed(SampleVector::const_iterator begin, SampleVector::const_iterator end, bool firstOfBurst);
	void start();
	void stop();
	void setSampleRate(int sampleRate);
	void handleMessage(Message* cmd);

private:
	struct AudioSample {
		qint16 l;
		qint16 r;
	};
	typedef std::vector<AudioSample> AudioVector;

	Real m_squelchLevel;
	int m_sampleRate;
	int m_frequency;

	NCO m_nco;
	Interpolator m_interpolator;
	Real m_sampleDistanceRemain;
	Lowpass<Real> m_lowpass;

	int m_squelchState;

	Complex m_lastSample;

	AudioVector m_audioBuffer;
	uint m_audioBufferFill;
	AudioOutput* m_audioOutput;
	AudioFifo m_audioFifo;
};

#endif // INCLUDE_NFMDEMOD_H
