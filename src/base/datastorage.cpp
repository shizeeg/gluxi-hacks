#include "datastorage.h"

#include <QtDebug>
#include <QSettings>

#include <QSqlDatabase>
#include <QSqlError>

DataStorage* DataStorage::myInstance=0;

DataStorage::DataStorage()
{
	if (myInstance)
	{
		qWarning() << "DataStorage Instance already exists. Not updating myInstance()";
		return;
	}
	myInstance=this;
	settings=new QSettings("gluxi.cfg", QSettings::IniFormat);
	loadSettings();
}


DataStorage::~DataStorage()
{
	if (myInstance==this)
	{
		myInstance=0;
		delete settings;
	}
}

DataStorage* DataStorage::instance()
{
	if (!myInstance)
		new DataStorage();
	return myInstance;
}

void DataStorage::loadSettings()
{
	myType=settings->value("database/type").toString();
	myServer=settings->value("database/server").toString();
	myPort=settings->value("database/port").toInt();
	myUser=settings->value("database/user").toString();
	myPassword=settings->value("database/password").toString();
	myDatabase=settings->value("database/database").toString();
}

QString DataStorage::getString(const QString& name)
{
	return settings->value(name).toString();
}

std::string DataStorage::getStdString(const QString&name)
{
	return getString(name).toStdString();
}

int DataStorage::getInt(const QString& name)
{
	return settings->value(name).toInt();
}

bool DataStorage::connect()
{
	database = QSqlDatabase::addDatabase(myType);
	database.setHostName(myServer);
	database.setPort(myPort);
	database.setDatabaseName(myDatabase);
	database.setUserName(myUser);
	database.setPassword(myPassword);
	if (!database.open())
	{
		qFatal("%s\n",database.lastError().text().toLatin1().data());
		return false;
	}
	qDebug() << "DataStorage:: Connect to database success";
	return true;
}

QSqlQuery DataStorage::prepareQuery(const QString& query)
{
	if (!database.isOpen())
	{
		qWarning() << "Database connection lost";
		QSqlDatabase::removeDatabase(myType);
		connect();
	}
	QSqlQuery q;
	q.prepare(query);
	return q;
}
