#include "conferencelist.h"

#include <QtDebug>
#include <QSqlQuery>

ConferenceList::ConferenceList()
{
	QSqlQuery query;
	query.clear();
	query.prepare("UPDATE conferences SET online = 0");
	query.exec();
}


ConferenceList::~ConferenceList()
{
	clear();
}

void ConferenceList::clear()
{
	while (count())
		delete takeAt(0);
}

Conference* ConferenceList::byName(const QString& name)
{
	int cnt=count();
	Conference *conf;
	for (int i=0; i<cnt; i++)
	{
		conf=value(i);
		if (conf->name()==name)
			return conf;
	}
	return 0;
}

void ConferenceList::remove(Conference* conf)
{
	qDebug() << "[CONFERENCELIST] Remove " << conf->name() << " Was: " << count();
	removeAll(conf);
	qDebug() << " Now " << count();
	delete conf;
}
