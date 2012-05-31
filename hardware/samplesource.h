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

class SampleFifo;

class SampleSource {
public:
	SampleSource(SampleFifo* sampleFifo);

	virtual bool startInput(int device, int rate) = 0;
	virtual void stopInput() = 0;

	virtual bool setCenterFrequency(qint64 freq) = 0;
	virtual bool setIQSwap(bool sw) = 0;
	virtual bool setDecimation(int dec) = 0;

protected:
	SampleFifo* m_sampleFifo;
};

#endif // INCLUDE_SAMPLESOURCE_H
