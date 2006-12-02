#include "pingrequest.h"

PingRequest::PingRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest=dest;
}

PingRequest::~PingRequest()
{
}

void PingRequest::exec()
{
	proc=new QProcess(this);

}

