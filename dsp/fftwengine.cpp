#include "fftwengine.h"

FFTWEngine::FFTWEngine() :
	m_plans(),
	m_currentPlan(NULL)
{
}

FFTWEngine::~FFTWEngine()
{
	freeAll();
}

void FFTWEngine::configure(int n, bool inverse)
{
	for(Plans::const_iterator it = m_plans.begin(); it != m_plans.end(); ++it) {
		if(((*it)->n == n) && ((*it)->inverse == inverse)) {
			m_currentPlan = *it;
			return;
		}
	}

	m_currentPlan = new Plan;
	m_currentPlan->n = n;
	m_currentPlan->inverse = inverse;
	m_currentPlan->in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * n);
	m_currentPlan->out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * n);
	m_currentPlan->plan = fftwf_plan_dft_1d(n, m_currentPlan->in, m_currentPlan->out, inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_PATIENT);
	m_plans.push_back(m_currentPlan);
}

void FFTWEngine::transform()
{
	fftwf_execute(m_currentPlan->plan);
}

Complex* FFTWEngine::in()
{
	return reinterpret_cast<Complex*>(m_currentPlan->in);
}

Complex* FFTWEngine::out()
{
	return reinterpret_cast<Complex*>(m_currentPlan->out);
}

void FFTWEngine::freeAll()
{
	for(Plans::iterator it = m_plans.begin(); it != m_plans.end(); ++it) {
		fftwf_destroy_plan((*it)->plan);
		fftwf_free((*it)->in);
		fftwf_free((*it)->out);
		delete *it;
	}
	m_plans.clear();
}
