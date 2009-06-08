#ifndef NICK_H
#define NICK_H

#include <QString>
#include <QDateTime>
#include <QList>

class Conference;
class Jid;
class JidStat;

class Nick
{
public:
	enum Affiliation
	{
		AffiliationNone = 0,
		AffiliationMember = 1,
		AffiliationAdmin = 2,
		AffiliationOwner = 3
	};

	enum Role
	{
		RoleUnknown = -1,
		RoleVisitor = 0,
		RoleParticipant = 1,
		RoleModerator = 2
	};
public:
    Nick(Conference* parent, const QString& nick, const QString& jid=QString::null);
    Nick(Conference* parent, int id);
    ~Nick();

	QString jidStr() const { return myJidS; };
	QString nick() const { return myNick; };
	QString affiliation() const { return myAffiliation; };
	QString role() const { return myRole; };

	Affiliation affiliationValue() const;
	Role roleValue() const;

	QDateTime joined() const { return myJoined; };
	QDateTime lastActivity() const { return myLastActivity; };
	QDateTime lastMessage() const;
	QString show() const { return myShow; };
	QString status() const { return myStatus; };
	Conference* conference() const { return myParent; };
	Jid* jid() const { return myJid; };
	JidStat *jidStat() const { return myJidStat; }
	bool validateRequired() const { return myValidateRequired; };
	bool isVersionStored() const { return versionStored_; }
	QString versionName() const { return versionName_; }
	QString versionOs() const { return versionOs_; }
	QString versionClient() const { return versionClient_; }
	int vcardPhotoSize() const { return vcardPhotoSize_; }
	int id() const { return myId; }

	QStringList similarNicks();
	static QStringList nickToJids(Conference* conf, const QString& n, bool last = false);

	void setJid(const QString& jid);
	void setNick(const QString& nick);
	void setAffiliation(const QString& affiliation) { myAffiliation=affiliation; };
	void setRole(const QString& role) { myRole=role; };
	void setLazyLeave(const bool lazyLeave) { myLazyLeave=lazyLeave; }
	void updateLastActivity(bool message = false);
	void setShow(const QString& show) { myShow=show; };
	void setStatus(const QString& status) { myStatus=status; };
	void setValidateRequired(const bool value) { myValidateRequired=value; };
	void commit();
	static void setAllOffline (Conference* conf);
	bool isDevoicedNoVCard() const {return devoicedNoVCard_; }
	bool setVersionStored(bool v) { versionStored_=v; }
	void setDevoicedNoVCard(bool v) { devoicedNoVCard_ = v; }
	void setVersionName(const QString& name) { versionName_=name; };
	void setVersionOs(const QString& os) { versionOs_=os; };
	void setVersionClient(const QString& client) { versionClient_=client; };
	void setVCardPhotoSize(int vcardPhotoSize) { vcardPhotoSize_=vcardPhotoSize; };
private:
	int myId;
	bool myLazyLeave;
	bool myValidateRequired;
	Conference *myParent;
	Jid *myJid;
	JidStat *myJidStat;
	QString myJidS;
	QString myNick;
	QString myAffiliation;
	QString myRole;
	QDateTime myJoined;
	QDateTime myLastActivity;
	QDateTime myLastMessage;
	QString myShow;
	QString myStatus;
	bool devoicedNoVCard_;
	bool versionStored_;
	QString versionName_;
	QString versionOs_;
	QString versionClient_;
	int vcardPhotoSize_;
	QList<QDateTime> myLastMessages;
};

#endif

