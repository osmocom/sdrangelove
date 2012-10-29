#include <QTime>
#include <stdio.h>
#include "channelizer.h"
#include "hardware/audiooutput.h"

Channelizer::Channelizer()
{
	m_spectrum.configure(128 , 25, FFTWindow::Bartlett);
	m_buffer.resize(2048);
	m_bufferFill = 0;
	m_nco.setFreq(125000, 500000);
	m_interpolator.create(51, 32, 32 * 500000, 150000 / 2);
	m_distance = 500000.0 / 176400.0;

	m_interpolator2.create(19, 8, 8 * 176400, 16000 / 2);
	m_distance2 = 4;

	m_audioFifo.setSize(4, 44100 / 2 * 4);
	m_audioOutput = new AudioOutput;
	m_audioOutput->start(0, 44100, &m_audioFifo);
	m_resampler = 1.0;

	m_resamplerCtrl.setup(0.00001, 0, -0.00001);
}

Channelizer::~Channelizer()
{
	m_audioOutput->stop();
	delete m_audioOutput;
}

void Channelizer::setGLSpectrum(GLSpectrum* glSpectrum)
{
	m_spectrum.setGLSpectrum(glSpectrum);
}

size_t Channelizer::workUnitSize()
{
	return m_buffer.size();
}

size_t Channelizer::work(SampleVector::const_iterator begin, SampleVector::const_iterator end)
{
	int buffered = m_audioOutput->bufferedSamples();

	if(m_audioFifo.fill() < (m_audioFifo.size() / 6)) {
		while(m_audioFifo.fill() < (m_audioFifo.size() / 2)) {
			quint32 d = 0;
			m_audioFifo.write((quint8*)&d, 4);
		}
		qDebug("underflow - fill %d (vs %d)", m_audioFifo.fill(), m_audioFifo.size() / 4 / 2);
	}


	buffered = m_audioOutput->bufferedSamples();
	int fill = m_audioFifo.fill() / 4 + buffered;
	float err = (float)fill / ((m_audioFifo.size() / 4) / 2);

	float ctrl = m_resamplerCtrl.feed(err);
	//float resamplerRate = (ctrl / 1.0);
	float resamplerRate = err;

	if(resamplerRate < 0.9999)
		resamplerRate = 0.9999;
	else if(resamplerRate > 1.0001)
		resamplerRate = 1.0001;
	m_resampler = m_resampler * 0.99 + resamplerRate * 0.01;
	//m_resampler = resamplerRate;

	if(m_resampler < 0.995)
		m_resampler = 0.995;
	else if(m_resampler > 1.005)
		m_resampler = 1.005;

	//qDebug("%lld %5d %f %f %f", QDateTime::currentMSecsSinceEpoch(), fill, ctrl, m_resampler, err);

	struct AudioSample {
		qint16 l;
		qint16 r;
	};

	size_t count = end - begin;
	Complex ci;
	bool consumed;
	bool consumed2;

	for(SampleVector::const_iterator it = begin; it < end; it++) {
		Complex c(it->real() / 32768.0, it->imag() / 32768.0);
		//c *= m_nco.nextIQ();

		consumed = false;
		if(m_interpolator.interpolate(&m_distance, c, &consumed, &ci)) {

			Complex d = ci * conj(m_lastSample);
			m_lastSample = ci;
			//Complex demod(atan2(d.imag(), d.real()) * 0.5, 0);
			Real demod = atan2(d.imag(), d.real()) / M_PI;

			consumed2 = false;
			c = Complex(demod, 0);
			while(!consumed2) {
				if(m_interpolator2.interpolate(&m_distance2, c, &consumed2, &ci)) {
					m_buffer[m_bufferFill++] = Sample(ci.real() * 32767.0, 0.0);

					AudioSample s;
					s.l = ci.real() * 32767.0;
					s.r = s.l;
					m_audioFifo.write((quint8*)&s, 4, 1);

					if(m_bufferFill >= m_buffer.size()) {
						m_spectrum.feed(m_buffer.begin(), m_buffer.end());
						m_bufferFill = 0;
					}
					m_distance2 += 4.0 * m_resampler;
				}
			}
			m_distance += 500000 / 176400.0;
		}
	}

	return count;
}
