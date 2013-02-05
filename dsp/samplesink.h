#ifndef INCLUDE_SAMPLESINK_H
#define INCLUDE_SAMPLESINK_H

#include "dsptypes.h"

class Message;

class SampleSink {
public:
	SampleSink();
	virtual ~SampleSink();

	virtual void feed(SampleVector::const_iterator begin, SampleVector::const_iterator end, bool firstOfBurst) = 0;
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void setSampleRate(int sampleRate) = 0;
	virtual void handleMessage(Message* cmd) = 0;
};

#endif // INCLUDE_SAMPLESINK_H

#if 0
#ifndef INCLUDE_SAMPLESINK_H
#define INCLUDE_SAMPLESINK_H

#include "dsptypes.h"

class SampleSink {
public:
	SampleSink();

	virtual size_t workUnitSize() = 0;

	void feed(SampleVector::const_iterator begin, SampleVector::const_iterator end);
	virtual size_t work(SampleVector::const_iterator begin, SampleVector::const_iterator end) = 0;

private:
	SampleVector m_sinkBuffer;
	size_t m_sinkBufferFill;
};

#endif // INCLUDE_SAMPLESINK_H
#endif
