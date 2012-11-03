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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gui/indicator.h"
#include "dsp/channelizer.h"
#include "osdrupgrade.h"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_settings(),
	m_dspEngine(&m_settings),
	m_startOSDRUpdateAfterStop(false)
{
	m_settings.load();

	ui->setupUi(this);
	createStatusBar();

	m_dspEngine.setGLSpectrum(ui->glSpectrum);
	m_dspEngine.start();
	m_lastEngineState = (DSPEngine::State)-1;

	connect(&m_statusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
	m_statusTimer.start(500);

	ui->centerFrequency->setValueRange(7, 20000U, 2200000U);

	for(int i = 0; i < 6; i++) {
		if(m_settings.fftSize() == (1 << (i + 7))) {
			ui->fftSize->setValue(i);
			break;
		}
	}
	ui->fftWindow->setCurrentIndex(m_settings.fftWindow());

	ui->waterfall->setChecked(m_settings.displayWaterfall());
	ui->action_View_Waterfall->setChecked(m_settings.displayWaterfall());
	ui->glSpectrum->setDisplayWaterfall(m_settings.displayWaterfall());
	ui->glSpectrum->setInvertedWaterfall(m_settings.invertedWaterfall());
	ui->liveSpectrum->setChecked(m_settings.displayLiveSpectrum());
	ui->action_View_LiveSpectrum->setChecked(m_settings.displayLiveSpectrum());
	ui->glSpectrum->setDisplayLiveSpectrum(m_settings.displayLiveSpectrum());
	ui->action_View_Histogram->setChecked(m_settings.displayHistogram());
	ui->glSpectrum->setDisplayHistogram(m_settings.displayHistogram());
	ui->histogram->setChecked(m_settings.displayHistogram());

	ui->iqSwap->setChecked(m_settings.iqSwap());
	ui->decimation->setValue(m_settings.decimation());
	ui->dcOffset->setChecked(m_settings.dcOffsetCorrection());
	ui->iqImbalance->setChecked(m_settings.iqImbalanceCorrection());
	ui->e4000LNAGain->setValue(e4kLNAGainToIdx(m_settings.e4000LNAGain()));
	ui->e4000MixerGain->setCurrentIndex((m_settings.e4000MixerGain() - 40) / 80);
	if(m_settings.e4000MixerEnh() == 0)
		ui->e4000MixerEnh->setCurrentIndex(0);
	else ui->e4000MixerEnh->setCurrentIndex((m_settings.e4000MixerEnh() + 10) / 20);

	ui->e4000if1->setCurrentIndex((m_settings.e4000if1() + 30) / 90);
	ui->e4000if2->setCurrentIndex(m_settings.e4000if2() / 30);
	ui->e4000if3->setCurrentIndex(m_settings.e4000if3() / 30);
	ui->e4000if4->setCurrentIndex(m_settings.e4000if4() / 10);
	ui->e4000if5->setCurrentIndex(m_settings.e4000if5() / 30 - 1);
	ui->e4000if6->setCurrentIndex(m_settings.e4000if6() / 30 - 1);
	ui->filterI1->setValue(m_settings.filterI1());
	ui->filterI2->setValue(m_settings.filterI2());
	ui->filterQ1->setValue(m_settings.filterQ1());
	ui->filterQ2->setValue(m_settings.filterQ2());

	updateSampleRate();
	updateCenterFreqDisplay();
/*
	Channelizer* channelizer = new Channelizer;
	channelizer->setGLSpectrum(ui->chanSpectrum);
	m_dspEngine.addChannelizer(channelizer);
*/
}

MainWindow::~MainWindow()
{
	m_dspEngine.stop();

	m_settings.save();
	delete ui;
}

void MainWindow::createStatusBar()
{
	m_sampleRateWidget = new QLabel(tr("Rate: 0 kHz"), this);
	m_sampleRateWidget->setToolTip(tr("Sample Rate"));
	statusBar()->addPermanentWidget(m_sampleRateWidget);

	m_engineIdle = new Indicator(tr("Idle"), this);
	m_engineIdle->setToolTip(tr("DSP engine is idle"));
	statusBar()->addPermanentWidget(m_engineIdle);

	m_engineRunning = new Indicator(tr("Run"), this);
	m_engineRunning->setToolTip(tr("DSP engine is running"));
	statusBar()->addPermanentWidget(m_engineRunning);

	m_engineError = new Indicator(tr("Err"), this);
	m_engineError->setToolTip(tr("DSP engine failed"));
	statusBar()->addPermanentWidget(m_engineError);
}

void MainWindow::updateCenterFreqDisplay()
{
	qint64 freq = m_settings.centerFreq();
	ui->centerFrequency->setValue(freq / 1000);
	ui->glSpectrum->setCenterFrequency(freq);
}

void MainWindow::updateSampleRate()
{
	m_sampleRate = 4000000 / (1 << m_settings.decimation());
	ui->glSpectrum->setSampleRate(m_sampleRate);
	m_sampleRateWidget->setText(tr("Rate: %1 kHz").arg((float)m_sampleRate / 1000));
}

int MainWindow::e4kLNAGainToIdx(int gain) const
{
	static const quint32 gainList[13] = {
		-50, -25, 0, 25, 50, 75, 100, 125, 150, 175, 200, 250, 300
	};
	for(int i = 0; i < 13; i++) {
		if(gainList[i] == gain)
			return i;
	}
	return 0;
}

int MainWindow::e4kIdxToLNAGain(int idx) const
{
	static const quint32 gainList[13] = {
		-50, -25, 0, 25, 50, 75, 100, 125, 150, 175, 200, 250, 300
	};
	if((idx < 0) || (idx >= 13))
		return -50;
	else return gainList[idx];
}

void MainWindow::updateStatus()
{
	DSPEngine::State state = m_dspEngine.state();
	if(m_lastEngineState != state) {
		switch(state) {
			case DSPEngine::StNotStarted:
				m_engineIdle->setColor(Qt::gray);
				m_engineRunning->setColor(Qt::gray);
				m_engineError->setColor(Qt::gray);
				statusBar()->clearMessage();
				break;

			case DSPEngine::StIdle:
				m_engineIdle->setColor(Qt::cyan);
				m_engineRunning->setColor(Qt::gray);
				m_engineError->setColor(Qt::gray);
				statusBar()->clearMessage();
				if(m_startOSDRUpdateAfterStop)
					on_actionOsmoSDR_Firmware_Upgrade_triggered();
				break;

			case DSPEngine::StRunning:
				m_engineIdle->setColor(Qt::gray);
				m_engineRunning->setColor(Qt::green);
				m_engineError->setColor(Qt::gray);
				statusBar()->showMessage(tr("Sampling from %1").arg(m_dspEngine.deviceDesc()));
				break;

			case DSPEngine::StError:
				m_engineIdle->setColor(Qt::gray);
				m_engineRunning->setColor(Qt::gray);
				m_engineError->setColor(Qt::red);
				statusBar()->showMessage(tr("Error: %1").arg(m_dspEngine.errorMsg()));
				if(m_startOSDRUpdateAfterStop)
					on_actionOsmoSDR_Firmware_Upgrade_triggered();
				break;
		}
		m_lastEngineState = state;
	}
}

void MainWindow::viewToolBoxClosed()
{
	ui->action_View_Toolbox->setChecked(false);
}

void MainWindow::viewToolBoxWaterfallUpward(bool checked)
{
	ui->glSpectrum->setInvertedWaterfall(checked);
	m_settings.setInvertedWaterfall(checked);
}

void MainWindow::on_action_Start_triggered()
{
	m_dspEngine.startAcquisition();
}

void MainWindow::on_action_Stop_triggered()
{
	m_dspEngine.stopAcquistion();
}

void MainWindow::on_fftWindow_currentIndexChanged(int index)
{
	m_settings.setFFTWindow(index);
}

void MainWindow::on_iqSwap_toggled(bool checked)
{
	m_settings.setIQSwap(checked);
}

void MainWindow::on_e4000MixerGain_currentIndexChanged(int index)
{
	m_settings.setE4000MixerGain(index * 80 + 40);
}

void MainWindow::on_e4000MixerEnh_currentIndexChanged(int index)
{
	if(index == 0)
		m_settings.setE4000MixerEnh(0);
	else m_settings.setE4000MixerEnh(index * 20 - 10);
}

void MainWindow::on_e4000if1_currentIndexChanged(int index)
{
	m_settings.setE4000if1(index * 90 - 30);
}

void MainWindow::on_e4000if2_currentIndexChanged(int index)
{
	m_settings.setE4000if2(index * 30);
}

void MainWindow::on_e4000if3_currentIndexChanged(int index)
{
	m_settings.setE4000if3(index * 30);
}

void MainWindow::on_e4000if4_currentIndexChanged(int index)
{
	m_settings.setE4000if4(index * 10);
}

void MainWindow::on_e4000if5_currentIndexChanged(int index)
{
	m_settings.setE4000if5((index + 1) * 30);
}

void MainWindow::on_e4000if6_currentIndexChanged(int index)
{
	m_settings.setE4000if6((index + 1) * 30);
}

void MainWindow::on_centerFrequency_changed(quint64 value)
{
	m_settings.setCenterFreq(value * 1000);
	updateCenterFreqDisplay();
}

void MainWindow::on_action_Debug_triggered()
{
	m_dspEngine.triggerDebug();
}

void MainWindow::on_dcOffset_toggled(bool checked)
{
	m_settings.setDCOffsetCorrection(checked);
}

void MainWindow::on_iqImbalance_toggled(bool checked)
{
	m_settings.setIQImbalanceCorrection(checked);
}

void MainWindow::on_filterI1_valueChanged(int value)
{
	m_settings.setFilterI1(value);
}

void MainWindow::on_filterI2_valueChanged(int value)
{
	m_settings.setFilterI2(value);
}

void MainWindow::on_filterQ1_valueChanged(int value)
{
	m_settings.setFilterQ1(value);
}

void MainWindow::on_filterQ2_valueChanged(int value)
{
	m_settings.setFilterQ2(value);
}

void MainWindow::on_action_View_Waterfall_toggled(bool checked)
{
	ui->action_View_Waterfall->setChecked(checked);
	ui->glSpectrum->setDisplayWaterfall(checked);
	m_settings.setDisplayWaterfall(checked);
}

void MainWindow::on_action_View_Histogram_toggled(bool checked)
{
	ui->action_View_Histogram->setChecked(checked);
	ui->glSpectrum->setDisplayHistogram(checked);
	m_settings.setDisplayHistogram(checked);
}

void MainWindow::on_action_View_LiveSpectrum_toggled(bool checked)
{
	ui->action_View_LiveSpectrum->setChecked(checked);
	ui->glSpectrum->setDisplayLiveSpectrum(checked);
	m_settings.setDisplayLiveSpectrum(checked);
}

void MainWindow::on_action_View_Fullscreen_toggled(bool checked)
{
	if(checked)
		showFullScreen();
	else showNormal();
}

void MainWindow::on_actionOsmoSDR_Firmware_Upgrade_triggered()
{
	DSPEngine::State engineState = m_dspEngine.state();
	if((engineState != DSPEngine::StIdle) && (engineState != DSPEngine::StError)) {
		m_startOSDRUpdateAfterStop = true;
		m_dspEngine.stopAcquistion();
		return;
	}
	m_startOSDRUpdateAfterStop = false;

	OSDRUpgrade osdrUpgrade;
	osdrUpgrade.exec();
}

void MainWindow::on_decimation_valueChanged(int value)
{
	ui->decimationDisplay->setText(tr("1:%1").arg(1 << value));
	m_settings.setDecimation(value);
	updateSampleRate();
}

void MainWindow::on_fftSize_valueChanged(int value)
{
	ui->fftSizeDisplay->setText(tr("%1").arg(1 << (7 + value)));
	m_settings.setFFTSize(1 << (7 + value));
}

void MainWindow::on_e4000LNAGain_valueChanged(int value)
{
	int gain = e4kIdxToLNAGain(value);
	ui->e4000LNAGainDisplay->setText(tr("%1.%2").arg(gain / 10).arg(abs(gain % 10)));
	m_settings.setE4000LNAGain(gain);
}

void MainWindow::on_waterfall_toggled(bool checked)
{
	ui->action_View_Waterfall->setChecked(checked);
	ui->glSpectrum->setDisplayWaterfall(checked);
	m_settings.setDisplayWaterfall(checked);
}

void MainWindow::on_histogram_toggled(bool checked)
{
	ui->action_View_Histogram->setChecked(checked);
	ui->glSpectrum->setDisplayHistogram(checked);
	m_settings.setDisplayHistogram(checked);
}

void MainWindow::on_liveSpectrum_toggled(bool checked)
{
	ui->action_View_LiveSpectrum->setChecked(checked);
	ui->glSpectrum->setDisplayLiveSpectrum(checked);
	m_settings.setDisplayLiveSpectrum(checked);
}

void MainWindow::on_refLevel_valueChanged(int value)
{
	ui->glSpectrum->setReferenceLevel(value * 10);
	ui->refLevelDisplay->setText(tr("%1").arg(value * 10));
}

void MainWindow::on_levelRange_valueChanged(int value)
{
	ui->glSpectrum->setPowerRange(value * 10);
	ui->levelRangeDisplay->setText(tr("%1").arg(value * 10));
}
