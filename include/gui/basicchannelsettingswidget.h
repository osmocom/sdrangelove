#ifndef INCLUDE_BASICCHANNELSETTINGSWIDGET_H
#define INCLUDE_BASICCHANNELSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
	class BasicChannelSettingsWidget;
}

class BasicChannelSettingsWidget : public QWidget {
	Q_OBJECT

public:
	explicit BasicChannelSettingsWidget(QWidget* parent = NULL);
	~BasicChannelSettingsWidget();

private:
	Ui::BasicChannelSettingsWidget* ui;
};

#endif // INCLUDE_BASICCHANNELSETTINGSWIDGET_H
