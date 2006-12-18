#ifndef ROLELIST_H
#define ROLELIST_H

#include <QMap>
#include <QString>

#define ROLE_BOTOWNER 200
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
	void insert(const QString& key, const QString& role, const QString& affiliation);
	void update(const QString& key, const int value);
	int operator[](const QString& key);
	int get(const QString& key);
	static int calc(const QString& role, const QString& affiliation);
};

#endif

