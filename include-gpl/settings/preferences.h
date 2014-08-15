#ifndef INCLUDE_PREFERENCES_H
#define INCLUDE_PREFERENCES_H

#include <QString>

class Preferences {
public:
	Preferences();

	void resetToDefaults();
	QByteArray serialize() const;
	bool deserialize(const QByteArray& data);

	void setAudioOutput(const QString& value) { m_audioOutput = value; }
	const QString& getAudioOutput() const { return m_audioOutput; }

	void setAudioOutputRate(quint32 value) { m_audioOutputRate = value; }
	uint getAudioOutputRate() const { return m_audioOutputRate; }

protected:
	QString m_audioOutput;
	uint m_audioOutputRate;
};

#endif // INCLUDE_PREFERENCES_H
