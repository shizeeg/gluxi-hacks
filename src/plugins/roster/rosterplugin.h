/***************************************************************************
 *   Copyright (C) 2008 by Dmitry Nezhevenko                               *
 *   dion@inhex.net                                                        *
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
#ifndef ROSTERPLUGIN_H
#define ROSTERPLUGIN_H

#include "base/baseplugin.h"

#include <gloox/stanza.h>

class RosterStorage;

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class RosterPlugin : public BasePlugin
{
	Q_OBJECT
public:
	RosterPlugin(GluxiBot *parent = 0);
	~RosterPlugin();
	virtual QString name() const { return "Roster"; };
	virtual QString prefix() const { return "ROSTER"; };
	virtual StorageKey getStorage(gloox::Stanza*s);
private:
	RosterStorage* rosterStorage_;
	bool autoCreateStorage_;
};

#endif
