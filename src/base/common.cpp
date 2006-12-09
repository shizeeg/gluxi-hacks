#include "common.h"

#include <QStringList>
#include <QList>
#include <QRegExp>

#ifndef Q_WS_WIN
#include <sys/utsname.h>
#endif

QString secsToString(int secs)
{
	QStringList labels;
	labels << "sec." << "min." << "h." << "day(s)";

	QList<int> vals;

	vals << secs%60;	secs/=60;
	vals << secs%60;	secs/=60;
	vals << secs%24;	secs/=24;
	vals << secs;
	QString res;
	bool canSkip=true;
	int cnt=labels.count();
	for (int i=cnt-1; i>=0; i--)
	{
		if (canSkip && vals[i]==0 && i!=0)
			continue;
		canSkip=false;
		if (!res.isEmpty()) res+=" ";
		res+=QString::number(vals[i])+" "+labels[i];
	}
	return res;
}

QString version()
{
#ifdef Q_WS_WIN
	return "M$ Windows";
#endif
#ifdef Q_WS_MAC
	return "Mac OS X";
#endif
	struct utsname ver;
	if (uname(&ver)!=0)
		return "Unknown";
	QString release=QString(ver.release).section('-',0,0);
	QString res=QString("%1 %2 %3 %4").arg(ver.sysname).arg(release)
		.arg(ver.version).arg(ver.machine);
	return res;
}

bool isSafeArg(const QString& arg)
{
	QRegExp exp("^[0-9A-Za-z_\\-\\.]*$");
	exp.setMinimal(false);
	return exp.exactMatch(arg);
}

