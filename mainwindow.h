///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_MAINWINDOW_H
#define INCLUDE_MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "settings.h"
#include "preset.h"
#include "dsp/dspengine.h"
#include "util/messagequeue.h"

class QLabel;
class DSPEngine;
class Indicator;
class ScopeWindow;
class SpectrumVis;
class SampleSourceGUI;

class QTreeWidgetItem;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = NULL);
	~MainWindow();

private:
	Ui::MainWindow *ui;

	MessageQueue m_messageQueue;

	QSettings m_settingsStorage;
	PresetList m_presetList;
	Settings m_settings;

	SpectrumVis* m_spectrumVis;
	SampleSource* m_sampleSource;
	SampleSourceGUI* m_sampleSourceGUI;

	DSPEngine m_dspEngine;

	QTimer m_statusTimer;
	DSPEngine::State m_lastEngineState;

	QLabel* m_sampleRateWidget;
	Indicator* m_engineIdle;
	Indicator* m_engineRunning;
	Indicator* m_engineError;

	bool m_startOSDRUpdateAfterStop;

	ScopeWindow* m_scopeWindow;

	int m_sampleRate;
	quint64 m_centerFrequency;

	void loadSettings();
	void saveSettings();

	void createStatusBar();
	void closeEvent(QCloseEvent*);
	void updateCenterFreqDisplay();
	void updateSampleRate();
	void updatePresets();
	void applySettings();

private slots:
	void handleMessages();
	void updateStatus();
	void scopeWindowDestroyed();
	void on_action_Start_triggered();
	void on_action_Stop_triggered();
	void on_fftWindow_currentIndexChanged(int index);
	void on_action_Debug_triggered();
	void on_dcOffset_toggled(bool checked);
	void on_iqImbalance_toggled(bool checked);
	void on_action_View_Waterfall_toggled(bool checked);
	void on_action_View_Histogram_toggled(bool checked);
	void on_action_View_LiveSpectrum_toggled(bool checked);
	void on_action_View_Fullscreen_toggled(bool checked);
	void on_actionOsmoSDR_Firmware_Upgrade_triggered();
	void on_fftSize_valueChanged(int value);
	void on_waterfall_toggled(bool checked);
	void on_histogram_toggled(bool checked);
	void on_liveSpectrum_toggled(bool checked);
	void on_refLevel_valueChanged(int value);
	void on_levelRange_valueChanged(int value);
	void on_presetSave_clicked();
	void on_presetLoad_clicked();
	void on_presetDelete_clicked();
	void on_presetTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void on_presetTree_itemActivated(QTreeWidgetItem *item, int column);
	void on_action_Oscilloscope_triggered();
};

#endif // INCLUDE_MAINWINDOW_H
