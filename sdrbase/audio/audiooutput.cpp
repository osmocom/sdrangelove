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

#include <string.h>
#include "audio/audiooutput.h"
#include "audio/audiofifo.h"

AudioOutput::AudioOutput() :
	m_mutex(),
	m_stream(NULL),
	m_audioFifos(),
	m_sampleRate(0)
{
}

AudioOutput::~AudioOutput()
{
	stop();

	QMutexLocker mutexLocker(&m_mutex);
	for(AudioFifos::iterator it = m_audioFifos.begin(); it != m_audioFifos.end(); ++it)
		delete *it;
	m_audioFifos.clear();
}

bool AudioOutput::start(int device, int rate)
{
	QMutexLocker mutexLocker(&m_mutex);
	if(m_stream != NULL) {
		Pa_StopStream(m_stream);
		Pa_CloseStream(m_stream);
		m_stream = NULL;
		m_sampleRate = 0;
	}

	for(AudioFifos::iterator it = m_audioFifos.begin(); it != m_audioFifos.end(); ++it)
		(*it)->clear();

	PaStreamParameters outputParameters;
	const PaStreamInfo* streamInfo;
	PaError err;

	outputParameters.device = device;
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paInt16;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	qDebug("AudioOutput: open");
	if((err = Pa_OpenStream(&m_stream, NULL, &outputParameters, rate, 1024, paClipOff, callbackHelper, this)) != paNoError)
		goto failed;

	qDebug("AudioOutput: start");
//	m_streamStartTime = Pa_GetStreamTime(m_stream);
	if((err = Pa_StartStream(m_stream)) != paNoError)
		goto failed;

	streamInfo = Pa_GetStreamInfo(m_stream);
	m_sampleRate = streamInfo->sampleRate;

	qDebug("AudioOutput: playback has started (%s @ %d Hz)", Pa_GetDeviceInfo(outputParameters.device)->name, rate);
	return true;

failed:
	qCritical("AudioOutput: playback failed: %s (%d)", Pa_GetErrorText(err), err);
	Pa_CloseStream(m_stream);
	m_stream = NULL;
	m_sampleRate = 0;
	return false;
}

void AudioOutput::stop()
{
	m_mutex.lock();
	if(m_stream != NULL) {
		m_mutex.unlock();
		Pa_StopStream(m_stream);
		m_mutex.lock();
		Pa_CloseStream(m_stream);
		m_stream = NULL;
		m_sampleRate = 0;
		qDebug("AudioOutput: stopped");
	}
	m_mutex.unlock();
}

void AudioOutput::addFifo(AudioFifo* audioFifo)
{
	QMutexLocker mutexLocker(&m_mutex);

	m_audioFifos.push_back(audioFifo);
}

void AudioOutput::removeFifo(AudioFifo* audioFifo)
{
	QMutexLocker mutexLocker(&m_mutex);

	m_audioFifos.remove(audioFifo);
}

/*
int AudioOutput::bufferedSamples()
{
	return (Pa_GetStreamTime(m_stream) - m_streamStartTime) * m_sampleRate;
}
*/

int AudioOutput::callbackHelper(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	AudioOutput* audioOutput = (AudioOutput*)userData;

	if(audioOutput == NULL)
		return paAbort;

	if(outputBuffer == NULL)
		return paAbort;

	return audioOutput->callback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int AudioOutput::callback(
	const void* inputBuffer,
	void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags)
{
	QMutexLocker mutexLocker(&m_mutex);

	Q_UNUSED(inputBuffer);
	Q_UNUSED(timeInfo);
	Q_UNUSED(statusFlags);

	if(m_mixBuffer.size() != framesPerBuffer * 2) {
		m_mixBuffer.resize(framesPerBuffer * 2); // allocate 2 qint32 per frame (stereo)
		if(m_mixBuffer.size() != framesPerBuffer * 2)
			return paAbort;

	}
	memset(m_mixBuffer.data(), 0x00, m_mixBuffer.size() * sizeof(m_mixBuffer[0])); // start with silence

	// sum up a block from all fifos
	for(AudioFifos::iterator it = m_audioFifos.begin(); it != m_audioFifos.end(); ++it) {
		// use outputBuffer as temp - yes, one memcpy could be saved
		uint samples = (*it)->read((quint8*)outputBuffer, framesPerBuffer, 0);
		const qint16* src = (const qint16*)outputBuffer;
		std::vector<qint32>::iterator dst = m_mixBuffer.begin();
		for(int i = 0; i < samples; i++) {
			*dst += *src;
			++src;
			++dst;
			*dst += *src;
			++src;
			++dst;
		}
	}

	// convert to int16
	std::vector<qint32>::const_iterator  src = m_mixBuffer.begin();
	qint16* dst = (qint16*)outputBuffer;
	for(int i = 0; i < framesPerBuffer; ++i) {
		qint32 s = *src++;
		if(s < -32768)
			s = -32768;
		else if(s > 32767)
			s = 32767;
		*dst++ = s;
	}

//	m_streamStartTime += (PaTime)framesPerBuffer / (PaTime)m_sampleRate;

	return paContinue;
}
