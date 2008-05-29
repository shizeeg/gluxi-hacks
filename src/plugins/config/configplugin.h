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
#ifndef CONFIGPLUGIN_H
#define CONFIGPLUGIN_H

#include "base/baseplugin.h"
#include "base/config/configfield.h"

#include <gloox/stanza.h>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class ConfigPlugin : public BasePlugin
{
	Q_OBJECT
public:
	ConfigPlugin(GluxiBot *parent = 0);
	~ConfigPlugin();
	virtual QString name() const { return "Config"; };
	virtual QString prefix() const { return "CONFIG"; };
	virtual bool parseMessage(gloox::Stanza* );
	virtual bool canHandleIq(gloox::Stanza*);
	virtual bool onIq(gloox::Stanza*);
private:
	gloox::Tag* createCommandTag(const QString& nodePart, const QString& name, const QString& jid);
	gloox::Tag* createFieldTag(const ConfigField& field);
	QString fieldTypeToString(ConfigField::FieldType fieldType);
};

#endif
