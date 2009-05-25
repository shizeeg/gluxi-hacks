#include "common.h"

#include <QStringList>
#include <QList>
#include <QRegExp>
#include <QtDebug>
#include <QUrl>
#include <QTextCodec>
#include <QByteArray>

#ifndef Q_WS_WIN
#include <sys/utsname.h>
#endif

static const int TAB_SIZE=1;

static QMap<QString, QString> htmlMap;

void populateHtmlMap();
QString replaceHtmlToken(const QString& token);

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
	if (htmlMap.isEmpty())
		populateHtmlMap();

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
	QRegExp patExp("&([^;]+);");
	patExp.setMinimal(true);
	int ps=0;
	while ((ps=patExp.indexIn(res,ps))!=-1)
	{
		QStringList captured=patExp.capturedTexts();
		QString v;
		if (captured.count()==2)
		{
			QString sub=captured[1];
			v=replaceHtmlToken(sub);
		}
		res.remove(ps,patExp.matchedLength());
		if (!v.isEmpty())
		{
			res.insert(ps,v);
			ps+=v.length();
		}
	}

	return removeExtraSpaces(res);
}

QString replaceHtmlToken(const QString& token)
{
	if (htmlMap.contains(token))
		return htmlMap[token];
	if (token.startsWith("#"))
	{
		QString tInt(token);
		tInt.remove(0,1);
		bool ok=true;
		int code=tInt.toInt(&ok);
		if (ok)
			return QChar(code);
		else
			return " ";
	}
	return QString();
}

bool isBareJidValid(const QString& jid)
{
	QString user=jid.section('@',0,-2);
	QString server=jid.section('@',-1,-1);
	if (!isServerValid(server))
		return false;
	return (user.indexOf(' ')<0);
}

bool isServerValid(const QString& server) {
	QRegExp exp("[A-Za-z0-9\\-_\\.]+\\.[A-Za-z0-9\\-_\\.]+");
	return exp.exactMatch(server);
}

QString urlEncode(const QString& s, const QString& encoding)
{
	QString enc=encoding.toLower();
	if (enc.isEmpty() || enc=="utf8" || enc=="utf-8")
		return QString(QUrl::toPercentEncoding(s));

	QString dst;
	QTextCodec* codec=QTextCodec::codecForName(encoding.toAscii());
	QByteArray arr;
	if (codec)
		arr=codec->fromUnicode(s);
	else
		arr=s.toUtf8();
	int l=arr.length();
	for (int i=0; i<l; ++i)
	{
		char ch=arr.at(i);
		if ((ch >= 'A' && ch<='A') || (ch>='a' && ch<='a') || (ch>='0' && ch<='9')
				|| ch=='~' || ch=='~' || ch=='.' || ch=='-')
		{
			dst.append(ch);
		}
		else
		{
			QString num;
			num.setNum((unsigned char)ch,16);
			num=num.rightJustified(2,QChar('0'));
			dst.append("%"+num);
		}
	}
	return dst;
}

void populateHtmlMap()
{
	htmlMap.insert("nbsp"," ");
	htmlMap.insert("iexcl",  QChar(161));
	htmlMap.insert("cent",  QChar(162));
	htmlMap.insert("pound",  QChar(163));
	htmlMap.insert("curren",  QChar(164));
	htmlMap.insert("yen",  QChar(165));
	htmlMap.insert("brvbar",  QChar(166));
	htmlMap.insert("sect",  QChar(167));
	htmlMap.insert("uml",  QChar(168));
	htmlMap.insert("copy",  QChar(169));
	htmlMap.insert("ordf",  QChar(170));
	htmlMap.insert("laquo",  QChar(171));
	htmlMap.insert("raquo",  QChar(187));
	htmlMap.insert("not",  QChar(172));
	htmlMap.insert("shy",  QChar(173));
	htmlMap.insert("reg",  QChar(174));
	htmlMap.insert("macr",  QChar(175));
	htmlMap.insert("deg",  QChar(176));
	htmlMap.insert("plusmn",  QChar(177));
	htmlMap.insert("sup2",  QChar(178));
	htmlMap.insert("sup3",  QChar(179));
	htmlMap.insert("acute",  QChar(180));
	htmlMap.insert("micro",  QChar(181));
	htmlMap.insert("para",  QChar(182));
	htmlMap.insert("middot",  QChar(183));
	htmlMap.insert("cedil",  QChar(184));
	htmlMap.insert("sup1",  QChar(185));
	htmlMap.insert("ordm",  QChar(186));
	htmlMap.insert("frac14",  QChar(188));
	htmlMap.insert("frac12",  QChar(189));
	htmlMap.insert("frac34",  QChar(190));
	htmlMap.insert("iquest",  QChar(191));
	htmlMap.insert("Agrave",  QChar(192));
	htmlMap.insert("Aacute",  QChar(193));
	htmlMap.insert("Acirc",  QChar(194));
	htmlMap.insert("Atilde",  QChar(195));
	htmlMap.insert("Auml",  QChar(196));
	htmlMap.insert("Aring",  QChar(197));
	htmlMap.insert("AElig",  QChar(198));
	htmlMap.insert("Ccedil",  QChar(199));
	htmlMap.insert("Egrave",  QChar(200));
	htmlMap.insert("Eacute",  QChar(201));
	htmlMap.insert("Ecirc",  QChar(202));
	htmlMap.insert("Euml",  QChar(203));
	htmlMap.insert("Igrave",  QChar(204));
	htmlMap.insert("Iacute",  QChar(205));
	htmlMap.insert("Icirc",  QChar(206));
	htmlMap.insert("Iuml",  QChar(207));
	htmlMap.insert("ETH",  QChar(208));
	htmlMap.insert("Ntilde",  QChar(209));
	htmlMap.insert("Ograve",  QChar(210));
	htmlMap.insert("Oacute",  QChar(211));
	htmlMap.insert("Ocirc",  QChar(212));
	htmlMap.insert("Otilde",  QChar(213));
	htmlMap.insert("Ouml",  QChar(214));
	htmlMap.insert("times",  QChar(215));
	htmlMap.insert("Oslash",  QChar(216));
	htmlMap.insert("Ugrave",  QChar(217));
	htmlMap.insert("Uacute",  QChar(218));
	htmlMap.insert("Ucirc",  QChar(219));
	htmlMap.insert("Uuml",  QChar(220));
	htmlMap.insert("Yacute",  QChar(221));
	htmlMap.insert("THORN",  QChar(222));
	htmlMap.insert("szlig",  QChar(223));
	htmlMap.insert("agrave",  QChar(224));
	htmlMap.insert("aacute",  QChar(225));
	htmlMap.insert("acirc",  QChar(226));
	htmlMap.insert("atilde",  QChar(227));
	htmlMap.insert("auml",  QChar(228));
	htmlMap.insert("aring",  QChar(229));
	htmlMap.insert("aelig",  QChar(230));
	htmlMap.insert("ccedil",  QChar(231));
	htmlMap.insert("egrave",  QChar(232));
	htmlMap.insert("eacute",  QChar(233));
	htmlMap.insert("ecirc",  QChar(234));
	htmlMap.insert("euml",  QChar(235));
	htmlMap.insert("igrave",  QChar(236));
	htmlMap.insert("iacute",  QChar(237));
	htmlMap.insert("icirc",  QChar(238));
	htmlMap.insert("iuml",  QChar(239));
	htmlMap.insert("eth",  QChar(240));
	htmlMap.insert("ntilde",  QChar(241));
	htmlMap.insert("ograve",  QChar(242));
	htmlMap.insert("oacute",  QChar(243));
	htmlMap.insert("ocirc",  QChar(244));
	htmlMap.insert("otilde",  QChar(245));
	htmlMap.insert("ouml",  QChar(246));
	htmlMap.insert("divide",  QChar(247));
	htmlMap.insert("oslash",  QChar(248));
	htmlMap.insert("ugrave",  QChar(249));
	htmlMap.insert("uacute",  QChar(250));
	htmlMap.insert("ucirc",  QChar(251));
	htmlMap.insert("uuml",  QChar(252));
	htmlMap.insert("yacute",  QChar(253));
	htmlMap.insert("thorn",  QChar(254));
	htmlMap.insert("yuml",  QChar(255));

	htmlMap.insert("fnof",  QChar(402));

	htmlMap.insert("Alpha",  QChar(913));
	htmlMap.insert("Beta",  QChar(914));
	htmlMap.insert("Gamma",  QChar(915));
	htmlMap.insert("Delta",  QChar(916));
	htmlMap.insert("Epsilon",  QChar(917));
	htmlMap.insert("Zeta",  QChar(918));
	htmlMap.insert("Eta",  QChar(919));
	htmlMap.insert("Theta",  QChar(920));
	htmlMap.insert("Iota",  QChar(921));
	htmlMap.insert("Kappa",  QChar(922));
	htmlMap.insert("Lambda",  QChar(923));
	htmlMap.insert("Mu",  QChar(924));
	htmlMap.insert("Nu",  QChar(925));
	htmlMap.insert("Xi",  QChar(926));
	htmlMap.insert("Omicron",  QChar(927));
	htmlMap.insert("Pi",  QChar(928));
	htmlMap.insert("Rho",  QChar(929));
	htmlMap.insert("Sigma",  QChar(931));
	htmlMap.insert("Tau",  QChar(932));
	htmlMap.insert("Upsilon",  QChar(933));
	htmlMap.insert("Phi",  QChar(934));
	htmlMap.insert("Chi",  QChar(935));
	htmlMap.insert("Psi",  QChar(936));
	htmlMap.insert("Omega",  QChar(937));
	htmlMap.insert("alpha",  QChar(945));
	htmlMap.insert("beta",  QChar(946));
	htmlMap.insert("gamma",  QChar(947));
	htmlMap.insert("delta",  QChar(948));
	htmlMap.insert("epsilon",  QChar(949));
	htmlMap.insert("zeta",  QChar(950));
	htmlMap.insert("eta",  QChar(951));
	htmlMap.insert("theta",  QChar(952));
	htmlMap.insert("iota",  QChar(953));
	htmlMap.insert("kappa",  QChar(954));
	htmlMap.insert("lambda",  QChar(955));
	htmlMap.insert("mu",  QChar(956));
	htmlMap.insert("nu",  QChar(957));
	htmlMap.insert("xi",  QChar(958));
	htmlMap.insert("omicron",  QChar(959));
	htmlMap.insert("pi",  QChar(960));
	htmlMap.insert("rho",  QChar(961));
	htmlMap.insert("sigmaf",  QChar(962));
	htmlMap.insert("sigma",  QChar(963));
	htmlMap.insert("tau",  QChar(964));
	htmlMap.insert("upsilon",  QChar(965));
	htmlMap.insert("phi",  QChar(966));
	htmlMap.insert("chi",  QChar(967));
	htmlMap.insert("psi",  QChar(968));
	htmlMap.insert("omega",  QChar(969));
	htmlMap.insert("thetasy",  QChar(977));
	htmlMap.insert("upsih",  QChar(978));
	htmlMap.insert("piv",  QChar(982));

	htmlMap.insert("bull",  QChar(8226));
	htmlMap.insert("hellip",  QChar(8230));
	htmlMap.insert("prime",  QChar(8242));
	htmlMap.insert("Prime",  QChar(8243));
	htmlMap.insert("oline",  QChar(8254));
	htmlMap.insert("frasl",  QChar(8260));

	htmlMap.insert("weierp",  QChar(8472));
	htmlMap.insert("image",  QChar(8465));
	htmlMap.insert("real",  QChar(8476));
	htmlMap.insert("trade",  QChar(8482));
	htmlMap.insert("alefsym",  QChar(8501));

	htmlMap.insert("larr",  QChar(8592));
	htmlMap.insert("uarr",  QChar(8593));
	htmlMap.insert("rarr",  QChar(8594));
	htmlMap.insert("darr",  QChar(8595));
	htmlMap.insert("harr",  QChar(8596));
	htmlMap.insert("crarr",  QChar(8629));
	htmlMap.insert("lArr",  QChar(8656));
	htmlMap.insert("uArr",  QChar(8657));
	htmlMap.insert("rArr",  QChar(8658));
	htmlMap.insert("dArr",  QChar(8659));
	htmlMap.insert("hArr",  QChar(8660));

	htmlMap.insert("forall",  QChar(8704));
	htmlMap.insert("part",  QChar(8706));
	htmlMap.insert("exist",  QChar(8707));
	htmlMap.insert("empty",  QChar(8709));
	htmlMap.insert("nabla",  QChar(8711));
	htmlMap.insert("isin",  QChar(8712));
	htmlMap.insert("notin",  QChar(8713));
	htmlMap.insert("ni",  QChar(8715));
	htmlMap.insert("prod",  QChar(8719));
	htmlMap.insert("sum",  QChar(8721));
	htmlMap.insert("minus",  QChar(8722));
	htmlMap.insert("lowast",  QChar(8727));
	htmlMap.insert("radic",  QChar(8730));
	htmlMap.insert("prop",  QChar(8733));
	htmlMap.insert("infin",  QChar(8734));
	htmlMap.insert("ang",  QChar(8736));
	htmlMap.insert("and",  QChar(8743));
	htmlMap.insert("or",  QChar(8744));
	htmlMap.insert("cap",  QChar(8745));
	htmlMap.insert("cup",  QChar(8746));
	htmlMap.insert("int",  QChar(8747));
	htmlMap.insert("there4",  QChar(8756));
	htmlMap.insert("sim",  QChar(8764));
	htmlMap.insert("cong",  QChar(8773));
	htmlMap.insert("asymp",  QChar(8776));
	htmlMap.insert("ne",  QChar(8800));
	htmlMap.insert("equiv",  QChar(8801));
	htmlMap.insert("le",  QChar(8804));
	htmlMap.insert("ge",  QChar(8805));
	htmlMap.insert("sub",  QChar(8834));
	htmlMap.insert("sup",  QChar(8835));
	htmlMap.insert("nsub",  QChar(8836));
	htmlMap.insert("sube",  QChar(8838));
	htmlMap.insert("supe",  QChar(8839));
	htmlMap.insert("oplus",  QChar(8853));
	htmlMap.insert("otimes",  QChar(8855));
	htmlMap.insert("perp",  QChar(8869));
	htmlMap.insert("sdot",  QChar(8901));

	htmlMap.insert("lceil",  QChar(8968));
	htmlMap.insert("rceil",  QChar(8969));
	htmlMap.insert("lfloor",  QChar(8970));
	htmlMap.insert("rfloor",  QChar(8971));
	htmlMap.insert("lang",  QChar(9001));
	htmlMap.insert("rang",  QChar(9002));

	htmlMap.insert("loz",  QChar(9674));

	htmlMap.insert("spades",  QChar(9824));
	htmlMap.insert("clubs",  QChar(9827));
	htmlMap.insert("hearts",  QChar(9829));
	htmlMap.insert("diams",  QChar(9830));

	htmlMap.insert("quot",  QChar(34));
	htmlMap.insert("amp",  QChar(38));
	htmlMap.insert("lt",  QChar(60));
	htmlMap.insert("gt",  QChar(62));
	htmlMap.insert("OElig",  QChar(338));
	htmlMap.insert("oelig",  QChar(339));
	htmlMap.insert("Scaron",  QChar(352));
	htmlMap.insert("scaron",  QChar(353));
	htmlMap.insert("Yuml",  QChar(376));
	htmlMap.insert("circ",  QChar(710));
	htmlMap.insert("tilde",  QChar(732));
	htmlMap.insert("ensp",  QChar(8194));
	htmlMap.insert("emsp",  QChar(8195));
	htmlMap.insert("thinsp",  QChar(8201));
	htmlMap.insert("zwnj",  QChar(8204));
	htmlMap.insert("zwj",  QChar(8205));
	htmlMap.insert("lrm",  QChar(8206));
	htmlMap.insert("rlm",  QChar(8207));
	htmlMap.insert("ndash",  QChar(8211));
	htmlMap.insert("mdash",  QChar(8212));
	htmlMap.insert("lsquo",  QChar(8216));
	htmlMap.insert("rsquo",  QChar(8217));
	htmlMap.insert("sbquo",  QChar(8218));
	htmlMap.insert("ldquo",  QChar(8220));
	htmlMap.insert("rdquo",  QChar(8221));
	htmlMap.insert("bdquo",  QChar(8222));
	htmlMap.insert("dagger",  QChar(8224));
	htmlMap.insert("Dagger",  QChar(8225));
	htmlMap.insert("permil",  QChar(8240));
	htmlMap.insert("lsaquo",  QChar(8249));
	htmlMap.insert("rsaquo",  QChar(8250));
	htmlMap.insert("euro",  QChar(8364));
}


QString formatTable(const QVector<QVector<QString> > resultTable)
{
	int cnt = resultTable.count() > 0 ? resultTable.first().count() : 0;

	QVector<int> colWidth(cnt);
	colWidth.fill(0);

	for (int row=0; row<resultTable.count(); ++row)
		for (int col=0; col<cnt; ++col)
			if (resultTable[row][col].length()>colWidth[col])
				colWidth[col]=resultTable[row][col].length();

	QString result;
	for (int row=0; row<resultTable.count(); ++row)
	{
		QString line;
		QVector<QString> rowVector=resultTable[row];
		for (int col=0; col<cnt; ++col)
		{
			QString term=rowVector[col];
			int l=term.length();
			int maxL=colWidth[col];
			line+=term;
			line+=QString().fill(' ',(maxL-l)/TAB_SIZE + 2);
		}
		result+="\n"+line;
	}
	return result;
}
