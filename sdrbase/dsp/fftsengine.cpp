#include <QTime>
#include "dsp/fftsengine.h"

FFTSEngine::FFTSEngine() :
	m_currentplan(ffts_init_1d(1024, -1))
{
	allocate(8192);
}

FFTSEngine::~FFTSEngine()
{
	ffts_free(m_currentplan);
	free(m_imem);
	free(m_omem);
}

void FFTSEngine::allocate(int n)
{
	m_imem = malloc(n * sizeof(Real) * 2 + 15);
	m_iptr = (void*)(((unsigned long)m_imem + 15) & (unsigned long)(~0x0f));
	m_omem = malloc(n * sizeof(Real) * 2 + 15);
	m_optr = (void*)(((unsigned long)m_omem + 15) & (unsigned long)(~0x0f));
}

void FFTSEngine::configure(int n, bool inverse)
{
	ffts_free(m_currentplan);
	m_currentplan = ffts_init_1d(n, inverse ? 1 : -1);
}

void FFTSEngine::transform()
{
	ffts_execute(m_currentplan, m_iptr, m_optr);
}

Complex* FFTSEngine::in()
{
	return reinterpret_cast<Complex*>(m_iptr);
}

Complex* FFTSEngine::out()
{
	return reinterpret_cast<Complex*>(m_optr);
}
