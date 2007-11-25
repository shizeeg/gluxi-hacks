#include "rolelist.h"

#include <QtDebug>

RoleList::RoleList()
{
}

RoleList::~RoleList()
{
}

void RoleList::insert(const QString& key, const int value)
{
	if (keys().indexOf(key)>=0)
	{
		qDebug() << "RoleList: Key already exists";
		remove(key);
	}

	if (value>=ROLE_MEMBER)
		QMap<QString, int>::insert(key,value);
	qDebug() << "RoleList: NEW " << key << value;
}

void RoleList::insert(const QString& key, const QString& from)
{
	if (from.isEmpty())
		return;
	int value=(*this)[from];
	QMap<QString, int>::insert(key,value);
}

void RoleList::insert(const QString& key, const QString& role, const QString& affiliation)
{
	insert(key,calc(role,affiliation));
}

void RoleList::update(const QString& key, int value)
{
	qDebug() << "RoleList::update: " << key << value;
	if (get(key)>=value)
		return;
	remove(key);
	insert(key,value);
}

int RoleList::calc(const QString& role, const QString& affiliation)
{

	QString r=role.toUpper();
	QString a=affiliation.toUpper();
	if (a.startsWith("OWNER"))
		return(ROLE_OWNER);
	if (a.startsWith("ADMIN"))
		return(ROLE_ADMIN);
	if (r.startsWith("MODER"))
		return(ROLE_MODERATOR);
	if (a.startsWith("MEMBER"))
		return(ROLE_MEMBER);
	if (r.startsWith("PARTICIPANT"))
		return(ROLE_PARTICIPANT);
	return 0;
}

int RoleList::operator[](const QString&key)
{
	return get(key);
}

int RoleList::get(const QString& key)
{
	return QMap<QString, int>::value(key,0);
}

