#include <QMessageBox>
//#include <portaudio.h>
#include "audio/portaudioholder.h"

PortAudioHolder::PortAudioHolder() :
	m_initialized(false)
{
	//PaError err;

	//if((err = Pa_Initialize()) == paNoError) {
		m_initialized = true;
		qDebug("PortAudio initialized");
	//} else {
	//	qCritical("PortAudio: could not initialise: %s (%d)", Pa_GetErrorText(err), err);
	//	QString error = QObject::tr("PortAudio could not be initialised: %1 (%2)").arg(Pa_GetErrorText(err)).arg(err);
	//	QMessageBox::critical(NULL, "PortAudio failure", error);
	//}
}

PortAudioHolder::~PortAudioHolder()
{
	if(m_initialized) {
		//Pa_Terminate();
		qDebug("PortAudio terminated");
	}
}
