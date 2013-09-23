#include "tcpsrc.h"
#include "dsp/dspcommands.h"

MessageRegistrator TCPSrc::MsgConfigureTCPSrc::ID("MsgConfigureTCPSrc");

TCPSrc::TCPSrc(SampleSink* spectrum)
{
	m_inputSampleRate = 100000;
	m_sampleFormat = 0;
	m_outputSampleRate = 50000;
	m_rfBandwidth = 50000;
	m_tcpPort = 9999;
	m_nco.setFreq(0, m_inputSampleRate);
	m_interpolator.create(16, m_inputSampleRate, m_rfBandwidth / 2.1);
	m_sampleDistanceRemain = m_inputSampleRate / m_outputSampleRate;
	m_spectrum = spectrum;
}

TCPSrc::~TCPSrc()
{
}

void TCPSrc::configure(MessageQueue* messageQueue, int sampleFormat, Real outputSampleRate, Real rfBandwidth, int tcpPort)
{
	Message* cmd = MsgConfigureTCPSrc::create(sampleFormat, outputSampleRate, rfBandwidth, tcpPort);
	cmd->submit(messageQueue, this);
}

void TCPSrc::feed(SampleVector::const_iterator begin, SampleVector::const_iterator end, bool firstOfBurst)
{
	Complex ci;
	bool consumed;

	for(SampleVector::const_iterator it = begin; it < end; ++it) {
		Complex c(it->real() / 32768.0, it->imag() / 32768.0);
		c *= m_nco.nextIQ();

		consumed = false;
		if(m_interpolator.interpolate(&m_sampleDistanceRemain, c, &consumed, &ci)) {
			m_sampleBuffer.push_back(Sample(ci.real() * 32768.0, ci.imag() * 32768.0));
			m_sampleDistanceRemain += m_inputSampleRate / m_outputSampleRate;
		}
	}

	if(m_spectrum != NULL)
		m_spectrum->feed(m_sampleBuffer.begin(), m_sampleBuffer.end(), firstOfBurst);
	m_sampleBuffer.clear();
}

void TCPSrc::start()
{
}

void TCPSrc::stop()
{
}

bool TCPSrc::handleMessage(Message* cmd)
{
	if(cmd->id() == DSPSignalNotification::ID()) {
		DSPSignalNotification* signal = (DSPSignalNotification*)cmd;
		qDebug("%d samples/sec, %lld Hz offset", signal->getSampleRate(), signal->getFrequencyOffset());
		m_inputSampleRate = signal->getSampleRate();
		m_nco.setFreq(-signal->getFrequencyOffset(), m_inputSampleRate);
		m_interpolator.create(16, m_inputSampleRate, m_rfBandwidth / 2.1);
		m_sampleDistanceRemain = m_inputSampleRate / m_outputSampleRate;
		cmd->completed();
		return true;
	} else if(cmd->id() == MsgConfigureTCPSrc::ID()) {
		MsgConfigureTCPSrc* cfg = (MsgConfigureTCPSrc*)cmd;
		m_sampleFormat = cfg->getSampleFormat();
		m_outputSampleRate = cfg->getOutputSampleRate();
		m_rfBandwidth = cfg->getRFBandwidth();
		m_tcpPort = cfg->getTCPPort();
		m_interpolator.create(16, m_inputSampleRate, m_rfBandwidth / 2.1);
		m_sampleDistanceRemain = m_inputSampleRate / m_outputSampleRate;
		cmd->completed();
		return true;
	} else {
		return false;
	}
}
