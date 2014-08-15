#include <QTreeWidgetItem>
#include <QAudioDeviceInfo>
#include "gui/preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "settings/preferences.h"

PreferencesDialog::PreferencesDialog(Preferences* preferences, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::PreferencesDialog),
	m_preferences(preferences)
{
	ui->setupUi(this);

	QStringList sl;
	bool found;

	QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

	sl.clear();
	sl.append(tr("Default output device (use first suitable)"));
	QTreeWidgetItem* dev = new QTreeWidgetItem(ui->audioTree, sl);
	dev->setFirstColumnSpanned(true);
	dev->setData(0, Qt::UserRole, "");
	for(QList<QAudioDeviceInfo>::ConstIterator it = devices.begin(); it != devices.end(); ++it) {
		sl.clear();
		sl.append(it->deviceName());
		QTreeWidgetItem* dev = new QTreeWidgetItem(ui->audioTree, sl);
		dev->setFirstColumnSpanned(true);
		dev->setData(0, Qt::UserRole, it->deviceName());
	}
	found = false;
	for(int i = 0; i < ui->audioTree->topLevelItemCount(); ++i) {
		if(ui->audioTree->topLevelItem(i)->data(0, Qt::UserRole).toString() == m_preferences->getAudioOutput()) {
			ui->audioTree->setCurrentItem(ui->audioTree->topLevelItem(i));
			found = true;
			break;
		}
	}
	if(!found)
		ui->audioTree->setCurrentItem(ui->audioTree->topLevelItem(0));

	ui->audioRate->addItem(tr("48000 Hz"), 48000);
	ui->audioRate->addItem(tr("44100 Hz"), 44100);
	ui->audioRate->addItem(tr("24000 Hz"), 24000);
	ui->audioRate->addItem(tr("22050 Hz"), 22050);
	found = false;
	for(int i = 0; i < ui->audioRate->count(); ++i) {
		if(ui->audioRate->itemData(i).toInt() == m_preferences->getAudioOutputRate()) {
			ui->audioRate->setCurrentIndex(i);
			found = true;
			break;
		}
	}
	if(!found)
		ui->audioRate->setCurrentIndex(1);

	ui->stackedWidget->setCurrentIndex(0);
	ui->configTree->setCurrentItem(ui->configTree->topLevelItem(0));
}

PreferencesDialog::~PreferencesDialog()
{
	delete ui;
}

void PreferencesDialog::accept()
{
	if(ui->audioTree->currentItem() != NULL)
		m_preferences->setAudioOutput(ui->audioTree->currentItem()->data(0, Qt::UserRole).toString());
	else m_preferences->setAudioOutput(QString());
	m_preferences->setAudioOutputRate(ui->audioRate->itemData(ui->audioRate->currentIndex()).toInt());

	QDialog::accept();
}
