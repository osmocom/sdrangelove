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

#include <QIODevice>
#include <QThread>
#include <QMutex>
#include <QAudioOutput>
#include <list>
#include <vector>
#include "util/export.h"

class AudioFifo;
class AudioOutput;

class SoundThread : public QThread
{
    Q_OBJECT
public:
    explicit SoundThread(AudioOutput* out, QObject *parent = 0);
    ~SoundThread();
    void run();
 
signals:
 
public slots:
    void play();
	void stop();
	void kill();
private:
    void playInt();
 
	AudioOutput*       m_generator;
    QAudioOutput*    m_audioOutput;

};

class SDRANGELOVE_API AudioOutput : public QIODevice{
	Q_OBJECT
public:
	AudioOutput();
	~AudioOutput();

	void start();
	bool start(int device, int rate);
	void stop();

	void addFifo(AudioFifo* audioFifo);
	void removeFifo(AudioFifo* audioFifo);

	qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
	//int bufferedSamples();

private:
	QMutex m_mutex;
	//AudioFifo* m_audioFifo;
	typedef std::list<AudioFifo*> AudioFifos;
	AudioFifos m_audioFifos;
	std::vector<qint32> m_mixBuffer;

	int m_sampleRate;
	//PaTime m_streamStartTime;

	SoundThread _sfxThread;

	int callback(void* outputBuffer,
		unsigned long framesPerBuffer);
};

#endif // INCLUDE_AUDIOOUTPUT_H
