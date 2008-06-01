#ifndef VCARDWRAPPER_H_
#define VCARDWRAPPER_H_

#include <QString>

#include <gloox/jid.h>
#include <gloox/vcard.h>

class VCardWrapper
{
public:
	VCardWrapper();
	VCardWrapper(const std::string& id, const gloox::JID &jid, gloox::VCard *vcard);
	VCardWrapper(const VCardWrapper& other);
	virtual ~VCardWrapper();
	gloox::JID jid() const { return jid_; }
	const gloox::VCard& vcard() const { return vcard_; }
	bool isEmpty() const { return empty; }
	QString id() const {return id_; }
private:
	QString id_;
	bool empty;
	gloox::JID jid_;
	gloox::VCard vcard_;
};

#endif /*VCARDWRAPPER_H_*/
