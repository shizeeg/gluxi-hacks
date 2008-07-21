#include "datastorage.h"

#include "dbversion.h"

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
	settings=0;
	setConfigFile("gluxi.cfg");
}

DataStorage::DataStorage(const QString& configFile)
{
	if (myInstance)
	{
		qWarning() << "DataStorage Instance already exists. Not updating myInstance()";
		return;
	}
	myInstance=this;
	settings=0;
	setConfigFile(configFile);
}

void DataStorage::setConfigFile(const QString& configFile)
{
	configFile_=configFile;
	if (settings)
		delete settings;
	settings=new QSettings(configFile_, QSettings::IniFormat);
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

DataStorage* DataStorage::instance(const QString& configFile)
{
	if (myInstance)
	{
		qDebug() << "Not using config file " << configFile <<" since DataStorage is already configured";
		return myInstance;
	}
	return new DataStorage(configFile);
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
	database = QSqlDatabase::addDatabase(myType, "gluxi");
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
	return checkDbVersion();
}

void DataStorage::disconnect()
{
	qDebug() << "Disconnecting from database";
	database.close();
	QSqlDatabase::removeDatabase("gluxi");
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

bool DataStorage::checkDbVersion()
{
	QSqlQuery query=prepareQuery("SELECT value FROM version WHERE name=? LIMIT 1");
	query.addBindValue("dbversion");
	if (!query.exec() || !query.next()) {
		qDebug() << "Unable to query for database version: " << query.lastError().text();
		return false;
	}
	int version=query.value(0).toInt();
	if (version!=GLUXI_DB_VERSION)
	{
		qDebug() << "Database version mismatch: this version of GluxiBot require "
			"version" << GLUXI_DB_VERSION << " however your database version is" << version;
		return false;
	}
	return true;
}
