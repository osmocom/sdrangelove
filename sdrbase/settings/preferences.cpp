#include "settings/preferences.h"
#include "util/simpleserializer.h"

Preferences::Preferences()
{
	resetToDefaults();
}

void Preferences::resetToDefaults()
{
	m_audioOutput.clear();
	m_audioOutputRate = 44100;
}

QByteArray Preferences::serialize() const
{
	SimpleSerializer s(1);
	s.writeString(1, m_audioOutput);
	s.writeU32(2, m_audioOutputRate);
	return s.final();
}

bool Preferences::deserialize(const QByteArray& data)
{
	SimpleDeserializer d(data);

	if(!d.isValid()) {
		resetToDefaults();
		return false;
	}

	if(d.getVersion() == 1) {
		d.readString(1, &m_audioOutput);
		quint32 tmp;
		d.readU32(2, &tmp, 44100);
		m_audioOutputRate = tmp;
		return true;
	} else {
		resetToDefaults();
		return false;
	}
}
