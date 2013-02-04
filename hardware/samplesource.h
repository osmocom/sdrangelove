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

#ifndef INCLUDE_SAMPLESOURCE_H
#define INCLUDE_SAMPLESOURCE_H

#include <QtGlobal>
#include "samplefifo.h"
#include "../util/message.h"

class SampleSourceGUI;
class MessageQueue;

class DSPCmdConfigureSource : public Message {
public:
	enum {
		Type = 13
	};

	DSPCmdConfigureSource() { }
	int type() const;
	const char* name() const;
	virtual int sourceType() const = 0;
};

class SampleSource {
public:
	SampleSource();

	virtual bool startInput(int device) = 0;
	virtual void stopInput() = 0;

	virtual const QString& getDeviceDescription() const = 0;
	virtual int getSampleRate() const = 0;
	virtual quint64 getCenterFrequency() const = 0;

	virtual SampleSourceGUI* createGUI(MessageQueue* msgQueue, QWidget* parent = NULL) const = 0;

	virtual void handleConfiguration(DSPCmdConfigureSource* cmd) = 0;

	SampleFifo* getSampleFifo() { return &m_sampleFifo; }

protected:
	SampleFifo m_sampleFifo;
};

#endif // INCLUDE_SAMPLESOURCE_H
