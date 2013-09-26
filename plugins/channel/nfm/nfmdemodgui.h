#ifndef INCLUDE_NFMDEMODGUI_H
#define INCLUDE_NFMDEMODGUI_H

#include "plugin/plugingui.h"

class QDockWidget;

class PluginAPI;
class ChannelMarker;

class AudioFifo;
class ThreadedSampleSink;
class Channelizer;
class NFMDemod;
class SpectrumVis;

namespace Ui {
	class NFMDemodGUI;
}

class NFMDemodGUI : public PluginGUI {
	Q_OBJECT

public:
	static NFMDemodGUI* create(PluginAPI* pluginAPI);
	void destroy();

	void setWidgetName(const QString& name);

	void resetToDefaults();
	QByteArray serialize() const;
	bool deserialize(const QByteArray& data);

	bool handleMessage(Message* message);

private slots:
	void viewChanged();
	void on_rfBW_valueChanged(int value);
	void on_afBW_valueChanged(int value);
	void on_volume_valueChanged(int value);
	void on_squelch_valueChanged(int value);

private:
	Ui::NFMDemodGUI* ui;
	PluginAPI* m_pluginAPI;
	QDockWidget* m_dockWidget;
	ChannelMarker* m_channelMarker;

	AudioFifo* m_audioFifo;
	ThreadedSampleSink* m_threadedSampleSink;
	Channelizer* m_channelizer;
	NFMDemod* m_nfmDemod;
	SpectrumVis* m_spectrumVis;

	static const QString m_demodName;
	static const int m_rfBW[];

	explicit NFMDemodGUI(PluginAPI* pluginAPI, QDockWidget* dockWidget, QWidget* parent = NULL);
	~NFMDemodGUI();

	void applySettings();
};

#endif // INCLUDE_NFMDEMODGUI_H
