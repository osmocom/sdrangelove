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

#include <QApplication>
#include <QTextCodec>
#include <QMessageBox>
#include "mainwindow.h"
#include "portaudio.h"

static void initPortAudio()
{
	PaError err;

	if((err = Pa_Initialize()) != paNoError) {
		qCritical("PortAudio: could not initialise: %s (%d)", Pa_GetErrorText(err), err);
		QString error = QObject::tr("PortAudio could not be initialised: %1 (%2)").arg(Pa_GetErrorText(err)).arg(err);
		QMessageBox::critical(NULL, "PortAudio failure", error);
	}
}

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	initPortAudio();

	MainWindow w;
	w.show();

	return a.exec();
}
