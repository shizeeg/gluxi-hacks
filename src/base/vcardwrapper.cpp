#include "vcardwrapper.h"

VCardWrapper::VCardWrapper()
{
	empty=true;
}

VCardWrapper::VCardWrapper(const std::string& id, const gloox::JID &jid, gloox::VCard *vcard)
{
	jid_=jid;
	id_=QString::fromStdString(id);
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
	id_=other.id();
	jid_=other.jid();
	vcard_=gloox::VCard(other.vcard());
	empty=other.isEmpty();
}

VCardWrapper::~VCardWrapper()
{
}

int VCardWrapper::photoSize() const
{
	gloox::VCard vcard=gloox::VCard(vcard_);
	std::string photoContentStd=vcard.photo().binval;
	return photoContentStd.size();
}

QString VCardWrapper::vcardStr() const
{
	gloox::VCard vcard=gloox::VCard(vcard_);

	QString fullName=QString::fromStdString(vcard.formattedname()).trimmed();
	QString nickName=QString::fromStdString(vcard.nickname()).trimmed();
	QString birthday=QString::fromStdString(vcard.bday()).trimmed();
	QString homepage=QString::fromStdString(vcard.url()).trimmed();
	QString desc=QString::fromStdString(vcard.desc()).trimmed();
	QString location;
	if (!vcard.addresses().empty())
	{
		gloox::VCard::Address addr=*(vcard.addresses().begin());
		QString country=QString::fromStdString(addr.ctry).trimmed();
		QString city=QString::fromStdString(addr.locality).trimmed();
		if (!country.isEmpty())
			location=country;
		if (!city.isEmpty())
		{
			if (location.isEmpty())
				location=city;
			else
				location=QString("%1, %2").arg(location).arg(city);
		}
	}

	QString photoMime=QString::fromStdString(vcard.photo().type).trimmed();

	if (photoMime.isEmpty())
		photoMime="N/A";

	std::string photoContentStd=vcard.photo().binval;
	QByteArray photoContent=QByteArray(photoContentStd.data(),
			photoContentStd.size());

	QString replyStr;
	if (!fullName.isEmpty())
		replyStr+=QString("\nName: %1").arg(fullName);
	if (!nickName.isEmpty())
		replyStr+=QString("\nNick: %1").arg(nickName);
	if (!birthday.isEmpty())
		replyStr+=QString("\nBirthday: %1").arg(birthday);
	if (!homepage.isEmpty())
		replyStr+=QString("\nHomepage: %1").arg(homepage);
	if (!location.isEmpty())
		replyStr+=QString("\nLocation: %1").arg(location);
	if (!photoContent.isEmpty())
		replyStr+=QString("\nPhoto: type: %1, size: %2 bytes").arg(photoMime).arg(photoContent.size());
	if (!desc.isEmpty())
		replyStr+=QString("\nAbout: %1").arg(desc);
	return replyStr;
}
