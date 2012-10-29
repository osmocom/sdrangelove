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

#include <QMessageBox>
#include "audiooutput.h"
#include "audiofifo.h"

int AudioOutput::callbackHelper(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	AudioOutput* audioOutput = (AudioOutput*)userData;

	if(outputBuffer == NULL)
		return 0;

	return audioOutput->callback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int AudioOutput::callback(
	const void* inputBuffer,
	void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo*
	timeInfo, PaStreamCallbackFlags statusFlags)
{
	Q_UNUSED(inputBuffer);
	Q_UNUSED(timeInfo);
	Q_UNUSED(statusFlags);

	int needed = framesPerBuffer * 4;
	int done = m_audioFifo->read((quint8*)outputBuffer, needed, 1);

	memset(((quint8*)outputBuffer) + done, 0x00, needed - done);

	m_streamStartTime += (PaTime)framesPerBuffer / (PaTime)m_sampleRate;

	return 0;
}

AudioOutput::AudioOutput() :
	m_stream(NULL),
	m_audioFifo(NULL),
	m_sampleRate(0)
{
}

AudioOutput::~AudioOutput()
{
	stop();
}

bool AudioOutput::start(int device, int rate, AudioFifo* audioFifo)
{
	if(m_stream != NULL)
		stop();

	PaStreamParameters outputParameters;
	const PaStreamInfo* streamInfo;
	PaError err;

	m_audioFifo = audioFifo;

	outputParameters.device = device;
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paInt16;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	qDebug("AudioOutput: open");
	if((err = Pa_OpenStream(&m_stream, NULL, &outputParameters, rate, 1024, paClipOff, callbackHelper, this)) != paNoError)
		goto failed;

	qDebug("AudioOutput: start");
	m_streamStartTime = Pa_GetStreamTime(m_stream);
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
	if(m_stream != NULL) {
		Pa_StopStream(m_stream);
		Pa_CloseStream(m_stream);
		m_stream = NULL;
		m_sampleRate = 0;
		qDebug("AudioOutput: stopped");
	}
}

int AudioOutput::bufferedSamples()
{
	return (Pa_GetStreamTime(m_stream) - m_streamStartTime) * m_sampleRate;
}
