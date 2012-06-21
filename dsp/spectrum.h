#ifndef INCLUDE_SPECTRUM_H
#define INCLUDE_SPECTRUM_H

#include "samplesink.h"
#include "kissfft.h"
#include "fftwindow.h"

class GLSpectrum;

class Spectrum : public SampleSink {
public:
	Spectrum();

	void setGLSpectrum(GLSpectrum* glSpectrum);
	void configure(int fftSize, int overlapPercent, FFTWindow::Function window);

	size_t workUnitSize();
	size_t work(SampleVector::const_iterator begin, SampleVector::const_iterator end);

private:
	typedef kissfft<Real, Complex> KissFFT;
	KissFFT m_fft;
	FFTWindow m_window;

	std::vector<Complex> m_fftBuffer;
	std::vector<Complex> m_fftIn;
	std::vector<Complex> m_fftOut;
	std::vector<Real> m_logPowerSpectrum;

	size_t m_fftSize;
	size_t m_overlapPercent;
	size_t m_overlapSize;
	size_t m_refillSize;

	GLSpectrum* m_glSpectrum;
};

#endif // INCLUDE_SPECTRUM_H
