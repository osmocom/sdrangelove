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
#include "dsp/dspengine.h"

class DSPEngine;
class Indicator;

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

	Settings m_settings;

	DSPEngine m_dspEngine;
	QTimer m_statusTimer;

	DSPEngine::State m_lastEngineState;

	Indicator* m_engineIdle;
	Indicator* m_engineRunning;
	Indicator* m_engineError;

	int m_sampleRate;

	void createStatusBar();
	void updateCenterFreqDisplay();
	void updateSampleRate();

private slots:
	void updateStatus();
	void on_action_Start_triggered();
	void on_action_Stop_triggered();
	void on_fftSize_currentIndexChanged(const QString& str);
	void on_fftWindow_currentIndexChanged(int index);
	void on_iqSwap_toggled(bool checked);
	void on_decimation_currentIndexChanged(int index);
	void on_e4000LNAGain_currentIndexChanged(int index);
	void on_e4000MixerGain_currentIndexChanged(int index);
	void on_e4000MixerEnh_currentIndexChanged(int index);
	void on_e4000if1_currentIndexChanged(int index);
	void on_e4000if2_currentIndexChanged(int index);
	void on_e4000if3_currentIndexChanged(int index);
	void on_e4000if4_currentIndexChanged(int index);
	void on_e4000if5_currentIndexChanged(int index);
	void on_e4000if6_currentIndexChanged(int index);
	void on_centerFrequency_changed(quint64 value);
	void on_action_Debug_triggered();
	void on_dcOffset_toggled(bool checked);
	void on_iqImbalance_toggled(bool checked);
	void on_dispLiveSpectrum_toggled(bool checked);
	void on_dispHistogram_toggled(bool checked);
	void on_dispWaterfall_toggled(bool checked);
	void on_waterfallInverted_toggled(bool checked);
};

#endif // INCLUDE_MAINWINDOW_H
