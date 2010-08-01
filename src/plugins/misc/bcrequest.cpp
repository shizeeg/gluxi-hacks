/* -*- mode: C++; tab-width: 4; -*- */
/***************************************************************************
 *   Copyright (C) 2010 by Sidgyck                                         *
 *   sidgyck@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "bcrequest.h"
#include "base/baseplugin.h"
#include "base/messageparser.h"

#include <QProcess>
#include <QtDebug>

BcRequest::BcRequest(BasePlugin *plugin, gloox::Stanza *from, MessageParser& parser)
	: AsyncRequest(-1, plugin, from, 300)
{
	myDest.append(FAC).append(parser.joinBody());
}

BcRequest::~BcRequest()
{
	proc_->kill();
	delete proc_;
}

void BcRequest::exec()
{
	if( myDest.isEmpty() ) {
		plugin()->reply(stanza(), QString("!misc bc <expression>") );
		deleteLater();
		return;
	}

	for (int i = 0; i < myDest.length(); i++) {
		if( myDest.at(i) > 127 || myDest.at(i) < 30 )
			myDest[i] = QChar(' ');
	}

	//	if( !myDest.contains(QRegExp("[0-9]")) ) {
	//	plugin()->reply(stanza(), QString("[ERROR]: unsupported expression."));
	//	deleteLater();
	//	return;
	//}

	proc_ = new QProcess(this);
	connect(proc_, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onProcessFinished()));
	connect(proc_, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)));

	QString cmd = "bc";
	QStringList args; args << "-l";

	proc_->start(cmd, args);
	
	if (!proc_->waitForStarted()) {
		qDebug() << "bc: unable to start.";
		deleteLater();
		return;
	}

	myDest.replace('!', ' ');
	myDest.append("\n");
	
	proc_->write(myDest.toUtf8());
	proc_->closeWriteChannel();
	
	if (!proc_->waitForFinished()) {
		proc_->kill();
		plugin()->reply(stanza(), "bc: timeout.");
		deleteLater();
		return;
	}
}

void BcRequest::onProcessFinished()
{
	qDebug() << "BC: Process finished";
	QByteArray out;
	if (proc_->bytesAvailable() > 1024 ) {
		plugin()->reply(stanza(), "sorry, result is too long");
		deleteLater();
		return;
	}
	out.append( proc_->readAllStandardOutput() );
	out.append( proc_->readAllStandardError() );
	QString msg(out.trimmed());
	int i = msg.length();

	if (i <= 0 ) {
		deleteLater();
		return;
	}
	if (msg.indexOf('.') != -1) {
		while (i--) {
			if (msg.at(i) != '0') break;
		}
		if (msg.at(i) != '.') i++;
	}

	plugin()->reply( stanza(), QString(msg.left(i).toUtf8()).trimmed());
	deleteLater();
}

void BcRequest::onStateChanged(QProcess::ProcessState newState)
{
	qDebug() << "STCH: " << newState;	
}

