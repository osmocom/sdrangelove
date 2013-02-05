#ifndef INCLUDE_DSPCOMMANDS_H
#define INCLUDE_DSPCOMMANDS_H

#include <QString>
#include "../util/message.h"
#include "fftwindow.h"

class SampleSource;
class SampleSink;

class DSPCmdPing : public Message {
public:
	enum {
		Type = 1
	};

	DSPCmdPing() { }
	int type() const;
	const char* name() const;
};

class DSPCmdExit : public Message {
public:
	enum {
		Type = 2
	};

	DSPCmdExit() { }
	int type() const;
	const char* name() const;
};

class DSPCmdAcquisitionStart : public Message {
public:
	enum {
		Type = 3
	};
	DSPCmdAcquisitionStart() { }
	int type() const;
	const char* name() const;
};

class DSPCmdAcquisitionStop : public Message {
public:
	enum {
		Type = 4
	};
	DSPCmdAcquisitionStop() { }
	int type() const;
	const char* name() const;
};

class DSPCmdGetDeviceDescription : public Message {
public:
	enum {
		Type = 5
	};
	DSPCmdGetDeviceDescription() { }
	int type() const;
	const char* name() const;

	void setDeviceDescription(const QString& text);
	const QString& getDeviceDescription() const { return m_deviceDescription; }

private:
	QString m_deviceDescription;
};

class DSPCmdGetErrorMessage : public Message {
public:
	enum {
		Type = 6
	};
	DSPCmdGetErrorMessage() { }
	int type() const;
	const char* name() const;

	void setErrorMessage(const QString& text);
	const QString& getErrorMessage() const { return m_errorMessage; }

private:
	QString m_errorMessage;
};

class DSPCmdSetSource : public Message {
public:
	enum {
		Type = 7
	};
	DSPCmdSetSource(SampleSource* source) : m_source(source) { }
	int type() const;
	const char* name() const;

	SampleSource* getSource() const { return m_source; }

private:
	SampleSource* m_source;
};

class DSPCmdAddSink : public Message {
public:
	enum {
		Type = 8
	};
	DSPCmdAddSink(SampleSink* sink) : m_sink(sink) { }
	int type() const;
	const char* name() const;

	SampleSink* getSink() const { return m_sink; }

private:
	SampleSink* m_sink;
};

class DSPCmdRemoveSink : public Message {
public:
	enum {
		Type = 9
	};
	DSPCmdRemoveSink(SampleSink* sink) : m_sink(sink) { }
	int type() const;
	const char* name() const;

	SampleSink* getSink() const { return m_sink; }

private:
	SampleSink* m_sink;
};

class DSPCmdConfigureSpectrumVis : public Message {
public:
	enum {
		Type = 10
	};
	int type() const;
	const char* name() const;

	int getFFTSize() const { return m_fftSize; }
	int getOverlapPercent() const { return m_overlapPercent; }
	FFTWindow::Function getWindow() const { return m_window; }

	static DSPCmdConfigureSpectrumVis* create(int fftSize, int overlapPercent, FFTWindow::Function window)
	{
		return new DSPCmdConfigureSpectrumVis(fftSize, overlapPercent, window);
	}

private:
	int m_fftSize;
	int m_overlapPercent;
	FFTWindow::Function m_window;

	DSPCmdConfigureSpectrumVis(int fftSize, int overlapPercent, FFTWindow::Function window) :
		Message(),
		m_fftSize(fftSize),
		m_overlapPercent(overlapPercent),
		m_window(window)
	{ }
};

class DSPCmdConfigureCorrection : public Message {
public:
	enum {
		Type = 11
	};
	int type() const;
	const char* name() const;

	bool getDCOffsetCorrection() const { return m_dcOffsetCorrection; }
	bool getIQImbalanceCorrection() const { return m_iqImbalanceCorrection; }

	static DSPCmdConfigureCorrection* create(bool dcOffsetCorrection, bool iqImbalanceCorrection)
	{
		return new DSPCmdConfigureCorrection(dcOffsetCorrection, iqImbalanceCorrection);
	}

private:
	bool m_dcOffsetCorrection;
	bool m_iqImbalanceCorrection;

	DSPCmdConfigureCorrection(bool dcOffsetCorrection, bool iqImbalanceCorrection) :
		m_dcOffsetCorrection(dcOffsetCorrection),
		m_iqImbalanceCorrection(iqImbalanceCorrection)
	{ }
};

class DSPRepEngineReport : public Message {
public:
	enum {
		Type = 12
	};
	int type() const;
	const char* name() const;

	int getSampleRate() const { return m_sampleRate; }
	quint64 getCenterFrequency() const { return m_centerFrequency; }

	static DSPRepEngineReport* create(int sampleRate, quint64 centerFrequency) { return new DSPRepEngineReport(sampleRate, centerFrequency); }

private:
	int m_sampleRate;
	quint64 m_centerFrequency;

	DSPRepEngineReport(int sampleRate, quint64 centerFrequency) :
		m_sampleRate(sampleRate),
		m_centerFrequency(centerFrequency)
	{ }
};

// Type == 13 -> DSPCmdConfigureSource (defined in samplesource.h)

class DSPCmdConfigureScopeVis : public Message {
public:
	enum {
		Type = 14
	};
	int type() const;
	const char* name() const;

	int getTriggerChannel() const { return m_triggerChannel; }
	Real getTriggerLevelHigh() const { return m_triggerLevelHigh; }
	Real getTriggerLevelLow() const { return m_triggerLevelLow; }

	static DSPCmdConfigureScopeVis* create(int triggerChannel, Real triggerLevelHigh, Real triggerLevelLow)
	{
		return new DSPCmdConfigureScopeVis(triggerChannel, triggerLevelHigh, triggerLevelLow);
	}

private:
	int m_triggerChannel;
	Real m_triggerLevelHigh;
	Real m_triggerLevelLow;

	DSPCmdConfigureScopeVis(int triggerChannel, Real triggerLevelHigh, Real triggerLevelLow) :
		Message(),
		m_triggerChannel(triggerChannel),
		m_triggerLevelHigh(triggerLevelHigh),
		m_triggerLevelLow(triggerLevelLow)
	{ }
};

#endif // INCLUDE_DSPCOMMANDS_H
