#ifndef INCLUDE_OSDRUPGRADE_H
#define INCLUDE_OSDRUPGRADE_H

#include <QDialog>

namespace Ui {
	class OSDRUpgrade;
}

class OSDRUpgrade : public QDialog {
	Q_OBJECT

public:
	explicit OSDRUpgrade(QWidget* parent = NULL);
	~OSDRUpgrade();

private:
	Ui::OSDRUpgrade* ui;
};

#endif // INCLUDE_OSDRUPGRADE_H
