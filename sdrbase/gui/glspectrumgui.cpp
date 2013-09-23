#include "gui/glspectrumgui.h"
#include "dsp/fftwindow.h"
#include "dsp/spectrumvis.h"
#include "gui/glspectrum.h"
#include "util/simpleserializer.h"
#include "ui_glspectrumgui.h"

GLSpectrumGUI::GLSpectrumGUI(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::GLSpectrumGUI),
	m_messageQueue(NULL),
	m_spectrumVis(NULL),
	m_glSpectrum(NULL),
	m_fftSize(1024),
	m_fftOverlap(10),
	m_fftWindow(FFTWindow::Hamming),
	m_refLevel(0),
	m_powerRange(100),
	m_displayWaterfall(true),
	m_invertedWaterfall(false),
	m_displayMaxHold(false),
	m_displayHistogram(true)
{
	ui->setupUi(this);
}

GLSpectrumGUI::~GLSpectrumGUI()
{
	delete ui;
}

void GLSpectrumGUI::setBuddies(MessageQueue* messageQueue, SpectrumVis* spectrumVis, GLSpectrum* glSpectrum)
{
	m_messageQueue = messageQueue;
	m_spectrumVis = spectrumVis;
	m_glSpectrum = glSpectrum;
	applySettings();
}

void GLSpectrumGUI::resetToDefaults()
{
	m_fftSize = 1024;
	m_fftOverlap = 10;
	m_fftWindow = FFTWindow::Hamming;
	m_refLevel = 0;
	m_powerRange = 100;
	m_displayWaterfall = true;
	m_invertedWaterfall = false;
	m_displayMaxHold = false;
	m_displayHistogram = true;
	applySettings();
}

QByteArray GLSpectrumGUI::serialize() const
{
	SimpleSerializer s(1);
	s.writeS32(1, m_fftSize);
	s.writeS32(2, m_fftOverlap);
	s.writeS32(3, m_fftWindow);
	s.writeReal(4, m_refLevel);
	s.writeReal(5, m_powerRange);
	s.writeBool(6, m_displayWaterfall);
	s.writeBool(7, m_invertedWaterfall);
	s.writeBool(8, m_displayMaxHold);
	s.writeBool(9, m_displayHistogram);
	return s.final();
}

bool GLSpectrumGUI::deserialize(const QByteArray& data)
{
	SimpleDeserializer d(data);

	if(!d.isValid()) {
		resetToDefaults();
		return false;
	}

	if(d.getVersion() == 1) {
		d.readS32(1, &m_fftSize, 1024);
		d.readS32(2, &m_fftOverlap, 10);
		d.readS32(3, &m_fftWindow, FFTWindow::Hamming);
		d.readReal(4, &m_refLevel, 0);
		d.readReal(5, &m_powerRange, 100);
		d.readBool(6, &m_displayWaterfall, true);
		d.readBool(7, &m_invertedWaterfall, false);
		d.readBool(8, &m_displayMaxHold, false);
		d.readBool(9, &m_displayHistogram, true);
		applySettings();
		return true;
	} else {
		resetToDefaults();
		return false;
	}
}

void GLSpectrumGUI::applySettings()
{
	for(int i = 0; i < 6; i++) {
		if(m_fftSize == (1 << (i + 7))) {
			ui->fftSize->setValue(i);
			break;
		}
	}
	ui->fftWindow->setCurrentIndex(m_fftWindow);
	ui->waterfall->setChecked(m_displayWaterfall);
	m_glSpectrum->setDisplayWaterfall(m_displayWaterfall);
	m_glSpectrum->setInvertedWaterfall(m_invertedWaterfall);
	ui->maxHold->setChecked(m_displayMaxHold);
	m_glSpectrum->setDisplayMaxHold(m_displayMaxHold);
	ui->histogram->setChecked(m_displayHistogram);
	m_glSpectrum->setDisplayHistogram(m_displayHistogram);
	ui->refLevel->setValue((int)(m_refLevel / 10.0));
	ui->levelRange->setValue((int)(m_powerRange / 10.0));

	m_spectrumVis->configure(m_messageQueue, m_fftSize, m_fftOverlap, (FFTWindow::Function)m_fftWindow);
}

void GLSpectrumGUI::on_fftSize_valueChanged(int value)
{
	m_fftSize = 1 << (7 + value);
	ui->fftSizeDisplay->setText(tr("%1").arg(m_fftSize));
	m_spectrumVis->configure(m_messageQueue, m_fftSize, m_fftOverlap, (FFTWindow::Function)m_fftWindow);
}

void GLSpectrumGUI::on_fftWindow_currentIndexChanged(int index)
{
	m_fftWindow = index;
	m_spectrumVis->configure(m_messageQueue, m_fftSize, m_fftOverlap, (FFTWindow::Function)m_fftWindow);
}

void GLSpectrumGUI::on_refLevel_valueChanged(int value)
{
	m_refLevel = value * 10;
	m_glSpectrum->setReferenceLevel(m_refLevel);
	ui->refLevelDisplay->setText(tr("%1").arg(m_refLevel));
}

void GLSpectrumGUI::on_levelRange_valueChanged(int value)
{
	m_powerRange = value * 10;
	m_glSpectrum->setPowerRange(m_powerRange);
	ui->levelRangeDisplay->setText(tr("%1").arg(m_powerRange));
}

void GLSpectrumGUI::on_decay_valueChanged(int value)
{
}

void GLSpectrumGUI::on_waterfall_toggled(bool checked)
{
	m_displayWaterfall = checked;
	m_glSpectrum->setDisplayWaterfall(m_displayWaterfall);
}

void GLSpectrumGUI::on_histogram_toggled(bool checked)
{
	m_displayHistogram = checked;
	m_glSpectrum->setDisplayHistogram(m_displayHistogram);
}

void GLSpectrumGUI::on_maxHold_toggled(bool checked)
{
	m_displayMaxHold = checked;
	m_glSpectrum->setDisplayMaxHold(m_displayMaxHold);
}
