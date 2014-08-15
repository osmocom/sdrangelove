#ifndef INCLUDE_PREFERENCESDIALOG_H
#define INCLUDE_PREFERENCESDIALOG_H

#include <QDialog>

class Preferences;

namespace Ui {
	class PreferencesDialog;
}

class PreferencesDialog : public QDialog {
	Q_OBJECT

public:
	explicit PreferencesDialog(Preferences* preferences, QWidget* parent = NULL);
	~PreferencesDialog();

private:
	Ui::PreferencesDialog* ui;

	Preferences* m_preferences;

private slots:
	void accept();
};

#endif // INCLUDE_PREFERENCESDIALOG_H
