#include "rtlsdrgui.h"
#include "ui_rtlsdrgui.h"

RTLSDRGui::RTLSDRGui(MessageQueue* msgQueue, QWidget* parent) :
	SampleSourceGUI(parent),
	ui(new Ui::RTLSDRGui),
	m_msgQueue(msgQueue),
	m_settings()
{
	ui->setupUi(this);
	ui->centerFrequency->setValueRange(7, 20000U, 2200000U);
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateHardware()));
	displaySettings();
}

RTLSDRGui::~RTLSDRGui()
{
	delete ui;
}

QString RTLSDRGui::serializeSettings() const
{
	return m_settings.serialize();
}

bool RTLSDRGui::deserializeSettings(const QString& settings)
{
	if(m_settings.deserialize(settings)) {
		displaySettings();
		sendSettings();
		return true;
	} else {
		return false;
	}
}

void RTLSDRGui::handleSourceMessage(DSPCmdSourceToGUI* cmd)
{
	if(cmd->sourceType() != DSPCmdGUIInfoRTLSDR::SourceType)
		return;

	m_gains = ((DSPCmdGUIInfoRTLSDR*)cmd)->getGains();
	displaySettings();
}

void RTLSDRGui::displaySettings()
{
	ui->centerFrequency->setValue(m_settings.centerFrequency / 1000);
	ui->decimation->setValue(m_settings.decimation);

	if(m_gains.size() > 0) {
		int dist = abs(m_settings.gain - m_gains[0]);
		int pos = 0;
		for(int i = 1; i < m_gains.size(); i++) {
			if(abs(m_settings.gain - m_gains[i]) < dist) {
				dist = abs(m_settings.gain - m_gains[i]);
				pos = i;
			}
		}
		ui->gainText->setText(tr("%1.%2").arg(m_gains[pos] / 10).arg(abs(m_gains[pos] % 10)));
		ui->gain->setMaximum(m_gains.size() - 1);
		ui->gain->setEnabled(true);
		ui->gain->setValue(pos);
	} else {
		ui->gain->setMaximum(0);
		ui->gain->setEnabled(false);
		ui->gain->setValue(0);
	}
}

void RTLSDRGui::sendSettings()
{
	if(!m_updateTimer.isActive())
		m_updateTimer.start(100);
}

void RTLSDRGui::on_centerFrequency_changed(quint64 value)
{
	m_settings.centerFrequency = value * 1000;
	sendSettings();
}

void RTLSDRGui::on_gain_valueChanged(int value)
{
	if(value > m_gains.size())
		return;
	int gain = m_gains[value];
	ui->gainText->setText(tr("%1.%2").arg(gain / 10).arg(abs(gain % 10)));
	m_settings.gain = gain;
	sendSettings();
}

void RTLSDRGui::on_decimation_valueChanged(int value)
{
	ui->decimationText->setText(tr("1:%1").arg(1 << value));
	m_settings.decimation = value;
	sendSettings();
}

void RTLSDRGui::updateHardware()
{
	DSPCmdConfigureSourceRTLSDR* cmd = DSPCmdConfigureSourceRTLSDR::create(m_settings);
	cmd->submit(m_msgQueue);
	m_updateTimer.stop();
}
