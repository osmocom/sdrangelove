#ifndef INCLUDE_LOWPASS_H
#define INCLUDE_LOWPASS_H

#include "dsptypes.h"

class Lowpass {
public:
	Lowpass();

	void create(int nTaps, double sampleRate, double cutoff);

	Complex filter(Complex sample)
	{
		Real rAcc = 0;
		Real iAcc = 0;
		int a = m_ptr;
		int b = a - 1;
		int i;

		m_samples[m_ptr] = sample;

		while(b < 0)
			b += m_samples.size();

		for(i = 0; i < (int)m_taps.size() - 1; i++) {
			rAcc += (m_samples[a].real() + m_samples[b].real()) * m_taps[i];
			iAcc += (m_samples[a].imag() + m_samples[b].imag()) * m_taps[i];
			a++;
			while(a >= (int)m_samples.size())
				a -= m_samples.size();
			b--;
			while(b < 0)
				b += m_samples.size();
		}
		rAcc += m_samples[a].real() * m_taps[i];
		iAcc += m_samples[a].imag() * m_taps[i];

		m_ptr++;
		while(m_ptr >= (int)m_samples.size())
			m_ptr -= m_samples.size();

		return Complex(rAcc, iAcc);
	}

private:
	std::vector<Real> m_taps;
	std::vector<Complex> m_samples;
	int m_ptr;
};

#endif // INCLUDE_LOWPASS_H
