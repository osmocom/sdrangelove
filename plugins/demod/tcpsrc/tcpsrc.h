#ifndef INCLUDE_TCPSRC_H
#define INCLUDE_TCPSRC_H

#include "dsp/samplesink.h"
#include "dsp/nco.h"
#include "dsp/interpolator.h"
#include "util/message.h"

class TCPSrc : public SampleSink {
public:
	TCPSrc(SampleSink* spectrum);
	~TCPSrc();

	void configure(MessageQueue* messageQueue, int sampleFormat, Real outputSampleRate, Real rfBandwidth, int tcpPort);

	void feed(SampleVector::const_iterator begin, SampleVector::const_iterator end, bool firstOfBurst);
	void start();
	void stop();
	bool handleMessage(Message* cmd);

protected:
	class MsgConfigureTCPSrc : public Message {
	public:
		static MessageRegistrator ID;

		int getSampleFormat() const { return m_sampleFormat; }
		Real getOutputSampleRate() const { return m_outputSampleRate; }
		Real getRFBandwidth() const { return m_rfBandwidth; }
		int getTCPPort() const { return m_tcpPort; }

		static MsgConfigureTCPSrc* create(int sampleFormat, Real sampleRate, Real rfBandwidth, int tcpPort)
		{
			return new MsgConfigureTCPSrc(sampleFormat, sampleRate, rfBandwidth, tcpPort);
		}

	private:
		int m_sampleFormat;
		Real m_outputSampleRate;
		Real m_rfBandwidth;
		int m_tcpPort;

		MsgConfigureTCPSrc(int sampleFormat, Real outputSampleRate, Real rfBandwidth, int tcpPort) :
			Message(ID()),
			m_sampleFormat(sampleFormat),
			m_outputSampleRate(outputSampleRate),
			m_rfBandwidth(rfBandwidth),
			m_tcpPort(tcpPort)
		{ }
	};

	int m_inputSampleRate;

	int m_sampleFormat;
	Real m_outputSampleRate;
	Real m_rfBandwidth;
	int m_tcpPort;

	NCO m_nco;
	Interpolator m_interpolator;
	Real m_sampleDistanceRemain;

	SampleVector m_sampleBuffer;
	SampleSink* m_spectrum;
};

#endif // INCLUDE_TCPSRC_H
