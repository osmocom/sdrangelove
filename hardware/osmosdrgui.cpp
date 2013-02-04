#include "osmosdrgui.h"
#include "ui_osmosdrgui.h"

OsmoSDRGui::OsmoSDRGui(MessageQueue* msgQueue, QWidget* parent) :
	SampleSourceGUI(parent),
	ui(new Ui::OsmoSDRGui),
	m_msgQueue(msgQueue),
	m_settings()
{
	ui->setupUi(this);
	ui->centerFrequency->setValueRange(7, 20000U, 2200000U);
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateHardware()));
	displaySettings();
}

OsmoSDRGui::~OsmoSDRGui()
{
	delete ui;
}

QString OsmoSDRGui::serializeSettings() const
{
	return m_settings.serialize();
}

bool OsmoSDRGui::deserializeSettings(const QString& settings)
{
	if(m_settings.deserialize(settings)) {
		displaySettings();
		sendSettings();
		return true;
	} else {
		return false;
	}
}

void OsmoSDRGui::displaySettings()
{
	ui->centerFrequency->setValue(m_settings.centerFrequency / 1000);
	ui->iqSwap->setChecked(m_settings.swapIQ);
	ui->decimation->setValue(m_settings.decimation);
	ui->e4000LNAGain->setValue(e4kLNAGainToIdx(m_settings.lnaGain));

	ui->e4000MixerGain->setCurrentIndex((m_settings.mixerGain - 40) / 80);
	if(m_settings.mixerEnhancement == 0)
		ui->e4000MixerEnh->setCurrentIndex(0);
	else ui->e4000MixerEnh->setCurrentIndex((m_settings.mixerEnhancement + 10) / 20);

	ui->e4000if1->setCurrentIndex((m_settings.if1gain + 30) / 90);
	ui->e4000if2->setCurrentIndex(m_settings.if2gain / 30);
	ui->e4000if3->setCurrentIndex(m_settings.if3gain / 30);
	ui->e4000if4->setCurrentIndex(m_settings.if4gain / 10);
	ui->e4000if5->setCurrentIndex(m_settings.if5gain / 30 - 1);
	ui->e4000if6->setCurrentIndex(m_settings.if6gain / 30 - 1);
	ui->filterI1->setValue(m_settings.opAmpI1);
	ui->filterI2->setValue(m_settings.opAmpI2);
	ui->filterQ1->setValue(m_settings.opAmpQ1);
	ui->filterQ2->setValue(m_settings.opAmpQ2);

	ui->e4kI->setValue(m_settings.dcOfsI);
	ui->e4kQ->setValue(m_settings.dcOfsQ);
}

void OsmoSDRGui::sendSettings()
{
	if(!m_updateTimer.isActive())
		m_updateTimer.start(200);
}

int OsmoSDRGui::e4kLNAGainToIdx(int gain) const
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

int OsmoSDRGui::e4kIdxToLNAGain(int idx) const
{
	static const quint32 gainList[13] = {
		-50, -25, 0, 25, 50, 75, 100, 125, 150, 175, 200, 250, 300
	};
	if((idx < 0) || (idx >= 13))
		return -50;
	else return gainList[idx];
}

void OsmoSDRGui::on_iqSwap_toggled(bool checked)
{
	m_settings.swapIQ = checked;
	sendSettings();
}

void OsmoSDRGui::on_e4000MixerGain_currentIndexChanged(int index)
{
	m_settings.mixerGain = index * 80 + 40;
	sendSettings();
}

void OsmoSDRGui::on_e4000MixerEnh_currentIndexChanged(int index)
{
	if(index == 0)
		m_settings.mixerEnhancement = 0;
	else m_settings.mixerEnhancement = index * 20 - 10;
	sendSettings();
}

void OsmoSDRGui::on_e4000if1_currentIndexChanged(int index)
{
	m_settings.if1gain = index * 90 - 30;
	sendSettings();
}

void OsmoSDRGui::on_e4000if2_currentIndexChanged(int index)
{
	m_settings.if2gain = index * 30;
	sendSettings();
}

void OsmoSDRGui::on_e4000if3_currentIndexChanged(int index)
{
	m_settings.if3gain = index * 30;
	sendSettings();
}

void OsmoSDRGui::on_e4000if4_currentIndexChanged(int index)
{
	m_settings.if4gain = index * 10;
	sendSettings();
}

void OsmoSDRGui::on_e4000if5_currentIndexChanged(int index)
{
	m_settings.if5gain = (index + 1) * 30;
	sendSettings();
}

void OsmoSDRGui::on_e4000if6_currentIndexChanged(int index)
{
	m_settings.if6gain = (index + 1) * 30;
	sendSettings();
}

void OsmoSDRGui::on_centerFrequency_changed(quint64 value)
{
	m_settings.centerFrequency = value * 1000;
	sendSettings();
}

void OsmoSDRGui::on_filterI1_valueChanged(int value)
{
	m_settings.opAmpI1 = value;
	sendSettings();
}

void OsmoSDRGui::on_filterI2_valueChanged(int value)
{
	m_settings.opAmpI2 = value;
	sendSettings();
}

void OsmoSDRGui::on_filterQ1_valueChanged(int value)
{
	m_settings.opAmpQ1 = value;
	sendSettings();
}

void OsmoSDRGui::on_filterQ2_valueChanged(int value)
{
	m_settings.opAmpQ2 = value;
	sendSettings();
}

void OsmoSDRGui::on_decimation_valueChanged(int value)
{
	ui->decimationDisplay->setText(tr("1:%1").arg(1 << value));
	m_settings.decimation = value;
	sendSettings();
}

void OsmoSDRGui::on_e4000LNAGain_valueChanged(int value)
{
	int gain = e4kIdxToLNAGain(value);
	ui->e4000LNAGainDisplay->setText(tr("%1.%2").arg(gain / 10).arg(abs(gain % 10)));
	m_settings.lnaGain = gain;
	sendSettings();
}

void OsmoSDRGui::on_e4kI_valueChanged(int value)
{
	m_settings.dcOfsI = value;
	sendSettings();
}

void OsmoSDRGui::on_e4kQ_valueChanged(int value)
{
	m_settings.dcOfsQ = value;
	sendSettings();
}

void OsmoSDRGui::updateHardware()
{
	DSPCmdConfigureSourceOsmoSDR* cmd = DSPCmdConfigureSourceOsmoSDR::create(m_settings);
	cmd->submit(m_msgQueue);
	m_updateTimer.stop();
}
