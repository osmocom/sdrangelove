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


SoundThread::SoundThread(AudioOutput* out, QObject *parent) :
	m_generator(out)
    ,   m_audioOutput(0)
    ,	QThread(parent)
{
    start();
 
    // Move event processing of SoundThread to this thread
    QObject::moveToThread(this);
}
 
SoundThread::~SoundThread()
{
	stop();
	//delete m_audioOutput;
    quit();
    wait();
}
 
void SoundThread::stop()
{
    m_audioOutput->stop();
}

void SoundThread::kill()
{
    m_audioOutput->stop();
	delete m_audioOutput;
}


void SoundThread::play()
{
    playInt();
}
 
 
void SoundThread::playInt()
{

    //connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), SLOT(stateChanged(QAudio::State)));
    m_generator->start();
    m_audioOutput->start(m_generator);
}

void SoundThread::run()
{
	QAudioFormat     m_format;
	QList<QAudioDeviceInfo> foo = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	QAudioDeviceInfo m_device(foo[0]);

	m_format.setSampleRate(44100);
    m_format.setChannelCount(2);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(m_format)) {
        qWarning("Default format not supported - trying to use nearest");
        m_format = info.nearestFormat(m_format);
    }

    //m_generator = new Generator(m_format, DurationSeconds*1000000, ToneFrequencyHz, this);

    delete m_audioOutput;
    m_audioOutput = 0;
    m_audioOutput = new QAudioOutput(m_device, m_format, this);
	//m_audioOutput->setBufferSize(16384);
 
    exec();
}

AudioOutput::AudioOutput() :
	m_mutex(),
	m_audioFifos(),
	m_sampleRate(0),
	_sfxThread(this)
{
}

AudioOutput::~AudioOutput()
{
	stop();
	QMetaObject::invokeMethod(&_sfxThread, "kill", Qt::QueuedConnection);

	QMutexLocker mutexLocker(&m_mutex);
	for(AudioFifos::iterator it = m_audioFifos.begin(); it != m_audioFifos.end(); ++it)
		delete *it;
	m_audioFifos.clear();
}

bool AudioOutput::start(int device, int rate)
{
	QMutexLocker mutexLocker(&m_mutex);

	for(AudioFifos::iterator it = m_audioFifos.begin(); it != m_audioFifos.end(); ++it)
		(*it)->clear();

	QMetaObject::invokeMethod(&_sfxThread, "play", Qt::QueuedConnection);
	return true;
}


void AudioOutput::start()
{
    open(QIODevice::ReadOnly);
}

void AudioOutput::stop()
{
	QMutexLocker mutexLocker(&m_mutex);
		QMetaObject::invokeMethod(&_sfxThread, "stop", Qt::QueuedConnection);
		qDebug("AudioOutput: stopped");
	close();
}


qint64 AudioOutput::readData(char *data, qint64 len)
{
	int reallen = len/4;
	QMutexLocker mutexLocker(&m_mutex);

	if(m_mixBuffer.size() != reallen * 2) {
		m_mixBuffer.resize(reallen * 2); // allocate 2 qint32 per frame (stereo)
		if(m_mixBuffer.size() != reallen * 2)
			return 0;

	}
	memset(m_mixBuffer.data(), 0x00, m_mixBuffer.size() * sizeof(m_mixBuffer[0])); // start with silence

	// sum up a block from all fifos
	for(AudioFifos::iterator it = m_audioFifos.begin(); it != m_audioFifos.end(); ++it) {
		// use outputBuffer as temp - yes, one memcpy could be saved
		uint samples = (*it)->read((quint8*)data, reallen, 0);
		const qint16* src = (const qint16*)data;
		std::vector<qint32>::iterator dst = m_mixBuffer.begin();
		for(uint i = 0; i < samples; i++) {
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
	qint16* dst = (qint16*)data;
	for(uint i = 0; i < reallen; ++i) {
		qint32 s = *src++;
		if(s < -32768)
			s = -32768;
		else if(s > 32767)
			s = 32767;
		*dst++ = s;
	}

	//qDebug("AudioOutput: read %d", len);
    return len;
}

qint64 AudioOutput::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
	qDebug("AudioOutput: watwatwatwatwat??!");
    return 0;
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

