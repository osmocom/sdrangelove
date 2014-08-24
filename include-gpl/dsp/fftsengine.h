#ifndef INCLUDE_FFTSENGINE_H
#define INCLUDE_FFTSENGINE_H

#include <ffts/ffts.h>
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
	void* m_imem;
	void* m_iptr;
	void* m_omem;
	void* m_optr;
};

#endif // INCLUDE_FFTSENGINE_H
