#include "gluxibot.h"
#include "datastorage.h"
#include "segfault.h"

#include <QCoreApplication>
#include <QLocale>
#include <QTextCodec>
#include <QStringList>

#ifndef Q_WS_WIN
#include <sys/types.h>
#include <signal.h>
#endif

GluxiBot *bot;
int wasSignal;

#ifndef Q_WS_WIN
void termHandler(int)
{
	if (!wasSignal)
	{
		wasSignal=1;
		QCoreApplication::postEvent(bot, new QuitEvent("got SIGTERM signal"));
	}
	else
		exit(0);
}

void quitHandler(int)
{
	if (!wasSignal)
	{
		wasSignal=1;
		QCoreApplication::postEvent(bot, 
			new QuitEvent("got SIGINT signal. Possible Ctrl+C from console"));
	}
	else
		exit(0);
}

void installSigHandlers()
{
	wasSignal=0;
        struct sigaction act;
        memset(&act,0,sizeof(act));
        act.sa_handler=termHandler;
        sigaction(SIGTERM, &act, NULL);
	act.sa_handler=quitHandler;
	sigaction(SIGINT, &act, NULL);
}
#endif

int main(int argc, char*argv[])
{
	initSegFaultHandler();
	setenv("LANG","en_US.UTF-8",1);
	QCoreApplication app(argc, argv);
	QLocale::setDefault(QLocale("en_US"));
	QTextCodec::setCodecForCStrings (QTextCodec::codecForName("UTF-8"));
	
	QStringList arguments=app.arguments();

	if (arguments.count()>1)
		DataStorage::instance(arguments[1]);
	
	installSigHandlers();	
	bot=new GluxiBot();
	int res=app.exec();
	delete bot;
	return res;
}

