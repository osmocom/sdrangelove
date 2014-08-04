#include <QTime>
#include "dsp/fftsengine.h"

FFTSEngine::FFTSEngine() :
    m_currentplan(ffts_init_1d(1024, 1))
{
    allocate(4096);
}

FFTSEngine::~FFTSEngine()
{
    ffts_free(m_currentplan);
    free(imem);
    free(omem);
}

void FFTSEngine::allocate(int n)
{
    imem = malloc(n*4*2+15);
    iptr = (void*)(((unsigned long)imem+15) & (unsigned long)(~ 0x0F));
    omem = malloc(n*4*2+15);
    optr = (void*)(((unsigned long)omem+15) & (unsigned long)(~ 0x0F));
}

void FFTSEngine::configure(int n, bool inverse)
{
    ffts_free(m_currentplan);
    m_currentplan = ffts_init_1d(n, 1);
}

void FFTSEngine::transform()
{
    ffts_execute(m_currentplan, iptr, optr);
}


Complex* FFTSEngine::in()
{
    return reinterpret_cast<Complex*>(iptr);
}

Complex* FFTSEngine::out()
{
    return reinterpret_cast<Complex*>(optr);
}

