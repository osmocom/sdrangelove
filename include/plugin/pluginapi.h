#ifndef INCLUDE_PLUGINAPI_H
#define INCLUDE_PLUGINAPI_H

#include <QObject>

class QDockWidget;
class QAction;

class PluginManager;
class PluginInterface;
class SampleSource;
class SampleSink;
class DSPEngine;
class AudioFifo;
class MessageQueue;
class MainWindow;
class ChannelMarker;
class PluginGUI;

class PluginAPI : public QObject {
	Q_OBJECT

public:
	// MainWindow access
	QDockWidget* createMainWindowDock(Qt::DockWidgetArea dockWidgetArea, const QString& title);
	MessageQueue* getMainWindowMessageQueue();

	// Demodulator stuff
	void registerDemodulator(const QString& demodName, PluginInterface* plugin, QAction* action);
	void registerDemodulatorInstance(const QString& demodName, PluginGUI* pluginGUI);

	void addChannelMarker(ChannelMarker* channelMarker);
	void removeChannelMarker(ChannelMarker* channelMarker);

	// DSPEngine access
	void setSampleSource(SampleSource* sampleSource);
	void addSampleSink(SampleSink* sampleSink);
	void removeSampleSink(SampleSink* sampleSink);
	MessageQueue* getDSPEngineMessageQueue();
	void addAudioSource(AudioFifo* audioFifo);
	void removeAudioSource(AudioFifo* audioFifo);

	// Sample Source stuff
	void registerSampleSource(const QString& sourceName, PluginInterface* plugin);

protected:
	PluginManager* m_pluginManager;
	MainWindow* m_mainWindow;
	DSPEngine* m_dspEngine;

	PluginAPI(PluginManager* pluginManager, MainWindow* mainWindow, DSPEngine* dspEngine);

	friend class PluginManager;
};

#endif // INCLUDE_PLUGINAPI_H
