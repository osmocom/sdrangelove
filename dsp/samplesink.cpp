#include "samplesink.h"

SampleSink::SampleSink() :
	m_buffer(),
	m_bufferFill(0)
{
}

void SampleSink::feed(SampleVector::const_iterator begin, SampleVector::const_iterator end)
{
	size_t wus = workUnitSize();

	// make sure our buffer is big enough for at least one work unit
	if(m_buffer.size() < wus)
		m_buffer.resize(wus);

	while(begin < end) {
		// if the buffer contains something, keep filling it until it contains one complete work unit
		if((m_bufferFill > 0) && (m_bufferFill < wus)) {
			// check number if missing samples, but don't copy more than we have
			size_t len = wus - m_bufferFill;
			if(len > (size_t)(end - begin))
				len = end - begin;
			// copy
			std::copy(begin, begin + len, m_buffer.begin() + m_bufferFill);
			// adjust pointers
			m_bufferFill += len;
			begin += len;
		}

		// if one complete work unit is in the buffer, feed it to the worker
		if(m_bufferFill >= wus) {
			size_t done = 0;
			while(m_bufferFill - done >= wus)
				done += work(m_buffer.begin() + done, m_buffer.begin() + done + wus);
			// now copy the remaining data to the front of the buffer (should be zero)
			if(m_bufferFill - done > 0) {
				qDebug("error in SampleSink buffer management");
				std::copy(m_buffer.begin() + done, m_buffer.begin() + m_bufferFill, m_buffer.begin());
			}
			m_bufferFill -= done;
		}

		// if no remainder from last run is buffered and we have at least one work unit left, feed the data to the worker
		if(m_bufferFill == 0) {
			while((size_t)(end - begin) > wus)
				begin += work(begin, begin + wus);
			// copy any remaining data to the buffer
			std::copy(begin, end, m_buffer.begin());
			m_bufferFill = end - begin;
			begin += m_bufferFill;
		}
	}
}
