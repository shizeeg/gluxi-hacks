#include "conference.h"
#include "jidstat.h"
#include "alist.h"
#include "base/common.h"
#include "base/datastorage.h"
#include "config/mucconfigurator.h"

#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>

Conference::Conference()
{
	qDebug() << "new Conference";
	myLazyLeave=false;
	myValidated=false;
}

Conference::Conference(const QString& name, const QString& nick, bool lazyLeave)
{
	myName=name;
	myNick=nick;
	myLazyLeave=lazyLeave;
	myValidated=false;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT id, name FROM conferences WHERE name = ?");
	query.addBindValue(myName);
	query.exec();
	if (query.next())
	{
		myId=query.value(0).toInt();
		if (myNick.isEmpty())
			myNick=query.value(2).toString();


		query.clear();
		query.prepare("UPDATE conferences SET nick = ?, joined = ?, online = true, autojoin = true WHERE id = ?");
		query.addBindValue(myNick);
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(myId);
		if (!query.exec())
		{
			qWarning() <<"Unable to update nickname in database";
		}

	}
	else
	{
		query.clear();
		query.prepare("INSERT INTO conferences ( name , nick , created , online , joined ) VALUES ( ? , ? , ? , ? , ? )");
		query.addBindValue(myName);
		query.addBindValue(myNick);
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(true);
		query.addBindValue(QDateTime::currentDateTime());
		if (!query.exec())
		{
			qWarning() << "Unable to insert new Conference";
			return;
		}
		query.clear();
		query.prepare("SELECT id FROM conferences WHERE name = ?");
		qDebug() << myName;
		query.addBindValue(myName);
		query.exec();
		if (!query.next())
		{
			qDebug() << QSqlDatabase::database().lastError().text();
		}
		myId=query.value(0).toInt();

		query.clear();
		query.prepare("UPDATE conferences SET joined = ? WHERE id = ?");
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(myId);
	}
	if (!lazyLeave)
		Nick::setAllOffline(this);
	myBan=new AList(this, "ban", ALIST_BAN);
	myKick=new AList(this, "kick", ALIST_KICK);
	myVisitor=new AList(this, "visitor", ALIST_VISITOR);
	myModerator=new AList(this, "moderator", ALIST_MODERATOR);
	myParticipant=new AList(this, "participant", ALIST_PARTICIPANT);
	myCommand=new AList(this, "command", ALIST_CMD);
	alistTraceList_=new QStringList();
}

Conference::~Conference()
{
	qDebug() << "~Conference";
	if (!myLazyLeave)
	{
		markOffline();
	}
	else
	{
		myNicks.lazyClear();
	}
	delete myBan;
	delete myKick;
	delete myVisitor;
	delete myModerator;
	delete myParticipant;
	delete myCommand;
	delete configurator_;
	delete alistTraceList_;
}

void Conference::markOffline()
{
	QSqlQuery query=DataStorage::instance()
				->prepareQuery("UPDATE conferences SET online = false WHERE id = ?");
	query.addBindValue(myId);
	query.exec();
}

QStringList Conference::autoJoinList()
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT name, nick FROM conferences WHERE autojoin=true");
	query.exec();
	QStringList result;
	while (query.next())
	{
		QString conf=query.value(0).toString();
		QString resource=query.value(1).toString();
		if (resource.isEmpty())
			result.append(conf);
		else
			result.append(conf+"/"+resource);
	}
	return result;
}

void Conference::setAutoJoin(bool b)
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conferences SET autojoin=? WHERE id=?");
	query.addBindValue(b);
	query.addBindValue(myId);
	query.exec();
}

void Conference::removeExpired()
{
	myKick->removeExpired();
	myVisitor->removeExpired();
	myModerator->removeExpired();
}

QString Conference::seen(const QString&n)
{
	Nick* nick=myNicks.byName(n);
//	int cnt=0;
	if (nick)
	{
		return QString("\"%1\" is already in room (Joined %2 ago)").arg(n)
			.arg(secsToString(nick->joined().secsTo(QDateTime::currentDateTime())));
	}
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT jid FROM conference_nicks WHERE conference_id=? AND nick=? ORDER BY lastaction DESC LIMIT 1");
	query.addBindValue(myId);
	query.addBindValue(n);
	if (query.exec() && query.next())
	{
		int jid=query.value(0).toInt();
		query.prepare("SELECT online, nick, lastaction, joined, jid FROM conference_nicks WHERE "
			"conference_id=? AND jid=? ORDER BY lastaction DESC LIMIT 1");
		query.addBindValue(myId);
		query.addBindValue(jid);
		if (query.exec() && query.next())
		{
			bool online=query.value(0).toBool();
			QString newNick=query.value(1).toString();
			QDateTime lastAction=query.value(2).toDateTime();
			QDateTime joinedTime=query.value(3).toDateTime();
			int jidId = query.value(4).toInt();
			if (online)
				return QString("%1 is here with nick \"%2\" (Joined %3 ago)").arg(n).arg(newNick)
					.arg(secsToString(joinedTime.secsTo(QDateTime::currentDateTime())));
			QString secs=secsToString(lastAction.secsTo(QDateTime::currentDateTime()));

			QString token("was here");
			QString reason;
			if (jidId > 0)
			{
				// Query for some stat information;
				JidStat *stat = JidStat::queryReadOnly(jidId);
				if (stat)
				{
					JidStat::StatAction act = stat->lastAction();
					delete stat;
					stat = NULL;

					switch (act.type)
					{
					case JidStat::ActionKick:
						token = "was kicked";
						break;
					case JidStat::ActionBan:
						token = "was banned";
						break;
					}
					reason = act.reason;
				}
			}
			QString reply;
			if (newNick == n)
				reply = QString("%1 %2 %3 ago").arg(n, token, secs);
			else
				reply = QString("%1 %2 %3 ago with nick \"%4\"").arg(n, token, secs, newNick);
			if (!reason.isEmpty())
				reply += QString(" (%1)").arg(reason);
			return reply;
		}
	}
	return QString("I never see \"%1\" here").arg(n);
}

QString Conference::clientStat()
{
	QSqlQuery query;
	query.prepare("SELECT resource, COUNT(id) AS count FROM conference_jids WHERE conference_id=? "
		"GROUP BY resource ORDER BY count DESC LIMIT 20");
	query.addBindValue(myId);
	int idx=0;
	if (query.exec())
	{
		QString res;
		while (query.next())
			res+=QString("\n%1) %2: %3").arg(++idx).arg(query.value(0).toString())
				.arg(query.value(1).toInt());
		return QString("Client stats: %1").arg(res);
	}
	else
		return QString::null;
}

void Conference::setNick(const QString& nick)
{
	myNick=nick;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conferences SET nick=? WHERE id=?");
	query.addBindValue(nick);
	query.addBindValue(myId);
	qDebug() << "Setting nick: " << myId << ", " << nick;
	if (!query.exec())
	{
		qDebug() << query.lastQuery() << ": " << query.lastError().text();
	}
}

void Conference::setLazyLeave(bool value)
{
	myLazyLeave=value;
}

void Conference::loadOnlineNicks()
{
	QSqlQuery query=DataStorage::instance()
			->prepareQuery("SELECT id FROM conference_nicks where conference_id=? AND online=? ORDER by joined");
	query.addBindValue(myId);
	query.addBindValue(true);
	if (!query.exec())
	{
		qDebug() << "Conference: Unable to load online nicks";
	}
	while (query.next())
	{
		Nick* nick=new Nick(this, query.value(0).toInt());
		nick->setValidateRequired(true);
		myNicks.append(nick);
	}
}

void Conference::cleanNonValidNicks()
{
	QList<Nick*> removeList;
	for (NickList::iterator it=myNicks.begin(); it!=myNicks.end(); ++it)
	{
		Nick* nick=*it;
		if (nick->validateRequired())
			removeList.append(nick);
	}
	foreach(Nick* nick , removeList)
	{
		nick->setValidateRequired(false);
		myNicks.remove(nick);
	}
	removeList.clear();
}

QStringList Conference::autoLeaveList()
{
	QStringList res;
	int deltaTime=DataStorage::instance()->getInt("muc/leave_minjids_interval");
	int minJids=DataStorage::instance()->getInt("muc/leave_minjids");
	QSqlQuery query=DataStorage::instance()
				->prepareQuery("select conferences.name,"
				" (select count(distinct jid) from conference_nicks"
				" where conference_nicks.conference_id=conferences.id and"
				" lastaction > ?) as cnt"
				" from conferences where autojoin=true and online=true and autoleave=true order by cnt");
	QDateTime currentDate=QDateTime::currentDateTime();
	currentDate=currentDate.addSecs(-deltaTime);
	query.addBindValue(currentDate);
	if (!query.exec())
	{
		qDebug() << query.lastError().text();
		return res;
	}
	while (query.next())
	{
		QString name=query.value(0).toString();
		int cnt=query.value(1).toInt();
		if (cnt>=minJids)
			break;
		res.append(QString("%1 %2").arg(cnt).arg(name));
	}
	return res;
}

void Conference::disableAutoJoin(const QString& conference)
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conferences SET autojoin=? WHERE name=?");
	query.addBindValue(false);
	query.addBindValue(conference);
	query.exec();
}
