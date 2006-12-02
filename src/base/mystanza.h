#ifndef MYSTANZA_H
#define MYSTANZA_H

namespace gloox
{
	class Stanza;
}

class MyStanza
{
public:
	MyStanza();
	MyStanza(const MyStanza&);
	MyStanza(gloox::Stanza* s);
	~MyStanza();

	gloox::Stanza* stanza() const { return st;};
	void setStanza(gloox::Stanza* stanza);
private:
	gloox::Stanza *st;
};

#endif

