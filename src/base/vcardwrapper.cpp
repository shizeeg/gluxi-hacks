#include "vcardwrapper.h"

VCardWrapper::VCardWrapper()
{
	empty=true;
}

VCardWrapper::VCardWrapper(const gloox::JID &jid, gloox::VCard *vcard)
{
	jid_=jid;
	if (vcard!=0)
	{
		empty=false;
		vcard_=gloox::VCard(*vcard);
	}
	else
	{
		empty=true;
	}
	
}

VCardWrapper::VCardWrapper(const VCardWrapper& other)
{
	jid_=other.jid();
	vcard_=gloox::VCard(other.vcard());
	empty=other.isEmpty();
}

VCardWrapper::~VCardWrapper()
{		
}
