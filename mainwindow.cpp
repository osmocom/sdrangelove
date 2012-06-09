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

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_settings(),
	m_dspEngine(&m_settings)
{
	m_settings.load();

	ui->setupUi(this);
	createStatusBar();

	m_dspEngine.setGLSpectrum(ui->glSpectrum);
	m_dspEngine.start();
	m_lastEngineState = (DSPEngine::State)-1;

	connect(&m_statusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
	m_statusTimer.start(500);

	statusBar()->showMessage("Welcome to SDRangelove", 3000);

	ui->centerFrequency->setValueRange(20000000U, 2200000000U);

	for(int i = 0; i < ui->fftSize->count(); i++) {
		if(ui->fftSize->itemText(i).toInt() == m_settings.fftSize()) {
			ui->fftSize->setCurrentIndex(i);
			break;
		}
	}
	ui->fftWindow->setCurrentIndex(m_settings.fftWindow());
	ui->liveSpectrum->setValue(m_settings.liveSpectrumAlpha());
	ui->glSpectrum->setLiveSpectrumAlpha(m_settings.liveSpectrumAlpha());
	ui->iqSwap->setChecked(m_settings.iqSwap());
	ui->decimation->setCurrentIndex(m_settings.decimation() - 2);
	ui->dcOffset->setChecked(m_settings.dcOffsetCorrection());
	ui->iqImbalance->setChecked(m_settings.iqImbalanceCorrection());
	ui->e4000LNAGain->setCurrentIndex((m_settings.e4000LNAGain() + 50) / 25);
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

	updateSampleRate();
	updateCenterFreqDisplay();
}

MainWindow::~MainWindow()
{
	m_dspEngine.stop();

	m_settings.save();
	delete ui;
}

void MainWindow::createStatusBar()
{
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
	ui->centerFrequency->setValue(freq);
	ui->glSpectrum->setCenterFrequency(freq);
}

void MainWindow::updateSampleRate()
{
	m_sampleRate = 4000000 / (1 << m_settings.decimation());
	ui->glSpectrum->setSampleRate(m_sampleRate);
	ui->sampleRate->setText(tr("%1k").arg((float)m_sampleRate / 1000));
}

void MainWindow::updateStatus()
{
	if(m_lastEngineState != m_dspEngine.state()) {
		switch(m_dspEngine.state()) {
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
				break;

			case DSPEngine::StRunning:
				m_engineIdle->setColor(Qt::gray);
				m_engineRunning->setColor(Qt::green);
				m_engineError->setColor(Qt::gray);
				statusBar()->clearMessage();
				break;

			case DSPEngine::StError:
				m_engineIdle->setColor(Qt::gray);
				m_engineRunning->setColor(Qt::gray);
				m_engineError->setColor(Qt::red);
				statusBar()->showMessage(m_dspEngine.errorMsg());
				break;
		}
		m_lastEngineState = m_dspEngine.state();
	}
}

void MainWindow::on_action_Start_triggered()
{
	m_dspEngine.startAcquisition();
}

void MainWindow::on_action_Stop_triggered()
{
	m_dspEngine.stopAcquistion();
}

void MainWindow::on_fftSize_currentIndexChanged(const QString& str)
{
	m_settings.setFFTSize(str.toInt());
}

void MainWindow::on_fftWindow_currentIndexChanged(int index)
{
	m_settings.setFFTWindow(index);
}

void MainWindow::on_iqSwap_toggled(bool checked)
{
	m_settings.setIQSwap(checked);
}

void MainWindow::on_decimation_currentIndexChanged(int index)
{
	m_settings.setDecimation(index + 2);
	updateSampleRate();
}

void MainWindow::on_e4000LNAGain_currentIndexChanged(int index)
{
	m_settings.setE4000LNAGain(index * 25 - 50);
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
	m_settings.setCenterFreq(value);
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

void MainWindow::on_liveSpectrum_sliderMoved(int position)
{
	ui->glSpectrum->setLiveSpectrumAlpha(position);
}
