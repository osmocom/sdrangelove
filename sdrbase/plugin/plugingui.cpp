#include "plugin/plugingui.h"

PluginGUI::PluginGUI(QWidget* parent) :
	QWidget(parent)
{
}

void PluginGUI::setWidgetName(const QString& name)
{
	setObjectName(name);
}

QByteArray PluginGUI::serializeGeneral() const
{
	return QByteArray();
}

bool PluginGUI::deserializeGeneral(const QByteArray& data)
{
	return false;
}

quint64 PluginGUI::getCenterFrequency() const
{
	return 0;
}
