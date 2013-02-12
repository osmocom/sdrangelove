#ifndef INCLUDE_SAMPLESOURCEGUI_H
#define INCLUDE_SAMPLESOURCEGUI_H

#include <QWidget>
#include "samplesource.h"

class SampleSourceGUI : public QWidget {
	Q_OBJECT

public:
	SampleSourceGUI(QWidget* parent = NULL);

	virtual QString serializeSettings() const = 0;
	virtual bool deserializeSettings(const QString& settings) = 0;

	virtual void handleSourceMessage(DSPCmdSourceToGUI* cmd) = 0;
};

#endif // INCLUDE_SAMPLESOURCEGUI_H
