#include <QApplication>
#include <QPluginLoader>
#include <QComboBox>
#include "plugin/pluginmanager.h"
#include "plugin/plugingui.h"
#include "settings/preset.h"
#include "mainwindow.h"
#include "dsp/dspengine.h"
#include "dsp/samplesource/samplesource.h"

PluginManager::PluginManager(MainWindow* mainWindow, DSPEngine* dspEngine, QObject* parent) :
	QObject(parent),
	m_pluginAPI(this, mainWindow, dspEngine),
	m_mainWindow(mainWindow),
	m_dspEngine(dspEngine),
	m_sampleSource(),
	m_sampleSourceInstance(NULL)
{
}

PluginManager::~PluginManager()
{
	freeAll();
}

void PluginManager::loadPlugins()
{
	QDir pluginsDir = QDir(QApplication::instance()->applicationDirPath());

	loadPlugins(pluginsDir);

	qSort(m_plugins);

	for(Plugins::const_iterator it = m_plugins.begin(); it != m_plugins.end(); ++it)
		it->plugin->initPlugin(&m_pluginAPI);

	updateSampleSourceDevices();
}

void PluginManager::registerDemodulator(const QString& demodName, PluginInterface* plugin, QAction* action)
{
	m_demodRegistrations.append(DemodRegistration(demodName, plugin));
	m_mainWindow->addDemodCreateAction(action);
}

void PluginManager::registerDemodulatorInstance(const QString& demodName, PluginGUI* pluginGUI)
{
	connect(pluginGUI, SIGNAL(destroyed(QObject*)), this, SLOT(demodInstanceDestroyed(QObject*)));
	m_demodInstanceRegistrations.append(DemodInstanceRegistration(demodName, pluginGUI));
	renameDemodInstances();
}

void PluginManager::registerSampleSource(const QString& sourceName, PluginInterface* plugin)
{
	m_sampleSourceRegistrations.append(SampleSourceRegistration(sourceName, plugin));
}

void PluginManager::loadSettings(const Preset* preset)
{
	qDebug("-------- [%s | %s] --------", qPrintable(preset->getGroup()), qPrintable(preset->getDescription()));

	// copy currently open demods and clear list
	DemodInstanceRegistrations availableDemods = m_demodInstanceRegistrations;
	m_demodInstanceRegistrations.clear();

	for(int i = 0; i < preset->getDemodCount(); i++) {
		const Preset::DemodConfig& demodConfig = preset->getDemodConfig(i);
		DemodInstanceRegistration reg;
		// if we have one instance available already, use it
		for(int i = 0; i < availableDemods.count(); i++) {
			qDebug("compare [%s] vs [%s]", qPrintable(availableDemods[i].m_demodName), qPrintable(demodConfig.m_demod));
			if(availableDemods[i].m_demodName == demodConfig.m_demod) {
				qDebug("demod [%s] found", qPrintable(availableDemods[i].m_demodName));
				reg = availableDemods.takeAt(i);
				m_demodInstanceRegistrations.append(reg);
				break;
			}
		}
		// if we haven't one already, create one
		if(reg.m_gui == NULL) {
			for(int i = 0; i < m_demodRegistrations.count(); i++) {
				if(m_demodRegistrations[i].m_demodName == demodConfig.m_demod) {
					qDebug("creating new demod [%s]", qPrintable(demodConfig.m_demod));
					reg = DemodInstanceRegistration(demodConfig.m_demod, m_demodRegistrations[i].m_plugin->createDemod(demodConfig.m_demod));
					break;
				}
			}
		}
		if(reg.m_gui != NULL) {
			reg.m_gui->deserialize(demodConfig.m_config);
			reg.m_gui->raise();
		}
	}

	// everything, that is still "available" is not needed anymore
	for(int i = 0; i < availableDemods.count(); i++) {
		qDebug("destroying spare demod [%s]", qPrintable(availableDemods[i].m_demodName));
		availableDemods[i].m_gui->destroy();
	}

	renameDemodInstances();

	if(m_sampleSourceInstance != NULL) {
		m_sampleSourceInstance->deserializeGeneral(preset->getSourceGeneralConfig());
		if(m_sampleSource == preset->getSource()) {
			m_sampleSourceInstance->deserialize(preset->getSourceConfig());
		}
	}
}

void PluginManager::saveSettings(Preset* preset) const
{
	if(m_sampleSourceInstance != NULL) {
		preset->setSourceConfig(m_sampleSource, m_sampleSourceInstance->serializeGeneral(), m_sampleSourceInstance->serialize());
		preset->setCenterFrequency(m_sampleSourceInstance->getCenterFrequency());
	} else {
		preset->setSourceConfig(QString::null, QByteArray(), QByteArray());
	}
	for(int i = 0; i < m_demodInstanceRegistrations.size(); i++)
		preset->addDemod(m_demodInstanceRegistrations[i].m_demodName, m_demodInstanceRegistrations[i].m_gui->serialize());
}

void PluginManager::freeAll()
{
	m_dspEngine->stopAcquistion();

	while(!m_demodInstanceRegistrations.isEmpty()) {
		DemodInstanceRegistration reg(m_demodInstanceRegistrations.takeLast());
		reg.m_gui->disconnect(this);
		reg.m_gui->destroy();
	}

	if(m_sampleSourceInstance != NULL) {
		m_dspEngine->setSource(NULL);
		m_sampleSourceInstance->destroy();
		m_sampleSourceInstance = NULL;
		m_sampleSource.clear();
	}
}

bool PluginManager::handleMessage(Message* message)
{
	if(m_sampleSourceInstance != NULL) {
		if((message->destination() == NULL) || (message->destination() == m_sampleSourceInstance)) {
			if(m_sampleSourceInstance->handleMessage(message))
				return true;
		}
	}

	for(DemodInstanceRegistrations::iterator it = m_demodInstanceRegistrations.begin(); it != m_demodInstanceRegistrations.end(); ++it) {
		if((message->destination() == NULL) || (message->destination() == it->m_gui)) {
			if(it->m_gui->handleMessage(message))
				return true;
		}
	}
	return false;
}

void PluginManager::updateSampleSourceDevices()
{
	m_sampleSourceDevices.clear();
	for(int i = 0; i < m_sampleSourceRegistrations.count(); ++i) {
		PluginInterface::SampleSourceDevices ssd = m_sampleSourceRegistrations[i].m_plugin->enumSampleSources();
		for(int j = 0; j < ssd.count(); ++j)
			m_sampleSourceDevices.append(SampleSourceDevice(m_sampleSourceRegistrations[i].m_plugin, ssd[j].displayedName, ssd[j].name, ssd[j].address));
	}
}

void PluginManager::fillSampleSourceSelector(QComboBox* comboBox)
{
	comboBox->clear();
	for(int i = 0; i < m_sampleSourceDevices.count(); i++)
		comboBox->addItem(m_sampleSourceDevices[i].m_displayName, i);
}

int PluginManager::selectSampleSource(int index)
{
	m_dspEngine->stopAcquistion();

	if(m_sampleSourceInstance != NULL) {
		m_dspEngine->stopAcquistion();
		m_dspEngine->setSource(NULL);
		m_sampleSourceInstance->destroy();
		m_sampleSourceInstance = NULL;
		m_sampleSource.clear();
	}

	if(index == -1) {
		if(!m_sampleSource.isEmpty()) {
			for(int i = 0; i < m_sampleSourceDevices.count(); i++) {
				if(m_sampleSourceDevices[i].m_sourceName == m_sampleSource) {
					index = i;
					break;
				}
			}
		}
		if(index == -1) {
			if(m_sampleSourceDevices.count() > 0)
				index = 0;
		}
	}
	if(index == -1)
		return -1;

	m_sampleSource = m_sampleSourceDevices[index].m_sourceName;
	m_sampleSourceInstance = m_sampleSourceDevices[index].m_plugin->createSampleSource(m_sampleSource, m_sampleSourceDevices[index].m_address);
	m_mainWindow->setInputGUI(m_sampleSourceInstance);
	return index;
}

int PluginManager::selectSampleSource(const QString& source)
{
	int index = -1;

	m_dspEngine->stopAcquistion();

	if(m_sampleSourceInstance != NULL) {
		m_dspEngine->stopAcquistion();
		m_dspEngine->setSource(NULL);
		m_sampleSourceInstance->destroy();
		m_sampleSourceInstance = NULL;
		m_sampleSource.clear();
	}

	qDebug("finding sample source [%s]", qPrintable(source));
	for(int i = 0; i < m_sampleSourceDevices.count(); i++) {
		qDebug("*** %s vs %s", qPrintable(m_sampleSourceDevices[i].m_sourceName), qPrintable(source));
		if(m_sampleSourceDevices[i].m_sourceName == source) {
			index = i;
			break;
		}
	}
	if(index == -1) {
		if(m_sampleSourceDevices.count() > 0)
			index = 0;
	}
	if(index == -1)
		return -1;

	m_sampleSource = m_sampleSourceDevices[index].m_sourceName;
	m_sampleSourceInstance = m_sampleSourceDevices[index].m_plugin->createSampleSource(m_sampleSource, m_sampleSourceDevices[index].m_address);
	m_mainWindow->setInputGUI(m_sampleSourceInstance);
	return index;
}

void PluginManager::demodInstanceDestroyed(QObject* object)
{
	for(DemodInstanceRegistrations::iterator it = m_demodInstanceRegistrations.begin(); it != m_demodInstanceRegistrations.end(); ++it) {
		if(it->m_gui == object) {
			m_demodInstanceRegistrations.erase(it);
			break;
		}
	}
	renameDemodInstances();
}

void PluginManager::loadPlugins(const QDir& dir)
{
	QDir pluginsDir(dir);
	foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader* loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
		PluginInterface* plugin = qobject_cast<PluginInterface*>(loader->instance());
		if(loader->isLoaded())
			qDebug("loaded plugin %s", qPrintable(fileName));
		if(plugin != NULL) {
			m_plugins.append(Plugin(fileName, loader, plugin));
		} else {
			loader->unload();
			delete loader;
		}
	}
	foreach(QString dirName, pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
		loadPlugins(pluginsDir.absoluteFilePath(dirName));
}

void PluginManager::renameDemodInstances()
{
	for(int i = 0; i < m_demodInstanceRegistrations.count(); i++) {
		m_demodInstanceRegistrations[i].m_gui->setWidgetName(QString("%1:%2").arg(m_demodInstanceRegistrations[i].m_demodName).arg(i));
	}
}
