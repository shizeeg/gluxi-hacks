#include "mystanza.h"

#include <gloox/stanza.h>

MyStanza::MyStanza()
{
	st=0;
}

MyStanza::MyStanza(const MyStanza& stanza)
{
	st=stanza.stanza()->clone();
}

MyStanza::MyStanza(gloox::Stanza* stanza)
{
	st=stanza->clone();
}

MyStanza::~MyStanza()
{
	if (st)
		delete st;
}

void MyStanza::setStanza(gloox::Stanza* stanza)
{
	if (st)
		delete st;
	st=stanza->clone();
}

