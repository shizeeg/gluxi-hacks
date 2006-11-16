#include "glooxbot.h"

#include <QLocale>
#include <QTextCodec>

int main(/* int argc, char*argv[] */ )
{
	QLocale::setDefault(QLocale("en_US"));
	QTextCodec::setCodecForCStrings (QTextCodec::codecForName("UTF-8"));
	GlooxBot *bot=new GlooxBot();
	bot->run();
	delete bot;
}

