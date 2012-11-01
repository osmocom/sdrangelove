#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include "interpolator.h"

Interpolator::Interpolator()
{
}

void Interpolator::create(int nTaps, int phaseSteps, double sampleRate, double cutoff)
{
	double wc = 2.0 * M_PI * cutoff;
	double Wc = wc / sampleRate;
	int numTaps = nTaps * phaseSteps;
	int i;
	Real sum;

	// make room
	m_samples.resize(nTaps * 2);
	for(int i = 0; i < nTaps; i++)
		m_samples[i] = 0;
	m_ptr = 0;
	m_nTaps = nTaps;
	m_phaseSteps = phaseSteps;
	m_taps.resize(numTaps);

	std::vector<Real> taps;
	taps.resize(numTaps);

	// generate Sinc filter core
	for(i = 0; i < numTaps; i++) {
		if(i == (numTaps - 1) / 2)
			taps[i] = Wc / M_PI;
		else
			taps[i] = sin(((double)i - ((double)numTaps - 1.0) / 2.0) * Wc) / (((double)i - ((double)numTaps - 1.0) / 2.0) * M_PI);
	}

	// apply Hamming window
	for(i = 0; i < numTaps; i++)
		taps[i] *= 0.54 + 0.46 * cos((2.0 * M_PI * ((double)i - ((double)numTaps - 1.0) / 2.0)) / (double)numTaps);

	// copy to filter taps
	for(int phase = 0; phase < phaseSteps; phase++) {
		for(int i = 0; i < nTaps; i++)
			m_taps[phase * nTaps + i] = taps[i* phaseSteps + phase];
	}

	// normalize phase filters
	for(int phase = 0; phase < phaseSteps; phase++) {
		sum = 0;
		for(i = phase * nTaps; i < phase * nTaps + nTaps; i++)
			sum += m_taps[i];
		for(i = phase * nTaps; i < phase * nTaps + nTaps; i++)
			m_taps[i] /= sum;
	}
}
