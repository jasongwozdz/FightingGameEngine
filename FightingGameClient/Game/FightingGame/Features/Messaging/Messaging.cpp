#include "Messaging.h"

Messaging* Messaging::instance_ = nullptr;

void Messaging::setInstance(Messaging* messaging)
{
	instance_ = messaging;
}

void Messaging::destroyInstance()
{
	delete instance_;
	instance_ = nullptr;
}

void Messaging::sendMessage(MsgId msgId, void* msgData)
{
	auto found = msgCallbackMap_.find(msgId);
	if (found != msgCallbackMap_.end())
	{
		for (auto callback : found->second)
		{
			callback(msgId, msgData);
		}
	}
}

void Messaging::addMessageHandler(MsgId msgId, MessagingCallbackFunc& func)
{
	auto found = msgCallbackMap_.find(msgId);
	if (found == msgCallbackMap_.end())
	{
		msgCallbackMap_.insert({ msgId, { func } });
	}
	else
	{
		found->second.push_back(func);
	}
}
