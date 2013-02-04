#if 0

#ifndef INCLUDE_CHANNELIZER_H
#define INCLUDE_CHANNELIZER_H

#include "samplesink.h"
#include "spectrum.h"
#include "nco.h"
#include "interpolator.h"
#include "pidcontroller.h"
#include "hardware/audiofifo.h"

class AudioOutput;

class Channelizer : public SampleSink {
public:
	Channelizer();
	~Channelizer();

#if 0
	void setGLSpectrum(GLSpectrum* glSpectrum);
#endif

	size_t workUnitSize();
	size_t work(SampleVector::const_iterator begin, SampleVector::const_iterator end);

private:
#if 0
	NCO m_nco;
	Interpolator m_interpolator;
	Real m_distance;
	Interpolator m_interpolator2;
	Real m_distance2;

	SampleVector m_buffer;
	size_t m_bufferFill;
	Complex m_lastSample;

	AudioOutput* m_audioOutput;
	AudioFifo m_audioFifo;
	Real m_resampler;
	PIDController m_resamplerCtrl;

	Spectrum m_spectrum;
#endif
};

#endif // INCLUDE_CHANNELIZER_H
#endif
