#ifndef ROLELIST_H
#define ROLELIST_H

#include <QMap>
#include <QString>

#define ROLE_BOTOWNER 100
// MUC Roles
#define ROLE_OWNER 100
#define ROLE_ADMIN 50
#define ROLE_MODERATOR 40
#define ROLE_MEMBER 30
#define ROLE_PARTICIPANT 20

class RoleList: public QMap<QString, int>
{
public:
	RoleList();
	~RoleList();
	void insert(const QString& key, const int value);
	void insert(const QString& key, const QString& from);
	int operator[](const QString&key);
};

#endif

