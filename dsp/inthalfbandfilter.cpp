#include "inthalfbandfilter.h"

IntHalfbandFilter::IntHalfbandFilter()
{
	for(int i = 0; i < FILTERLEN; i++) {
		m_samples[i][0] = 0;
		m_samples[i][1] = 0;
	}
	m_ptr = 0;
}
