#include <QtPlugin>
#include <QAction>
#include "plugin/pluginapi.h"
#include "util/simpleserializer.h"
#include "gnuradioplugin.h"
#include "gnuradiogui.h"

const PluginDescriptor GNURadioPlugin::m_pluginDescriptor = {
	displayedName: QString("GR-OsmoSDR Input"),
	version: QString("---"),
	copyright: QString("(c) Dimitri Stolnikov <horiz0n@gmx.net>"),
	website: QString("http://sdr.osmocom.org/trac/wiki/gr-osmosdr"),
	licenseIsGPL: true,
	sourceCodeURL: QString("http://cgit.osmocom.org/cgit/gr-osmosdr")
};

GNURadioPlugin::GNURadioPlugin(QObject* parent) :
	QObject(parent)
{
}

const PluginDescriptor& GNURadioPlugin::getPluginDescriptor() const
{
	return m_pluginDescriptor;
}

void GNURadioPlugin::initPlugin(PluginAPI* pluginAPI)
{
	m_pluginAPI = pluginAPI;

	m_pluginAPI->registerSampleSource("org.osmocom.sdr.samplesource.gr-osmosdr", this);
}

PluginInterface::SampleSourceDevices GNURadioPlugin::enumSampleSources()
{
	SampleSourceDevices result;

	result.append(SampleSourceDevice("GNURadio OsmoSDR Driver", "org.osmocom.sdr.samplesource.gr-osmosdr", QByteArray()));

	return result;
}

PluginGUI* GNURadioPlugin::createSampleSource(const QString& sourceName, const QByteArray& address)
{
	if(sourceName == "org.osmocom.sdr.samplesource.gr-osmosdr") {
		return new GNURadioGui(m_pluginAPI);
	} else {
		return NULL;
	}
}

Q_EXPORT_PLUGIN2(gnuRadioPlugin, GNURadioPlugin);
