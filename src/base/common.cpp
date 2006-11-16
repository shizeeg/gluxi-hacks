#include "common.h"

#include <QStringList>
#include <QList>

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
		if (canSkip && vals[i]==0)
			continue;
		canSkip=false;
		if (!res.isEmpty()) res+=" ";
		res+=QString::number(vals[i])+" "+labels[i];
	}
	return res;
}
