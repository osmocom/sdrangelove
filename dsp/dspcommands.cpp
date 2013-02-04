#include "dspcommands.h"

int DSPCmdPing::type() const
{
	return Type;
}

const char* DSPCmdPing::name() const
{
	return "Ping";
}

int DSPCmdExit::type() const
{
	return Type;
}

const char* DSPCmdExit::name() const
{
	return "Exit";
}

int DSPCmdAcquisitionStart::type() const
{
	return Type;
}

const char* DSPCmdAcquisitionStart::name() const
{
	return "AcquisitionStart";
}

int DSPCmdAcquisitionStop::type() const
{
	return Type;
}

const char* DSPCmdAcquisitionStop::name() const
{
	return "AcquisitionStop";
}

int DSPCmdGetDeviceDescription::type() const
{
	return Type;
}

const char* DSPCmdGetDeviceDescription::name() const
{
	return "GetDeviceDescription";
}

void DSPCmdGetDeviceDescription::setDeviceDescription(const QString& text)
{
	m_deviceDescription = text;
	m_deviceDescription.detach();
}

int DSPCmdGetErrorMessage::type() const
{
	return Type;
}

const char* DSPCmdGetErrorMessage::name() const
{
	return "GetErrorMessage";
}

void DSPCmdGetErrorMessage::setErrorMessage(const QString& text)
{
	m_errorMessage = text;
	m_errorMessage.detach();
}

int DSPCmdSetSource::type() const
{
	return Type;
}

const char* DSPCmdSetSource::name() const
{
	return "SetSource";
}

int DSPCmdAddSink::type() const
{
	return Type;
}

const char* DSPCmdAddSink::name() const
{
	return "AddSink";
}

int DSPCmdRemoveSink::type() const
{
	return Type;
}

const char* DSPCmdRemoveSink::name() const
{
	return "RemoveSink";
}

int DSPCmdConfigureSpectrumVis::type() const
{
	return Type;
}

const char* DSPCmdConfigureSpectrumVis::name() const
{
	return "ConfigureSpectrumVis";
}

int DSPCmdConfigureCorrection::type() const
{
	return Type;
}

const char* DSPCmdConfigureCorrection::name() const
{
	return "ConfigureCorrection";
}

int DSPRepEngineReport::type() const
{
	return Type;
}

const char* DSPRepEngineReport::name() const
{
	return "EngineReport";
}
