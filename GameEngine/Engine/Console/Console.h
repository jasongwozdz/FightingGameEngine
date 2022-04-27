#pragma once
#include <unordered_map>
#include <list>
#include <queue>

#include "../Events.h"

class Application;
struct ImGuiInputTextCallbackData;

class Console
{
public:
	enum CommandType
	{
		CommandType_bool,
		CommandType_float,
		CommandType_int,
		CommandType_function,
	};

	union CommandData
	{
		bool boolData;
		float floatData;
		int intData;
		void(*funcData)(void);
	};

	struct CommandVar
	{
		CommandType type;
		CommandData data;
	};

	Console(Application* application);
	~Console() = default;

	static Console* getInstance() { return instance_; };
	static void setInstance(Console* console) { instance_ = console; }
	static void destroyInstance() { delete instance_; instance_ = nullptr; }

	void handleEvent(Events::Event& currEvent);
	void handleKeyPressedEvent(Events::KeyPressedEvent& keyPressedEvent);

	void addFloatVar(std::string command, float variableDefault);
	float getFloatVar(std::string comamnd);
	void addIntVar(std::string command, int variableDefault);
	int getIntVar(std::string command);
	void addBoolVar(std::string command, bool variableDefault);
	bool getBoolVar(std::string command);

	void update();

	void inputTextCallback(ImGuiInputTextCallbackData* data);

private:
	void drawConsole();
	void getAllCandidates(ImGuiInputTextCallbackData* data);
	

	static Console* instance_;
	bool shouldDraw_ = false;
	std::unordered_map<std::string, CommandVar> commandDataMap_;
	std::vector<std::string> candidates_;//candidates for likely command matches
	int currCandidate_;

	std::list<std::string> commandHistory_;

};