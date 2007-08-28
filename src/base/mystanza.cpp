#include "mystanza.h"

#include <gloox/stanza.h>

MyStanza::MyStanza()
{
	st=0;
}

MyStanza::MyStanza(const MyStanza& stanza)
{
	st=new gloox::Stanza(stanza.stanza());
}

MyStanza::MyStanza(gloox::Stanza* stanza)
{
	st=new gloox::Stanza(stanza);
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
	st=new gloox::Stanza(stanza);
}

