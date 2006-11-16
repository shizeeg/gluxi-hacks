#ifndef DATASTORAGE_H
#define DATASTORAGE_H

#include <QString>

#include <string>

class QSettings;

class DataStorage
{
public:
	DataStorage();
	~DataStorage();
	static DataStorage* instance();
	bool connect();
	QString getString(const QString& name);
	std::string getStdString(const QString& name);
	int getInt(const QString& name);
private:
	static DataStorage* myInstance;
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

