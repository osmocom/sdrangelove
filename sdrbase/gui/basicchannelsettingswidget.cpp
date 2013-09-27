#include "gui/basicchannelsettingswidget.h"
#include "ui_basicchannelsettingswidget.h"

BasicChannelSettingsWidget::BasicChannelSettingsWidget(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::BasicChannelSettingsWidget)
{
	ui->setupUi(this);
}

BasicChannelSettingsWidget::~BasicChannelSettingsWidget()
{
	delete ui;
}
