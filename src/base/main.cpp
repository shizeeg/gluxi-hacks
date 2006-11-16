#include "gluxibot.h"

#include <QLocale>
#include <QTextCodec>

int main(/* int argc, char*argv[] */ )
{
	QLocale::setDefault(QLocale("en_US"));
	QTextCodec::setCodecForCStrings (QTextCodec::codecForName("UTF-8"));
	GluxiBot *bot=new GluxiBot();
	bot->run();
	delete bot;
	return 0;
}

