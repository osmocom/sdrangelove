#ifndef INCLUDE_FFTSEngine_H
#define INCLUDE_FFTSEngine_H

#include <QMutex>
#include <ffts/ffts.h>
#include <list>
#include "dsp/fftengine.h"

class FFTSEngine : public FFTEngine {
public:
    FFTSEngine();
    ~FFTSEngine();

	void configure(int n, bool inverse);
	void transform();

	Complex* in();
	Complex* out();

protected:
    void allocate(int n);
    ffts_plan_t* m_currentplan;
    void *imem;
    void *iptr;
    void *omem;
    void *optr;
};

#endif // INCLUDE_FFTSEngine_H
