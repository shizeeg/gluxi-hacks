#include "common.h"

#include <QStringList>
#include <QList>
#include <QRegExp>
#include <QtDebug>

#ifndef Q_WS_WIN
#include <sys/utsname.h>
#endif

QString secsToString(int secs)
{
	QStringList labels;
	labels << "sec." << "min." << "h." << "day(s)";

	QList<int> vals;

	vals << secs%60;
	secs/=60;
	vals << secs%60;
	secs/=60;
	vals << secs%24;
	secs/=24;
	vals << secs;
	QString res;
	bool canSkip=true;
	int cnt=labels.count();
	for (int i=cnt-1; i>=0; i--)
	{
		if (canSkip && vals[i]==0 && i!=0)
			continue;
		canSkip=false;
		if (!res.isEmpty())
			res+=" ";
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
	QString release=QString(ver.release).section('-', 0, 0);
	QString res=QString("%1 %2 %3 %4").arg(ver.sysname).arg(release)
	.arg(ver.version).arg(ver.machine);
	return res;
}

bool isSafeArg(const QString& arg)
{
	QRegExp exp("^[0-9A-Za-z_\\-\\.\\:\\/]*$");
	exp.setMinimal(false);
	return exp.exactMatch(arg);
}

QString getValue(const QString&s, const QString&exp, bool last)
{
	QRegExp expr(exp);
	expr.setMinimal(TRUE);
	expr.setCaseSensitivity(Qt::CaseInsensitive);
	if (!last)
	{
		if (expr.indexIn(s)<0)
			return QString::null;
		}
		else
		{
			if (expr.lastIndexIn(s)<0)
			return QString::null;
		}
		QStringList list=expr.capturedTexts();
		if (list.count()!=2)
		return QString::null;
		return list.value(1);
	}

QString tagReplacement(const QString& str)
{
	QString tag=str.section(' ',0,0).toLower();
	if (tag=="br" || tag=="p")
		return "\n";
	return " ";
}

QString removeExtraSpaces(const QString& str)
{
	QString res;
	int l=str.length();
	bool wasEOL=false;
	bool wasSpace=false;
	for (int i=0; i<l; ++i)
	{
		QChar ch=str[i];
		if (ch=='\n')
		{
			if (wasEOL)
				continue;
			wasEOL=true;
		}
		else if (ch=='\n')
		{
			if (wasSpace)
				continue;
			wasSpace=true;
		}
		else
		{
			wasSpace=false;
			wasEOL=false;
		}
		res+=ch;
	}
	return res;
}

QString removeHtml(const QString& s)
{
	QString res=s;
	QRegExp exp("<([^>]*)>");
	exp.setMinimal(true);
	while (1)
	{
		int ps=exp.indexIn(res);
		if (ps<0)
			break;
		QStringList captured=exp.capturedTexts();
		QString repl;
		if (captured.size()>1)
		{
			QString tag=captured[1].toLower();
			repl=tagReplacement(tag);
		}
		res.remove(ps, exp.matchedLength());
		if (!repl.isEmpty())
		{
			res.insert(ps, repl);
		}
	}
	res.replace("&lt;", "<");
	res.replace("&amp;", "&");
	res.replace("&gt;", ">");
	res.replace("&quot;", "\"");
	res.replace("&#39;", "'");
	res.replace("&raquo;", ">>");
	res.replace("&ldquo;", "\"");
	res.replace("&rdquo;", "\"");
	res.replace("&rsquo;", "'");
	res.replace("&nbsp;", " ");
	return removeExtraSpaces(res);
}

