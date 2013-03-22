#ifndef INCLUDE_NFMPLUGIN_H
#define INCLUDE_NFMPLUGIN_H

#include <QObject>
#include "plugin/plugininterface.h"

class NFMPlugin : public QObject, PluginInterface {
	Q_OBJECT
	Q_INTERFACES(PluginInterface)

public:
	explicit NFMPlugin(QObject* parent = NULL);

	const PluginDescriptor& getPluginDescriptor() const;
	void initPlugin(PluginAPI* pluginAPI);

	PluginGUI* createDemod(const QString& demodName);

private:
	static const PluginDescriptor m_pluginDescriptor;

	PluginAPI* m_pluginAPI;

private slots:
	void createInstanceNFM();
};

#endif // INCLUDE_NFMPLUGIN_H
