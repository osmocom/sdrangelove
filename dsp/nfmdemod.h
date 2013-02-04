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

#ifndef INCLUDE_NFMDEMOD_H
#define INCLUDE_NFMDEMOD_H

#include <vector>
#include "channelizer.h"
#include "nco.h"
#include "interpolator.h"
#include "pidcontroller.h"
#include "hardware/audiofifo.h"

class AudioOutput;

class NFMDemod : public Channelizer {
public:
	NFMDemod();
	~NFMDemod();

	size_t workUnitSize();
	size_t work(SampleVector::const_iterator begin, SampleVector::const_iterator end);

private:
	struct AudioSample {
		qint16 l;
		qint16 r;
	};
	typedef std::vector<AudioSample> AudioVector;

	NCO m_nco;
	Interpolator m_interpolator;
	Real m_distance;

	Complex m_lastSample;

	AudioVector m_audioBuffer;
	uint m_audioBufferFill;
	AudioOutput* m_audioOutput;
	AudioFifo m_audioFifo;
};

#endif // INCLUDE_NFMDEMOD_H
#endif
