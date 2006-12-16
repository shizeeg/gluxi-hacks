#include "rolelist.h"

RoleList::RoleList()
{
}

RoleList::~RoleList()
{
}

void RoleList::insert(const QString& key, const int value)
{
	QMap<QString, int>::insert(key,value);
}

void RoleList::insert(const QString& key, const QString& from)
{
	int value=(*this)[from];
	QMap<QString, int>::insert(key,value);
}

int RoleList::operator[](const QString&key)
{
	return QMap<QString, int>::value(key,0);
}

