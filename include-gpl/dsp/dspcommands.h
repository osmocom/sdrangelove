#ifndef INCLUDE_DSPCOMMANDS_H
#define INCLUDE_DSPCOMMANDS_H

#include <QString>
#include "util/message.h"
#include "fftwindow.h"
#include "util/export.h"

class SampleSource;
class SampleSink;
class AudioFifo;

class SDRANGELOVE_API DSPPing : public Message {
public:
	static MessageRegistrator ID;

	DSPPing() : Message(ID()) { }
};

class SDRANGELOVE_API DSPExit : public Message {
public:
	static MessageRegistrator ID;

	DSPExit() : Message(ID()) { }
};

class SDRANGELOVE_API DSPAcquisitionStart : public Message {
public:
	static MessageRegistrator ID;

	DSPAcquisitionStart() : Message(ID()) { }
};

class SDRANGELOVE_API DSPAcquisitionStop : public Message {
public:
	static MessageRegistrator ID;

	DSPAcquisitionStop() : Message(ID()) { }
};

class SDRANGELOVE_API DSPGetDeviceDescription : public Message {
public:
	static MessageRegistrator ID;

	DSPGetDeviceDescription() : Message(ID()) { }

	void setDeviceDescription(const QString& text) { m_deviceDescription = text; }
	const QString& getDeviceDescription() const { return m_deviceDescription; }

private:
	QString m_deviceDescription;
};

class SDRANGELOVE_API DSPGetErrorMessage : public Message {
public:
	static MessageRegistrator ID;

	DSPGetErrorMessage() : Message(ID()) { }

	void setErrorMessage(const QString& text) { m_errorMessage = text; }
	const QString& getErrorMessage() const { return m_errorMessage; }

private:
	QString m_errorMessage;
};

class SDRANGELOVE_API DSPSetSource : public Message {
public:
	static MessageRegistrator ID;

	DSPSetSource(SampleSource* sampleSource) : Message(ID()), m_sampleSource(sampleSource) { }

	SampleSource* getSampleSource() const { return m_sampleSource; }

private:
	SampleSource* m_sampleSource;
};

class SDRANGELOVE_API DSPAddSink : public Message {
public:
	static MessageRegistrator ID;

	DSPAddSink(SampleSink* sampleSink) : Message(ID()), m_sampleSink(sampleSink) { }

	SampleSink* getSampleSink() const { return m_sampleSink; }

private:
	SampleSink* m_sampleSink;
};

class SDRANGELOVE_API DSPRemoveSink : public Message {
public:
	static MessageRegistrator ID;

	DSPRemoveSink(SampleSink* sampleSink) : Message(ID()), m_sampleSink(sampleSink) { }

	SampleSink* getSampleSink() const { return m_sampleSink; }

private:
	SampleSink* m_sampleSink;
};

class SDRANGELOVE_API DSPAddAudioSource : public Message {
public:
	static MessageRegistrator ID;

	DSPAddAudioSource(AudioFifo* audioFifo) : Message(ID()), m_audioFifo(audioFifo) { }

	AudioFifo* getAudioFifo() const { return m_audioFifo; }

private:
	AudioFifo* m_audioFifo;
};

class SDRANGELOVE_API DSPRemoveAudioSource : public Message {
public:
	static MessageRegistrator ID;

	DSPRemoveAudioSource(AudioFifo* audioFifo) : Message(ID()), m_audioFifo(audioFifo) { }

	AudioFifo* getAudioFifo() const { return m_audioFifo; }

private:
	AudioFifo* m_audioFifo;
};

class SDRANGELOVE_API DSPConfigureSpectrumVis : public Message {
public:
	static MessageRegistrator ID;

	int getFFTSize() const { return m_fftSize; }
	int getOverlapPercent() const { return m_overlapPercent; }
	FFTWindow::Function getWindow() const { return m_window; }

	static DSPConfigureSpectrumVis* create(int fftSize, int overlapPercent, FFTWindow::Function window)
	{
		return new DSPConfigureSpectrumVis(fftSize, overlapPercent, window);
	}

private:
	int m_fftSize;
	int m_overlapPercent;
	FFTWindow::Function m_window;

	DSPConfigureSpectrumVis(int fftSize, int overlapPercent, FFTWindow::Function window) :
		Message(ID()),
		m_fftSize(fftSize),
		m_overlapPercent(overlapPercent),
		m_window(window)
	{ }
};

class SDRANGELOVE_API DSPConfigureCorrection : public Message {
public:
	static MessageRegistrator ID;

	bool getDCOffsetCorrection() const { return m_dcOffsetCorrection; }
	bool getIQImbalanceCorrection() const { return m_iqImbalanceCorrection; }

	static DSPConfigureCorrection* create(bool dcOffsetCorrection, bool iqImbalanceCorrection)
	{
		return new DSPConfigureCorrection(dcOffsetCorrection, iqImbalanceCorrection);
	}

private:
	bool m_dcOffsetCorrection;
	bool m_iqImbalanceCorrection;

	DSPConfigureCorrection(bool dcOffsetCorrection, bool iqImbalanceCorrection) :
		Message(ID()),
		m_dcOffsetCorrection(dcOffsetCorrection),
		m_iqImbalanceCorrection(iqImbalanceCorrection)
	{ }
};

class SDRANGELOVE_API DSPEngineReport : public Message {
public:
	static MessageRegistrator ID;

	int getSampleRate() const { return m_sampleRate; }
	quint64 getCenterFrequency() const { return m_centerFrequency; }

	static DSPEngineReport* create(int sampleRate, quint64 centerFrequency)
	{
		return new DSPEngineReport(sampleRate, centerFrequency);
	}

private:
	int m_sampleRate;
	quint64 m_centerFrequency;

	DSPEngineReport(int sampleRate, quint64 centerFrequency) :
		Message(ID()),
		m_sampleRate(sampleRate),
		m_centerFrequency(centerFrequency)
	{ }
};

class SDRANGELOVE_API DSPConfigureScopeVis : public Message {
public:
	static MessageRegistrator ID;

	int getTriggerChannel() const { return m_triggerChannel; }
	Real getTriggerLevelHigh() const { return m_triggerLevelHigh; }
	Real getTriggerLevelLow() const { return m_triggerLevelLow; }

	static DSPConfigureScopeVis* create(int triggerChannel, Real triggerLevelHigh, Real triggerLevelLow)
	{
		return new DSPConfigureScopeVis(triggerChannel, triggerLevelHigh, triggerLevelLow);
	}

private:
	int m_triggerChannel;
	Real m_triggerLevelHigh;
	Real m_triggerLevelLow;

	DSPConfigureScopeVis(int triggerChannel, Real triggerLevelHigh, Real triggerLevelLow) :
		Message(ID()),
		m_triggerChannel(triggerChannel),
		m_triggerLevelHigh(triggerLevelHigh),
		m_triggerLevelLow(triggerLevelLow)
	{ }
};

class SDRANGELOVE_API DSPSignalNotification : public Message {
public:
	static MessageRegistrator ID;

	int getSampleRate() const { return m_sampleRate; }
	qint64 getFrequencyOffset() const { return m_frequencyOffset; }

	static DSPSignalNotification* create(int sampleRate, quint64 frequencyOffset)
	{
		return new DSPSignalNotification(sampleRate, frequencyOffset);
	}

private:
	int m_sampleRate;
	qint64 m_frequencyOffset;

	DSPSignalNotification(int samplerate, qint64 frequencyOffset) :
		Message(ID()),
		m_sampleRate(samplerate),
		m_frequencyOffset(frequencyOffset)
	{ }
};

class SDRANGELOVE_API DSPConfigureChannelizer : public Message {
public:
	static MessageRegistrator ID;

	int getSampleRate() const { return m_sampleRate; }
	int getCenterFrequency() const { return m_centerFrequency; }

	static DSPConfigureChannelizer* create(int sampleRate, int centerFrequency)
	{
		return new DSPConfigureChannelizer(sampleRate, centerFrequency);
	}

private:
	int m_sampleRate;
	int m_centerFrequency;

	DSPConfigureChannelizer(int sampleRate, int centerFrequency) :
		Message(ID()),
		m_sampleRate(sampleRate),
		m_centerFrequency(centerFrequency)
	{ }
};

#endif // INCLUDE_DSPCOMMANDS_H
