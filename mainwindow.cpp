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
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gui/indicator.h"
#include "gui/presetitem.h"
#include "gui/scopewindow.h"
#include "dsp/spectrumvis.h"
#include "dsp/dspcommands.h"
#include "hardware/samplesourcegui.h"
#include "osdrupgrade.h"
#include "hardware/osmosdrinput.h"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_messageQueue(),
	m_settings(),
	m_dspEngine(&m_messageQueue),
	m_lastEngineState((DSPEngine::State)-1),
	m_startOSDRUpdateAfterStop(false),
	m_scopeWindow(NULL),
	m_sampleRate(0),
	m_centerFrequency(0)
{
	ui->setupUi(this);
	createStatusBar();

	connect(&m_messageQueue, SIGNAL(messageEnqueued()), this, SLOT(handleMessages()), Qt::QueuedConnection);

	loadSettings();

	connect(&m_statusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
	m_statusTimer.start(500);

	m_dspEngine.start();

	m_sampleSource = new OsmoSDRInput();
	m_dspEngine.setSource(m_sampleSource);
	m_sampleSourceGUI = m_sampleSource->createGUI(m_dspEngine.getMessageQueue());
	ui->inputDock->setWidget(m_sampleSourceGUI);
	ui->inputDock->setWindowTitle(ui->inputDock->widget()->windowTitle());

	m_spectrumVis = new SpectrumVis(ui->glSpectrum);
	m_dspEngine.addSink(m_spectrumVis);

	applySettings();
	updatePresets();

/*
	NFMDemod* nfmDemod = new NFMDemod;
	m_dspEngine.addChannelizer(nfmDemod);
*/
/*
	Channelizer* channelizer = new Channelizer;
	//channelizer->setGLSpectrum(ui->chanSpectrum);
	m_dspEngine.addChannelizer(channelizer);
	*/
}

MainWindow::~MainWindow()
{
	m_dspEngine.removeSink(m_spectrumVis);
	m_dspEngine.stop();

	if(m_scopeWindow != NULL) {
		delete m_scopeWindow;
		m_scopeWindow = NULL;
	}

	m_settings.setSourceSettings(m_sampleSourceGUI->serializeSettings());
	saveSettings();
	delete ui;
}

void MainWindow::loadSettings()
{
	if(m_settingsStorage.value("version", 0).toInt() != 3) {
		m_settingsStorage.clear();
		return;
	}

	// load current settings
	m_settings.load(&m_settingsStorage, "");

	// load presets
	QStringList groups = m_settingsStorage.childGroups();
	for(int i = 0; i < groups.size(); ++i) {
		if(groups[i].startsWith("preset")) {
			// load preset name
			m_settingsStorage.beginGroup(groups[i]);
			QString name = m_settingsStorage.value("presetname", tr("Preset %1").arg(m_presetList.size() + 1)).toString();
			m_settingsStorage.endGroup();

			// load preset content
			Settings settings;
			settings.load(&m_settingsStorage, groups[i]);

			// create preset struct
			Preset* preset = new Preset(name, settings);
			m_presetList.append(preset);

			// insert into preset widget
			QStringList strings;
			strings.append(tr("%1").arg(settings.centerFrequency() / 1000));
			strings.append(name);
			PresetItem* item = new PresetItem(ui->presetTree, strings, settings.centerFrequency());
			item->setTextAlignment(0, Qt::AlignRight);
			item->setData(0, Qt::UserRole, qVariantFromValue(preset));
		}
	}
	ui->presetTree->sortItems(0, Qt::AscendingOrder);
}

void MainWindow::saveSettings()
{
	m_settingsStorage.setValue("version", 3);

	// save current settings
	m_settings.save(&m_settingsStorage, "");

	// delete all old presets
	QStringList groups = m_settingsStorage.childGroups();
	for(int i = 0; i < groups.size(); ++i) {
		if(groups[i].startsWith("preset")) {
			m_settingsStorage.remove(groups[i]);
		}
	}

	// save presets
	for(int i = 0; i < m_presetList.size(); ++i) {
		QString group = tr("preset%1").arg(i + 1);
		m_settingsStorage.beginGroup(group);
		m_settingsStorage.setValue("presetname", m_presetList[i]->name());
		m_settingsStorage.endGroup();

		m_presetList[i]->settings()->save(&m_settingsStorage, group);
	}
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
	ui->glSpectrum->setCenterFrequency(m_centerFrequency);
	//ui->glSpectrum->setDisplayChannel(true, freq + 100000.0, 25000.0);
}

void MainWindow::updateSampleRate()
{
	ui->glSpectrum->setSampleRate(m_sampleRate);
	m_sampleRateWidget->setText(tr("Rate: %1 kHz").arg((float)m_sampleRate / 1000));
}

void MainWindow::updatePresets()
{
	ui->presetTree->resizeColumnToContents(0);
	if(ui->presetTree->currentItem() != NULL) {
		ui->presetDelete->setEnabled(true);
		ui->presetLoad->setEnabled(true);
	} else {
		ui->presetDelete->setEnabled(false);
		ui->presetLoad->setEnabled(false);
	}
}

void MainWindow::applySettings()
{
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

	ui->dcOffset->setChecked(m_settings.dcOffsetCorrection());
	ui->iqImbalance->setChecked(m_settings.iqImbalanceCorrection());

	updateCenterFreqDisplay();
	updateSampleRate();

	m_dspEngine.configureCorrections(m_settings.dcOffsetCorrection(), m_settings.iqImbalanceCorrection());
	m_spectrumVis->configure(m_dspEngine.getMessageQueue(), m_settings.fftSize(), m_settings.fftOverlap(), (FFTWindow::Function)m_settings.fftWindow());
	m_sampleSourceGUI->deserializeSettings(m_settings.sourceSettings());
}

void MainWindow::handleMessages()
{
	Message* cmd;
	while((cmd = m_messageQueue.accept()) != NULL) {
		qDebug("CMD: %s", cmd->name());

		switch(cmd->type()) {
			case DSPRepEngineReport::Type: {
				DSPRepEngineReport* rep = (DSPRepEngineReport*)cmd;
				m_sampleRate = rep->getSampleRate();
				m_centerFrequency = rep->getCenterFrequency();
				qDebug("SampleRate:%d, CenterFrequency:%llu", rep->getSampleRate(), rep->getCenterFrequency());
				updateCenterFreqDisplay();
				updateSampleRate();
				cmd->completed();
				break;
			}

			default:
				cmd->completed();
				break;
		}
	}
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
				statusBar()->showMessage(tr("Sampling from %1").arg(m_dspEngine.deviceDescription()));
				break;

			case DSPEngine::StError:
				m_engineIdle->setColor(Qt::gray);
				m_engineRunning->setColor(Qt::gray);
				m_engineError->setColor(Qt::red);
				statusBar()->showMessage(tr("Error: %1").arg(m_dspEngine.errorMessage()));
				if(m_startOSDRUpdateAfterStop)
					on_actionOsmoSDR_Firmware_Upgrade_triggered();
				break;
		}
		m_lastEngineState = state;
	}
}

void MainWindow::scopeWindowDestroyed()
{
	m_scopeWindow = NULL;
	ui->action_Oscilloscope->setChecked(false);
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
	m_spectrumVis->configure(m_dspEngine.getMessageQueue(), m_settings.fftSize(), m_settings.fftOverlap(), (FFTWindow::Function)m_settings.fftWindow());
}

void MainWindow::on_action_Debug_triggered()
{
//	m_dspEngine.triggerDebug();
}

void MainWindow::on_dcOffset_toggled(bool checked)
{
	m_settings.setDCOffsetCorrection(checked);
	m_dspEngine.configureCorrections(m_settings.dcOffsetCorrection(), m_settings.iqImbalanceCorrection());
}

void MainWindow::on_iqImbalance_toggled(bool checked)
{
	m_settings.setIQImbalanceCorrection(checked);
	m_dspEngine.configureCorrections(m_settings.dcOffsetCorrection(), m_settings.iqImbalanceCorrection());
}

void MainWindow::on_action_View_Waterfall_toggled(bool checked)
{
	m_settings.setDisplayWaterfall(checked);
	ui->action_View_Waterfall->setChecked(checked);
	ui->glSpectrum->setDisplayWaterfall(checked);
}

void MainWindow::on_action_View_Histogram_toggled(bool checked)
{
	m_settings.setDisplayHistogram(checked);
	ui->action_View_Histogram->setChecked(checked);
	ui->glSpectrum->setDisplayHistogram(checked);
}

void MainWindow::on_action_View_LiveSpectrum_toggled(bool checked)
{
	m_settings.setDisplayLiveSpectrum(checked);
	ui->action_View_LiveSpectrum->setChecked(checked);
	ui->glSpectrum->setDisplayLiveSpectrum(checked);
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

void MainWindow::on_fftSize_valueChanged(int value)
{
	ui->fftSizeDisplay->setText(tr("%1").arg(1 << (7 + value)));
	m_settings.setFFTSize(1 << (7 + value));

	m_spectrumVis->configure(m_dspEngine.getMessageQueue(), m_settings.fftSize(), m_settings.fftOverlap(), (FFTWindow::Function)m_settings.fftWindow());
}

void MainWindow::on_waterfall_toggled(bool checked)
{
	m_settings.setDisplayWaterfall(checked);
	ui->action_View_Waterfall->setChecked(checked);
	ui->glSpectrum->setDisplayWaterfall(checked);
}

void MainWindow::on_histogram_toggled(bool checked)
{
	m_settings.setDisplayHistogram(checked);
	ui->action_View_Histogram->setChecked(checked);
	ui->glSpectrum->setDisplayHistogram(checked);
}

void MainWindow::on_liveSpectrum_toggled(bool checked)
{
	m_settings.setDisplayLiveSpectrum(checked);
	ui->action_View_LiveSpectrum->setChecked(checked);
	ui->glSpectrum->setDisplayLiveSpectrum(checked);
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

void MainWindow::on_presetSave_clicked()
{
	bool ok;
	QString name = QInputDialog::getText(this, tr("Save Preset"), tr("Preset name"), QLineEdit::Normal, tr(""), &ok);
	if(!ok)
		return;

	m_settings.setCenterFrequency(m_centerFrequency);
	m_settings.setSourceSettings(m_sampleSourceGUI->serializeSettings());

	Preset* preset = new Preset(name, m_settings);
	m_presetList.append(preset);

	QStringList strings;
	strings.append(tr("%1").arg(m_centerFrequency / 1000));
	strings.append(name);
	PresetItem* item = new PresetItem(ui->presetTree, strings, m_settings.centerFrequency()); // FIXME
	item->setTextAlignment(0, Qt::AlignRight);
	item->setData(0, Qt::UserRole, qVariantFromValue(preset));
	ui->presetTree->sortItems(0, Qt::AscendingOrder);
}

void MainWindow::on_presetLoad_clicked()
{
	QTreeWidgetItem* item = ui->presetTree->currentItem();
	if(item == NULL) {
		updatePresets();
		return;
	}
	Preset* preset = qvariant_cast<Preset*>(item->data(0, Qt::UserRole));

	m_settings = *preset->settings();
	applySettings();
}

void MainWindow::on_presetDelete_clicked()
{
	QTreeWidgetItem* item = ui->presetTree->currentItem();
	if(item == NULL) {
		updatePresets();
		return;
	}
	Preset* preset = qvariant_cast<Preset*>(item->data(0, Qt::UserRole));

	if(QMessageBox::question(this, tr("Delete Preset"), tr("Do you want to delete preset '%1'?").arg(preset->name()), QMessageBox::No | QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
		delete item;
		m_presetList.removeAll(preset);
	}
}

void MainWindow::on_presetTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	updatePresets();
}

void MainWindow::on_presetTree_itemActivated(QTreeWidgetItem *item, int column)
{
	on_presetLoad_clicked();
}

void MainWindow::on_action_Oscilloscope_triggered()
{
	if(m_scopeWindow == NULL) {
		m_scopeWindow = new ScopeWindow();
		connect(m_scopeWindow, SIGNAL(destroyed()), this, SLOT(scopeWindowDestroyed()));
		m_scopeWindow->show();
	} else {
		delete m_scopeWindow;
		m_scopeWindow = NULL;
	}
}
