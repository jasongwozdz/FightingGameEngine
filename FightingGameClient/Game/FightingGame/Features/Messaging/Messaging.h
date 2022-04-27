#pragma once
#include <functional>
#include <unordered_map>

class Entity;

class Messaging
{
public:
	enum class MsgId
	{
		MsgId_BaseMsg,
	};

	struct BaseMsgData
	{
		Entity* entity_;
		MsgId msgId_;
	};

	Messaging() = default;
	~Messaging() = default;

	static void setInstance(Messaging* messaging);
	static Messaging* getInstance() { return instance_; };
	static void destroyInstance();

	void sendMessage(MsgId msgId, void* msgData);

	typedef std::function<void(MsgId, void*)> MessagingCallbackFunc;
	void addMessageHandler(MsgId msgId, MessagingCallbackFunc& func);
	
private:
	static Messaging* instance_;
	std::unordered_map<MsgId, std::vector<MessagingCallbackFunc>> msgCallbackMap_;

};