#include "spectrumvis.h"
#include "gui/glspectrum.h"
#include "dspcommands.h"
#include "util/messagequeue.h"

#define MAX_FFT_SIZE 4096

SpectrumVis::SpectrumVis(GLSpectrum* glSpectrum) :
	SampleSink(),
	m_fftBuffer(MAX_FFT_SIZE),
	m_fftIn(MAX_FFT_SIZE),
	m_fftOut(MAX_FFT_SIZE),
	m_logPowerSpectrum(MAX_FFT_SIZE),
	m_fftBufferFill(0),
	m_glSpectrum(glSpectrum)
{
	handleConfigure(1024, 10, FFTWindow::BlackmanHarris);
}
/*
void SpectrumVis::setGLSpectrum(GLSpectrum* glSpectrum)
{
	m_glSpectrum = glSpectrum;
}
*/
void SpectrumVis::configure(MessageQueue* msgQueue, int fftSize, int overlapPercent, FFTWindow::Function window)
{
	Message* cmd = DSPCmdConfigureSpectrumVis::create(fftSize, overlapPercent, window);
	cmd->submit(msgQueue);
}

void SpectrumVis::feed(SampleVector::const_iterator begin, SampleVector::const_iterator end, bool firstOfBurst)
{
	// if no visualisation is set, send the samples to /dev/null
	if(m_glSpectrum == NULL)
		return;

	while(begin < end) {
		size_t todo = end - begin;
		size_t samplesNeeded = m_refillSize - m_fftBufferFill;

		if(todo >= samplesNeeded) {
			// fill up the buffer
			std::vector<Complex>::iterator it = m_fftBuffer.begin() + m_fftBufferFill;
			for(size_t i = 0; i < samplesNeeded; ++i, ++begin)
				*it++ = Complex(begin->real() / 32768.0, begin->imag() / 32768.0);

			// apply fft window (and copy from m_fftBuffer to m_fftIn)
			m_window.apply(m_fftBuffer, &m_fftIn);

			// calculate FFT
			m_fft.transform(&m_fftIn[0], &m_fftOut[0]);

			// extract power spectrum and reorder buckets
			Real ofs = 20.0f * log10f(1.0f / m_fftSize);
			Real mult = (10.0f / log2f(10.0f));
			for(size_t i = 0; i < m_fftSize; i++) {
				Complex c = m_fftOut[((i + (m_fftSize >> 1)) & (m_fftSize - 1))];
				Real v = c.real() * c.real() + c.imag() * c.imag();
				v = mult * log2f(v) + ofs;
				m_logPowerSpectrum[i] = v;
			}

			// send new data to visualisation
			m_glSpectrum->newSpectrum(m_logPowerSpectrum, m_fftSize);

			// advance buffer respecting the fft overlap factor
			std::copy(m_fftBuffer.begin() + m_refillSize, m_fftBuffer.end(), m_fftBuffer.begin());

			// start over
			m_fftBufferFill = m_overlapSize;
		} else {
			// not enough samples for FFT - just fill in new data and return
			for(std::vector<Complex>::iterator it = m_fftBuffer.begin() + m_fftBufferFill; begin < end; ++begin)
				*it++ = Complex(begin->real() / 32768.0, begin->imag() / 32768.0);
			m_fftBufferFill += todo;
		}
	}
}

void SpectrumVis::start()
{
}

void SpectrumVis::stop()
{
}

void SpectrumVis::setSampleRate(int sampleRate)
{
}

void SpectrumVis::handleMessage(Message* cmd)
{
	switch(cmd->type()) {
		case DSPCmdConfigureSpectrumVis::Type: {
			DSPCmdConfigureSpectrumVis* conf = (DSPCmdConfigureSpectrumVis*)cmd;
			handleConfigure(conf->getFFTSize(), conf->getOverlapPercent(), conf->getWindow());
			break;
		}

		default:
			break;
	}
}

void SpectrumVis::handleConfigure(int fftSize, int overlapPercent, FFTWindow::Function window)
{
	if(fftSize > MAX_FFT_SIZE)
		fftSize = MAX_FFT_SIZE;
	else if(fftSize < 64)
		fftSize = 64;
	if(overlapPercent > 100)
		m_overlapPercent = 100;
	else if(overlapPercent < 0)
		m_overlapPercent = 0;

	m_fftSize = fftSize;
	m_overlapPercent = overlapPercent;
	m_fft.configure(m_fftSize, false);
	m_window.create(window, m_fftSize);
	m_overlapSize = (m_fftSize * m_overlapPercent) / 100;
	m_refillSize = m_fftSize - m_overlapSize;
	m_fftBufferFill = m_overlapSize;
}
