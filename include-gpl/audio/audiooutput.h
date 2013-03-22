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

#ifndef INCLUDE_AUDIOOUTPUT_H
#define INCLUDE_AUDIOOUTPUT_H

#include <QMutex>
#include <list>
#include <vector>
#include "portaudio.h"

class AudioFifo;

class AudioOutput {
public:
	AudioOutput();
	~AudioOutput();

	bool start(int device, int rate);
	void stop();

	void addFifo(AudioFifo* audioFifo);
	void removeFifo(AudioFifo* audioFifo);

	//int bufferedSamples();

private:
	QMutex m_mutex;
	PaStream* m_stream;
	//AudioFifo* m_audioFifo;
	typedef std::list<AudioFifo*> AudioFifos;
	AudioFifos m_audioFifos;
	std::vector<qint32> m_mixBuffer;

	int m_sampleRate;
	//PaTime m_streamStartTime;

	static int callbackHelper(
		const void* inputBuffer,
		void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);

	int callback(
		const void* inputBuffer,
		void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags);
};

#endif // INCLUDE_AUDIOOUTPUT_H
