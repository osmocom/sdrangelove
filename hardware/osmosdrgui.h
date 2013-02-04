#ifndef INCLUDE_OSMOSDRGUI_H
#define INCLUDE_OSMOSDRGUI_H

#include <QTimer>
#include "samplesourcegui.h"
#include "osmosdrinput.h"

namespace Ui {
	class OsmoSDRGui;
}

class OsmoSDRGui : public SampleSourceGUI {
	Q_OBJECT

public:
	explicit OsmoSDRGui(MessageQueue* msgQueue, QWidget* parent = NULL);
	~OsmoSDRGui();

	QString serializeSettings() const;
	bool deserializeSettings(const QString& settings);

private:
	Ui::OsmoSDRGui* ui;

	MessageQueue* m_msgQueue;
	OsmoSDRInput::Settings m_settings;
	QTimer m_updateTimer;

	void displaySettings();
	void sendSettings();
	int e4kLNAGainToIdx(int gain) const;
	int e4kIdxToLNAGain(int idx) const;

private slots:
	void on_iqSwap_toggled(bool checked);
	void on_e4000MixerGain_currentIndexChanged(int index);
	void on_e4000MixerEnh_currentIndexChanged(int index);
	void on_e4000if1_currentIndexChanged(int index);
	void on_e4000if2_currentIndexChanged(int index);
	void on_e4000if3_currentIndexChanged(int index);
	void on_e4000if4_currentIndexChanged(int index);
	void on_e4000if5_currentIndexChanged(int index);
	void on_e4000if6_currentIndexChanged(int index);
	void on_centerFrequency_changed(quint64 value);
	void on_filterI1_valueChanged(int value);
	void on_filterI2_valueChanged(int value);
	void on_filterQ1_valueChanged(int value);
	void on_filterQ2_valueChanged(int value);
	void on_decimation_valueChanged(int value);
	void on_e4000LNAGain_valueChanged(int value);
	void on_e4kI_valueChanged(int value);
	void on_e4kQ_valueChanged(int value);

	void updateHardware();
};

#endif // INCLUDE_OSMOSDRGUI_H
