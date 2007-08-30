#include "directproxy.h"

//#define MY_DEBUG

#include "socketw/src/SocketW.h"

#include <QtGlobal>

#include <errno.h>

#ifdef MY_DEBUG
static int lastFileID=0;
#endif

DirectProxy::DirectProxy(int id, int typ, QString nfo)
		: BaseProxy(id, typ, nfo)
{
	qWarning() << "DirectProxy";
// 	lastURL="";
}


DirectProxy::~DirectProxy()
{}

QByteArray DirectProxy::httpRequest(const QString& prot, const QString&host, const QString&url, QStringList* cookies, QStringList* custHeader, QString& lastURL)
{
	redirectTo.clear();
	QByteArray arr;
	int length;
	if (!custHeader && !wascustom)
	{
		arr.append(QString("GET %1 HTTP/1.0\r\n").arg(url));
	}
	else
	{
		arr.append(QString("POST %1 HTTP/1.0\r\n").arg(url));
	}
	if (lastURL!="") arr.append(QString("Referer: %1\r\n").arg(lastURL));
	lastURL=QString("%1%2%3").arg(prot,host,url);
	arr.append(QString("Host: %1\r\n").arg(host));
	arr.append(QString("Accept: %1\r\n").arg(contentTypes.join(",")));
	arr.append(QString("User-Agent: %1\r\n").arg(userAgent));
	arr.append("Connection: close\r\n");
	if (cookies->count())
	{
		arr.append("Cookie: ");
	for (int i=0; i<cookies->count(); i++)
	{
		arr.append(QString("%1").arg(cookies->value(i)));
		if (i!=cookies->count()-1) arr.append("; ");
	}
		arr.append("\r\n");
	}

	if (custHeader)
	{
		length=0;
		for (int i=0; i<custHeader->count(); i++)
			length+=custHeader->value(i).length();

		arr.append("Content-Type: application/x-www-form-urlencoded\r\n");
		arr.append(QString("Content-Length: %1\r\n\r\n").arg(length));
		for (int i=0; i<custHeader->count(); i++)
			arr.append(QString("%1\r\n").arg(custHeader->value(i)));
	}
	if (wascustom)
	{
		arr.append(QString("Content-Length: %1\r\n").arg(postSize));
		arr.append(postData);
	}
	else
		arr.append("\r\n");

#ifdef Q_WS_WIN
	arr.replace("\r\n","\n");
#endif

 	qWarning() << "---------------------------------->>>>>>>>>>>>" << endl << arr << "----------------------------------";
	return arr;
}

int DirectProxy::parseResponseLine(QString& line, QStringList*cookies, QString&redirect)
{
	qWarning() << "Response | "<<line;
	headers.append(line);
	if (line.startsWith("HTTP/"))
	{
		errorString=line.section(" ",1);
		QString code=line.section(" ",1,1);
		// 		qDebug() << "CODE:" << code;
		if (code[0]=='2') return 0;
		if (code[0]=='4') return -1;
		if (code[0]=='3') return 0;
		return 0;
	}
	//"Set-Cookie: PHPSESSID=0a59981fcf687e75a80f243eb691f23f; path=/; domain=lafox.net"
	if (line.toUpper().startsWith("SET-COOKIE: "))
	{
		QString cookie=line.section(" ",1,100);
		cookie=cookie.section(";",0,0);
		if (cookie.endsWith(";")) cookie.remove(cookie.length()-1,1);
		QString cookieName=cookie.section("=",0,0);
		QString cookieValue=cookie.section("=",1,100).trimmed();
		QRegExp exp(QString("%1.*").arg(cookieName));
		int idd=cookies->indexOf(exp);
		while (idd>=0)
		{
			cookies->removeAt(idd);
			idd=cookies->indexOf(exp);
		}
		if (cookieValue!="" && cookieValue!="deleted")
			cookies->append(QString("%1").arg(cookie));
	}
	if (line.toUpper().startsWith("CONTENT-TYPE:"))
	{
		line=line.section(": ",1);
		contentType=line.section(';',0,0).trimmed(); // get Content-type
		// Find Charset=
		QRegExp exp("CHARSET=([A-Za-z0-9\\-_]+)");
		exp.setMinimal(false);
		exp.setCaseSensitivity(Qt::CaseInsensitive);
		if (exp.indexIn(line)>=0)
			charset=exp.capturedTexts()[1];
		if (!contentTypes.isEmpty() && contentTypes.indexOf(contentType)<0)
		{
			errorString=QString("Content-type \"%1\" is not allowed").arg(contentType);
			return -1;
		}
		return 0;

	}
	if (line.toUpper().startsWith("CONTENT-LENGTH:"))
	{
		contentLength=line.section(" ",1,1).toInt();
		if (contentLength>maxSize)
		{
			errorString=QString("Content-length limit: %1").arg(contentLength);
			return -1;
		}
		return contentLength;
	}

	if (line.toUpper().startsWith("LOCATION:"))
	{
		redirect=line.section(":",1,1000).trimmed();
		redirectTo=redirect;
		errorString=QString("Redirect to: %1").arg(redirect);
	}

	return 0;
}

int DirectProxy::readResponse(SWInetSocket* socket, MyBuf *buf, QStringList* cookies, QString&redirect)
{
	SWBaseSocket::SWBaseError error;
	char ch;
	QString s;
	int wasn=0;
	s="";
	int res;
	int ret;

	ret =0;
	redirect="";
	while (1)
	{

		if (buf->pos>=buf->count)
		{
			buf->count=socket->recv(buf->data,MYBUFSIZE,&error);
			if (buf->count<=0) return -1;
			buf->pos=0;
		}

		ch=buf->data[buf->pos++];

		if (ch==0x0d) continue;
		if (ch=='\n')
		{
			if (wasn) return ret;
			res=parseResponseLine(s,cookies,redirect);
			if (res<0) return res;
			if (res>0) ret=res;
			s="";
			wasn=1;

		}
		else
		{
			wasn=0;
			s+=ch;
		}

	}
	return ret;
}

QString DirectProxy::fetchPage(const QString&host, const QString&url, QString&referer, QStringList *cookies, QStringList*custHeader, int custom)
{
	headers.clear();
	charset.clear();
	contentLength=0;
	SWBaseSocket::SWBaseError error;
	QString redirect;
	SWInetSocket socket;
// 	socket.set_timeout(Preferences::connectionTimeout, Preferences::connectionTimeout*1000*1000);
	wascustom=custom;

	if (!socket.connect(80,host.toLatin1().data(),&error))
	{
		errorString=QString::fromStdString(socket.get_error());
		return 0;
	}
	QByteArray arr=httpRequest("http://",host,url, cookies, custHeader, referer);
	if (!socket.send(arr.data(),arr.size(),&error)) return 0;

	struct MyBuf buf;

	buf.count=socket.recv(buf.data,MYBUFSIZE,&error);
	buf.pos=0;
	if (buf.count<=0) return 0;
	int respResult = readResponse(&socket,&buf,cookies,redirect);
	if (respResult<0) return 0;

	if (redirect!="")
	{
 		qWarning("Redirect");
		arr.clear();
		arr.append(redirect);
		return QTextStream(arr).readAll();
	}

	if (headersOnly)
		return 0;

	arr.clear();
	int total=0;
	if (buf.pos<buf.count)
	{
		arr.append(QByteArray(buf.data+buf.pos,buf.count-buf.pos));
		total+=arr.count();
		buf.count=0;
	}

	while (respResult==0 || (total<respResult))
	{
		buf.count=socket.recv(buf.data,MYBUFSIZE,&error);
		if (buf.count)
		{
			arr.append(QByteArray(buf.data,buf.count));
			total+=buf.count;
		}
//  		qDebug() << total << " of " << respResult;
		if (error == SWBaseSocket::ok) continue;
		if (error == SWBaseSocket::notConnected || error==SWBaseSocket::terminated
			|| error==SWBaseSocket::timeout) break;
		return 0;
	}

#ifdef MY_DEBUG
	lastFileID++;
	QFile fp("files/"+QString::number(lastFileID).rightJustified(2,QChar('0'))+".html");
	fp.open(QIODevice::WriteOnly);
	fp.write(arr.data(),arr.count());
	fp.close();
#endif
	lastData=arr;
	return QTextStream(arr).readAll();
}

QString DirectProxy::fetchPageSSL(const QString&host, const QString&url, QString&referer, QStringList *cookies, QStringList*custHeader, int custom)
{
	headers.clear();
	charset.clear();
	contentLength=0;
	QString redirect;
	SWBaseSocket::SWBaseError error;
	SWSSLSocket socket;
	lastData.clear();

	wascustom=custom;
// 	socket.set_timeout(Preferences::connectionTimeout, Preferences::connectionTimeout*1000*1000);
//	Preferences::addHost(host);

	if (!socket.connect(443,host.toLatin1().data(),&error))
	{
		errorString=QString::fromStdString(socket.get_error());
		return 0;
	}
	QByteArray arr=httpRequest("https://",host,url, cookies, custHeader, referer);
	if (!socket.send(arr.data(),arr.size(),&error)) return 0;

	struct MyBuf buf;

	buf.count=socket.recv(buf.data,MYBUFSIZE,&error);
	buf.pos=0;
	if (buf.count<=0) return 0;
	int respResult = readResponse(&socket,&buf,cookies, redirect);
	if (respResult<0) return 0;

	if (redirect!="")
	{
// 		qDebug("Redirect");
		arr.clear();
		arr.append(redirect);
		return QTextStream(arr).readAll();
	}

	arr.clear();
	int total=0;
	if (buf.pos<buf.count)
	{
		arr.append(QByteArray(buf.data+buf.pos,buf.count-buf.pos));
		total+=arr.count();
		buf.count=0;
	}


	while (respResult==0 || (total<respResult))
	{
		buf.count=socket.recv(buf.data,MYBUFSIZE,&error);
		if (buf.count)
		{
			arr.append(QByteArray(buf.data,buf.count));
			total+=buf.count;
		}
// 		qDebug() << total << " of " << respResult;
		if (error == SWBaseSocket::ok) continue;
		if (error == SWBaseSocket::notConnected || error==SWBaseSocket::terminated
			||error==SWBaseSocket::timeout) break;
		return 0;
	}
	lastData=arr;
#ifdef MY_DEBUG
	lastFileID++;
	QFile fp("files/"+QString::number(lastFileID).rightJustified(2,QChar('0'))+".html");
	fp.open(QIODevice::WriteOnly);
	fp.write(arr.data(),arr.count());
	fp.close();
#endif

	return QTextStream(arr).readAll();
}
