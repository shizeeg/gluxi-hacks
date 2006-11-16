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
	QSqlDatabase db = QSqlDatabase::addDatabase(myType);
	db.setHostName(myServer);
	db.setPort(myPort);
	db.setDatabaseName(myDatabase);
	db.setUserName(myUser);
	db.setPassword(myPassword);
	if (!db.open())
	{
		qFatal("%s\n",db.lastError().text().toLatin1().data());
		return false;
	}
	qDebug() << "DataStorage:: Connect to database success";
	return true;
}

