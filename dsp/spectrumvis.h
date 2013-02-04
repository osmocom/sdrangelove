#ifndef INCLUDE_SPECTRUMVIS_H
#define INCLUDE_SPECTRUMVIS_H

#include "samplesink.h"
#include "kissfft.h"
#include "fftwindow.h"

class GLSpectrum;
class MessageQueue;

class SpectrumVis : public SampleSink {
public:
	SpectrumVis(GLSpectrum* glSpectrum = NULL);

	void setGLSpectrum(GLSpectrum* glSpectrum);
	void configure(MessageQueue* msgQueue, int fftSize, int overlapPercent, FFTWindow::Function window);

	void feed(SampleVector::const_iterator begin, SampleVector::const_iterator end);
	void start();
	void stop();
	void handleMessage(Message* cmd);

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
	size_t m_fftBufferFill;

	GLSpectrum* m_glSpectrum;

	void handleConfigure(int fftSize, int overlapPercent, FFTWindow::Function window);
};

#endif // INCLUDE_SPECTRUMVIS_H
