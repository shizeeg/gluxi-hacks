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
#ifndef CLEANASYNCREQUEST_H
#define CLEANASYNCREQUEST_H

#include "base/asyncrequest.h"

class QTimer;

class CleanAsyncRequest: public AsyncRequest
{
        Q_OBJECT
public:
	CleanAsyncRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	virtual ~CleanAsyncRequest();
	virtual void exec();
	gloox::Stanza* stanza() { return stanza_; };
	int count() const { return count_; }
	int step()  const { return step_; }
	int sent()  const { return sent_; }

	void setCount(const int count) { count_ = ( count > 0 && count < 50 ) ? count:20; }
	void setStep(const int step) { step_ = ( step >= 1500 && step < 4000 ) ? step:1500; }

private:
	QString myDest_;
	QTimer* timer_;
	gloox::Stanza* stanza_;
	int step_;
	int count_;
	int sent_;

private slots:
	void sltTimerTimeout();
};
#endif /* CLEANASYNCREQUEST_H */

