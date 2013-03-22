#include <QtPlugin>
#include <QAction>
#include "plugin/pluginapi.h"
#include "nfmplugin.h"
#include "nfmdemodgui.h"

const PluginDescriptor NFMPlugin::m_pluginDescriptor = {
	displayedName: QString("NFM Demodulator"),
	version: QString("---"),
	copyright: QString("(c) maintech GmbH (written by Christian Daniel)"),
	website: QString("http://www.maintech.de"),
	licenseIsGPL: true,
	sourceCodeURL: QString("http://www.maintech.de")
};

NFMPlugin::NFMPlugin(QObject* parent) :
	QObject(parent)
{
}

const PluginDescriptor& NFMPlugin::getPluginDescriptor() const
{
	return m_pluginDescriptor;
}

void NFMPlugin::initPlugin(PluginAPI* pluginAPI)
{
	m_pluginAPI = pluginAPI;

	// register NFM demodulator
	QAction* action = new QAction(tr("&NFM"), this);
	connect(action, SIGNAL(triggered()), this, SLOT(createInstanceNFM()));
	m_pluginAPI->registerDemodulator("de.maintech.sdrangelove.demod.nfm", this, action);
}

PluginGUI* NFMPlugin::createDemod(const QString& demodName)
{
	if(demodName == "de.maintech.sdrangelove.demod.nfm") {
		PluginGUI* gui = NFMDemodGUI::create(m_pluginAPI);
		m_pluginAPI->registerDemodulatorInstance("de.maintech.sdrangelove.demod.nfm", gui);
		return gui;
	} else {
		return NULL;
	}
}

void NFMPlugin::createInstanceNFM()
{
	m_pluginAPI->registerDemodulatorInstance("de.maintech.sdrangelove.demod.nfm", NFMDemodGUI::create(m_pluginAPI));
}

Q_EXPORT_PLUGIN2(nfmPlugin, NFMPlugin);
