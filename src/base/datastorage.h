#ifndef DATASTORAGE_H
#define DATASTORAGE_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <string>

class QSettings;

class DataStorage
{
public:
	static DataStorage* instance();
	bool connect();
	QString getString(const QString& name);
	std::string getStdString(const QString& name);
	int getInt(const QString& name);
	QSqlQuery prepareQuery(const QString& query);
private:
	static DataStorage* myInstance;
	DataStorage();
	~DataStorage();
	
	QSqlDatabase database;
	QSettings *settings;
	QString myType;
	QString myServer;
	int myPort;
	QString myUser;
	QString myPassword;
	QString myDatabase;
	void loadSettings();
// 	void saveSettings();
};

#endif

