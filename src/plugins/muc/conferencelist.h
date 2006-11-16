#ifndef CONFERENCELIST_H
#define CONFERENCELIST_H

#include "conference.h"

class ConferenceList: public QList<Conference*>
{
public:
	ConferenceList();
	~ConferenceList();
	void clear();
	void remove(Conference* conf);
	Conference *byName(const QString&);
};

#endif
