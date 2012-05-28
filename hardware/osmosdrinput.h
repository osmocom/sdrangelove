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

#ifndef INCLUDE_OSMOSDRINPUT_H
#define INCLUDE_OSMOSDRINPUT_H

#include "samplesource.h"
#include <osmosdr.h>

class OsmoSDRThread;

class OsmoSDRInput : public SampleSource {
public:
	OsmoSDRInput(SampleFifo* sampleFifo);
	~OsmoSDRInput();

	bool startInput(int device, int rate);
	void stopInput();

	bool setCenterFrequency(qint64 freq);

private:
	osmosdr_dev_t* m_dev;
	OsmoSDRThread* m_osmoSDRThread;
};

#endif // INCLUDE_OSMOSDRINPUT_H
