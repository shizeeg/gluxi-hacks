/***************************************************************************
 *   Copyright (C) 2009 by Dmitry Nezhevenko                               *
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
#ifndef ACTIONTYPE_H_
#define ACTIONTYPE_H_

enum ActionType
{
	ActionNone = 0,
	ActionJoin = 1,
	ActionLeave = 2,
	ActionPresence = 3,
	ActionNickChange = 4,

	ActionVisitor = 5,
	ActionParticipant = 6,
	ActionModerator = 7,

	ActionNoAffiliation = 8,
	ActionMember = 9,
	ActionAdministrator = 10,
	ActionOwner = 11,

	ActionBan = 12,
	ActionKick = 13,

	ActionMessage = 14,
	ActionSubject = 15,
	ActionExpandedAlias = 16,

	ActionAListCommand = 17,
	ActionAListBan = 18,
	ActionAListKick = 19,
	ActionAListVisitor = 20,
	ActionAListParticipant = 21,
	ActionAListModerator = 22
};

#endif /* ACTIONTYPE_H_ */
