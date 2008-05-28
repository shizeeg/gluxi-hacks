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
	static DataStorage* instance(const QString& configFile);
	bool connect();
	QString getString(const QString& name);
	std::string getStdString(const QString& name);
	int getInt(const QString& name);
	QSqlQuery prepareQuery(const QString& query);
	void setConfigFile(const QString& file);
private:
	static DataStorage* myInstance;
	DataStorage();
	DataStorage(const QString& configFile);
	~DataStorage();
	
	QSqlDatabase database;
	QSettings *settings;
	QString myType;
	QString myServer;
	int myPort;
	QString myUser;
	QString myPassword;
	QString myDatabase;
	QString configFile_;
	void loadSettings();
// 	void saveSettings();
};

#endif

