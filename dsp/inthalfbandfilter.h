#ifndef INCLUDE_INTHALFBANDFILTER_H
#define INCLUDE_INTHALFBANDFILTER_H

#include <QtGlobal>

// uses Q1.14 format internally, input and output are S16

#define SHIFT 14
#define FILTERLEN 16

class IntHalfbandFilter {
public:
	IntHalfbandFilter();

	// process one sample
	bool workDecimate(qint16* i, qint16* q)
	{
		// insert sample into ring-buffer
		m_samples[m_ptr][0] = *i;
		m_samples[m_ptr][1] = *q;

		if((m_ptr & 1) != 0) {
			// save result
			doFIR(i, q);

			// advance write-pointer
			m_ptr = (m_ptr + (FILTERLEN - 1)) % FILTERLEN;

			// tell caller we have a new sample
			return true;
		} else {
			// advance write-pointer
			m_ptr = (m_ptr + (FILTERLEN - 1)) % FILTERLEN;

			return false;
		}
	}

protected:
	qint16 m_samples[FILTERLEN][2];
	int m_ptr;

	void doFIR(qint16* iResult, qint16* qResult)
	{
		// coefficents
		static const qint32 COEFF[FILTERLEN / 2] = {
			-0.026129089617881795515330622947658412158 * (1 << SHIFT),
			-0.012950412689549032144165074953434668714 * (1 << SHIFT),
			 0.041565523271949324224383559567286283709 * (1 << SHIFT),
			 0.03266607144386817623837870883107825648  * (1 << SHIFT),
			-0.068224549707270459864005829331290442497 * (1 << SHIFT),
			-0.076102167644322413209145850032655289397 * (1 << SHIFT),
			 0.157303124195510402039133168727857992053 * (1 << SHIFT),
			 0.43955491415211112027350282005500048399  * (1 << SHIFT)
		};

		// init read-pointer
		int a = m_ptr;
		int b = (m_ptr + (FILTERLEN - 1)) % FILTERLEN;

		// go through samples in buffer
		qint32 iAcc = 0;
		qint32 qAcc = 0;
		for(int i = 0; i < (FILTERLEN / 2); i++) {
			// do multiply-accumulate
			qint32 iTmp = m_samples[a][0] + m_samples[b][0];
			qint32 qTmp = m_samples[a][1] + m_samples[b][1];
			iAcc += iTmp * COEFF[i];
			qAcc += qTmp * COEFF[i];

			// update read-pointer
			a = (a + 1) % FILTERLEN;
			b = (b + (FILTERLEN - 1)) % FILTERLEN;
		}

		// done, save result
		*iResult = iAcc >> SHIFT;
		*qResult = qAcc >> SHIFT;
	}
};

#endif // INCLUDE_INTHALFBANDFILTER_H
