
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
#include "cleanasyncrequest.h"
#include "base/baseplugin.h"

#include <QTimer>

CleanAsyncRequest::CleanAsyncRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	sent_   = 0;
	myDest_ = dest;
	setCount( myDest_.toInt() );
	setStep( 1500 );

	stanza_ = new gloox::Stanza(from);
	timer_  = new QTimer(this);
	connect(timer_, SIGNAL(timeout()), SLOT(sltTimerTimeout()));
}

CleanAsyncRequest::~CleanAsyncRequest()
{
	delete timer_; 
	delete stanza_;
}

void CleanAsyncRequest::exec()
{
	setCount( myDest_.toInt() );
	plugin()->reply(stanza(), QString("Clean started, please wait %1 sec...")
			.arg( step() * count() / 1000 ), true);
	timer_->stop();
	timer_->start( step() );
}

void CleanAsyncRequest::sltTimerTimeout()
{
	stanza()->addAttribute("type", "groupchat");  
	stanza()->finalize();
	plugin()->reply(stanza(), "", false, false);
	count_ --; sent_ ++;

	if( count() <= 0 ) {
		timer_->stop();
		plugin()->reply(stanza(), QString("Clean finished: %1 messages sent.").arg(sent()), true);
		deleteLater();
	}
}

