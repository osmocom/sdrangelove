#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <vector>
#include "dsp/interpolator.h"

static std::vector<Real> createPolyphaseLowPass(
	int phaseSteps,
	double gain,
	double sampleRateHz,
	double cutoffFreqHz,
	double transitionWidthHz,
	double oobAttenuationdB)
{
	int ntaps = (int)(oobAttenuationdB * sampleRateHz / (22.0 * transitionWidthHz));
	if((ntaps % 2) == 0)
		ntaps++;
	ntaps *= phaseSteps;

	std::vector<float> taps(ntaps);
	std::vector<float> window(ntaps);

	for(int n = 0; n < ntaps; n++)
		window[n] = 0.54 - 0.46 * cos ((2 * M_PI * n) / (ntaps - 1));

	int M = (ntaps - 1) / 2;
	double fwT0 = 2 * M_PI * cutoffFreqHz / sampleRateHz;
	for(int n = -M; n <= M; n++) {
		if(n == 0) taps[n + M] = fwT0 / M_PI * window[n + M];
			else taps[n + M] =  sin (n * fwT0) / (n * M_PI) * window[n + M];
	}

	double max = taps[0 + M];
	for(int n = 1; n <= M; n++)
		max += 2.0 * taps[n + M];

	gain /= max;

	for(int i = 0; i < ntaps; i++)
		taps[i] *= gain;

	return taps;
}

Interpolator::Interpolator()
{
}

void Interpolator::create(int phaseSteps, double sampleRate, double cutoff)
{
	std::vector<Real> taps = createPolyphaseLowPass(
		phaseSteps, // number of polyphases
		1.0, // gain
		phaseSteps * sampleRate, // sampling frequency
		cutoff, // hz beginning of transition band
		sampleRate / 5.0,  // hz width of transition band
		20.0); // out of band attenuation

	// init state
	m_ptr = 0;
	m_nTaps = taps.size() / phaseSteps;
	m_phaseSteps = phaseSteps;
	m_taps.resize(taps.size());
	m_samples.resize(m_nTaps);
	for(int i = 0; i < m_nTaps; i++)
		m_samples[i] = 0;

	// copy to filter taps
	for(int phase = 0; phase < phaseSteps; phase++) {
		for(int i = 0; i < m_nTaps; i++)
			m_taps[phase * m_nTaps + i] = taps[i * phaseSteps + phase];
	}

	// normalize phase filters
	for(int phase = 0; phase < phaseSteps; phase++) {
		Real sum = 0;
		for(int i = phase * m_nTaps; i < phase * m_nTaps + m_nTaps; i++)
			sum += m_taps[i];
		for(int i = phase * m_nTaps; i < phase * m_nTaps + m_nTaps; i++)
			m_taps[i] /= sum;
	}
}
