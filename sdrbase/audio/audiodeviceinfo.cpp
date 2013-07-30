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

#include "audio/audiodeviceinfo.h"

AudioDeviceInfo::AudioDeviceInfo()
{
//	const PaDeviceInfo *deviceInfo;
//	const PaHostApiInfo *apiInfo;
//	PaError err;
//	int numDevices;
//	int i;
//
//	if((numDevices = Pa_GetDeviceCount()) < 0) {
//		err = numDevices;
//		goto failed;
//	}
//
//	m_devices.clear();
//
//	for(i = 0; i < numDevices; i++) {
//		deviceInfo = Pa_GetDeviceInfo(i);
//		if(deviceInfo->maxOutputChannels >= 2) {
//			apiInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
//			m_devices.append(Device(
//				QString::fromLatin1(deviceInfo->name),
//				QString::fromLatin1(apiInfo->name),
//				i));
//		}
//	}
//	qDebug("Audio initialisation: %d devices found", m_devices.count());
//	return;
//
//failed:
//	if(err != paNoError)
//		qCritical("Audio initialisation failed: %s (%d)", Pa_GetErrorText(err), err);
}

int AudioDeviceInfo::match(const QString& api, const QString device) const
{
	if(m_devices.count() <= 0)
		return -1;

	// default - use first device
	if(api.isEmpty())
		return 0;

	if(device.isEmpty()) {
		// only match API
		for(int i = 0; i < m_devices.count(); ++i) {
			if(m_devices[i].api == api)
				return i;
		}
	} else {
		// match API and device name
		for(int i = 0; i < m_devices.count(); ++i) {
			if((m_devices[i].api == api) && (m_devices[i].name == device))
				return i;
		}
		// not found - only match API
		for(int i = 0; i < m_devices.count(); ++i) {
			if(m_devices[i].api == api)
				return i;
		}
	}

	// nothing found - fall back to default
	return 0;
}
