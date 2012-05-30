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

#include <QInputDialog>
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
	ui->freqScale->setOrientation(Qt::Horizontal);
	ui->freqScale->setRange(Unit::Frequency, m_settings.centerFreq() - 250000.0, m_settings.centerFreq() + 250000.0);
	ui->timeScale->setOrientation(Qt::Vertical);
	ui->timeScale->setRange(Unit::Time, -1.0, 0.0);
	ui->powerScale->setOrientation(Qt::Vertical);
	ui->powerScale->setRange(Unit::DecibelMilliWatt, 0.0, -100.0);
	updateCenterFreqDisplay();

	m_dspEngine.setWaterfall(ui->waterfall);
	m_dspEngine.setSpectroHistogram(ui->spectroHistogram);
	m_dspEngine.start();
	m_lastEngineState = (DSPEngine::State)-1;

	connect(&m_statusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
	m_statusTimer.start(500);

	statusBar()->showMessage("Welcome to SDRangelove", 3000);

	for(int i = 0; i < ui->fftSize->count(); i++) {
		if(ui->fftSize->itemText(i).toInt() == m_settings.fftSize()) {
			ui->fftSize->setCurrentIndex(i);
			break;
		}
	}
	ui->fftWindow->setCurrentIndex(m_settings.fftWindow());
}

MainWindow::~MainWindow()
{
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
	ui->centerFreq->setText(QString("%1 kHz").arg(freq / 1000));
	ui->freqScale->setRange(Unit::Frequency, freq - 250000.0, freq + 250000.0);
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

void MainWindow::on_freqDown_clicked()
{
	m_settings.setCenterFreq(m_settings.centerFreq() - 100000);
	updateCenterFreqDisplay();
}

void MainWindow::on_freqDown2_clicked()
{
	m_settings.setCenterFreq(m_settings.centerFreq() - 500000);
	updateCenterFreqDisplay();
}

void MainWindow::on_freqDown3_clicked()
{
	m_settings.setCenterFreq(m_settings.centerFreq() - 1000000);
	updateCenterFreqDisplay();
}

void MainWindow::on_freqUp_clicked()
{
	m_settings.setCenterFreq(m_settings.centerFreq() + 100000);
	updateCenterFreqDisplay();
}

void MainWindow::on_freqUp2_clicked()
{
	m_settings.setCenterFreq(m_settings.centerFreq() + 500000);
	updateCenterFreqDisplay();
}

void MainWindow::on_freqUp3_clicked()
{
	m_settings.setCenterFreq(m_settings.centerFreq() + 1000000);
	updateCenterFreqDisplay();
}

void MainWindow::on_freqSet_clicked()
{
	bool ok;
	int freq = QInputDialog::getInt(this, tr("Frequency in kHz"), tr("kHz"), m_settings.centerFreq() / 1000, 0, INT_MAX, 250, &ok);
	if(ok)
		m_settings.setCenterFreq(freq * 1000);
	updateCenterFreqDisplay();
}

void MainWindow::on_fftSize_currentIndexChanged(const QString& str)
{
	m_settings.setFFTSize(str.toInt());
}

void MainWindow::on_fftWindow_currentIndexChanged(int index)
{
	m_settings.setFFTWindow(index);
}
