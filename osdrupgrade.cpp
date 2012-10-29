#include "osdrupgrade.h"
#include "ui_osdrupgrade.h"

OSDRUpgrade::OSDRUpgrade(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::OSDRUpgrade)
{
	ui->setupUi(this);
}

OSDRUpgrade::~OSDRUpgrade()
{
	delete ui;
}
