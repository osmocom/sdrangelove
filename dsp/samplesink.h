#ifndef INCLUDE_SAMPLESINK_H
#define INCLUDE_SAMPLESINK_H

#include "dsptypes.h"

class SampleSink {
public:
	SampleSink();

	virtual size_t workUnitSize() = 0;

	void feed(SampleVector::const_iterator begin, SampleVector::const_iterator end);
	virtual size_t work(SampleVector::const_iterator begin, SampleVector::const_iterator end) = 0;

protected:
	SampleVector m_buffer;
	size_t m_bufferFill;
};

#endif // INCLUDE_SAMPLESINK_H
