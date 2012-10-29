///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <QMessageBox>
#include "soundcardinfo.h"
#include "portaudio.h"

SoundcardInfo::SoundcardInfo()
{
	const PaDeviceInfo *deviceInfo;
	const PaHostApiInfo *apiInfo;
	PaError err;
	int numDevices;
	int i;
	QString name;

	if((numDevices = Pa_GetDeviceCount()) < 0) {
		err = numDevices;
		goto failed;
	}

	m_devices.clear();
	qDebug("Audio initialisation: %d devices found", numDevices);

	for(i = 0; i < numDevices; i++) {
		deviceInfo = Pa_GetDeviceInfo(i);
		apiInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);

		name = QString("%1 (record %2, playblack %3, API %4)").
			arg(QString::fromLatin1(deviceInfo->name)).
			arg(deviceInfo->maxInputChannels).
			arg(deviceInfo->maxOutputChannels).
			arg(QString::fromLatin1(apiInfo->name));

		m_devices.append(name);

		qDebug("  Device #%d: %s", i, qPrintable(name));
	}
	return;

failed:
	if(err != paNoError)
		qCritical("Audio initialisation failed: %s (%d)", Pa_GetErrorText(err), err);
}
#if 0
int SoundcardInfo::getInputDevice()
{
	// find device
	for(i = 0; i < m_soundCardInfo->getDeviceCount(); i++) {
		if(m_soundCardInfo->getDevice(i) == dev) {
			device = i;
			break;
		}
	}

	// if no device is found or configured, find first DirectSound input device
	if(device < 0) {
		int numHostApis = Pa_GetHostApiCount();
		const PaHostApiInfo *apiInfo;
		int i;
		for(i = 0; i < numHostApis; i++) {
			apiInfo = Pa_GetHostApiInfo(i);
			if(strcmp(apiInfo->name, defaultDev) == 0) {
				device = apiInfo->defaultInputDevice;
				if(device >= 0)
					break;
			}
		}
		if(device < 0)
			device = Pa_GetDefaultInputDevice();
	}
}
#endif
