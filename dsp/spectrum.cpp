#include "spectrum.h"
#include "gui/glspectrum.h"

#define MAX_FFT_SIZE 4096

Spectrum::Spectrum() :
	m_fftBuffer(MAX_FFT_SIZE),
	m_fftIn(MAX_FFT_SIZE),
	m_fftOut(MAX_FFT_SIZE),
	m_logPowerSpectrum(MAX_FFT_SIZE),
	m_glSpectrum(NULL)
{
}

void Spectrum::setGLSpectrum(GLSpectrum* glSpectrum)
{
	m_glSpectrum = glSpectrum;
}

void Spectrum::configure(int fftSize, int overlapPercent, FFTWindow::Function window)
{
	if(fftSize > MAX_FFT_SIZE)
		fftSize = MAX_FFT_SIZE;

	m_fftSize = fftSize;
	m_overlapPercent = overlapPercent;
	m_fft.configure(m_fftSize, false);
	m_window.create(window, m_fftSize);
	m_overlapSize = (m_fftSize * m_overlapPercent) / 100;
	m_refillSize = m_fftSize - m_overlapSize;
}

size_t Spectrum::workUnitSize()
{
	return m_refillSize;
}

size_t Spectrum::work(SampleVector::const_iterator begin, SampleVector::const_iterator end)
{
	size_t count = end - begin;

	// if no visualisation is set, send the samples to /dev/null
	if(m_glSpectrum == NULL)
		return count;

	// advance buffer respecting the fft overlap factor
	std::copy(m_fftBuffer.begin() + m_refillSize, m_fftBuffer.end(), m_fftBuffer.begin());

	// fill in new data
	for(std::vector<Complex>::iterator it = m_fftBuffer.begin() + m_overlapSize; begin < end; begin++)
		*it++ = Complex(begin->real() / 32768.0, begin->imag() / 32768.0);

	// apply fft window
	m_window.apply(m_fftBuffer, &m_fftIn);

	// calculate FFT
	m_fft.transform(&m_fftIn[0], &m_fftOut[0]);

	// extract power spectrum and reorder buckets
	for(size_t i = 0; i < m_fftSize; i++) {
		Complex c = m_fftOut[((i + (m_fftSize >> 1)) & (m_fftSize - 1))];
		Real v = sqrt(c.real() * c.real() + c.imag() * c.imag());
		v /= (Real)m_fftSize;
		v = 20.0 * log10(v);
		m_logPowerSpectrum[i] = v;
	}

	// send new data to visualisation
	m_glSpectrum->newSpectrum(m_logPowerSpectrum, m_fftSize);

	// we used up the whole work unit
	return count;
}
