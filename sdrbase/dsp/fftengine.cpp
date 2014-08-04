#include "dsp/fftengine.h"
#ifdef USE_KISSFFT
#include "dsp/kissengine.h"
#endif
#ifdef USE_FFTW
#include "dsp/fftwengine.h"
#endif // USE_FFTW
#ifdef USE_FFTS
#include "dsp/fftsengine.h"
#endif // USE_FFTS

FFTEngine::~FFTEngine()
{
}

FFTEngine* FFTEngine::create()
{
#ifdef USE_FFTW
	qDebug("FFT: using FFTW engine");
	return new FFTWEngine;
#endif // USE_FFTW
#ifdef USE_KISSFFT
	qDebug("FFT: using KissFFT engine");
	return new KissEngine;
#endif // USE_KISSFFT
#ifdef USE_FFTS
    qDebug("FFT: using FFTS engine");
    return new FFTSEngine;
#endif // USE_FFTS
	qCritical("FFT: no engine built");
	return NULL;
}
