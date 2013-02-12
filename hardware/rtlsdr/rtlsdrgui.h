#ifndef INCLUDE_RTLSDRGUI_H
#define INCLUDE_RTLSDRGUI_H

#include <QTimer>
#include "../samplesourcegui.h"
#include "rtlsdrinput.h"

namespace Ui {
	class RTLSDRGui;
}

class RTLSDRGui : public SampleSourceGUI {
	Q_OBJECT

public:
	explicit RTLSDRGui(MessageQueue* msgQueue, QWidget* parent = NULL);
	~RTLSDRGui();

	QString serializeSettings() const;
	bool deserializeSettings(const QString& settings);

	void handleSourceMessage(DSPCmdSourceToGUI* cmd);

private:
	Ui::RTLSDRGui* ui;

	MessageQueue* m_msgQueue;
	RTLSDRInput::Settings m_settings;
	QTimer m_updateTimer;
	std::vector<int> m_gains;

	void displaySettings();
	void sendSettings();

private slots:
	void on_centerFrequency_changed(quint64 value);
	void on_gain_valueChanged(int value);
	void on_decimation_valueChanged(int value);

	void updateHardware();
};

#endif // INCLUDE_RTLSDRGUI_H
