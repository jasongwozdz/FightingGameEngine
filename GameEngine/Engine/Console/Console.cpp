#include "Console.h"
#include "../../Application.h"
#include "../libs/imgui/imgui.h"

Console* Console::instance_ = nullptr;

Console::Console(Application* application)
{
	application->addEventCallback(ENGINE_EVENT_CALLBACK(Console::handleEvent));
    addFloatVar("test.float", 0.0f);
    addBoolVar("test.bool", false);
    addIntVar("test.int", 2);
	addVec3Var("test.vec3", { 1.0f, 0.0f, 2.0f });
}

void Console::handleEvent(Events::Event& currEvent)
{
	Events::EventDispatcher dispatcher(currEvent);
	dispatcher.dispatch<Events::KeyPressedEvent>(std::bind(&Console::handleKeyPressedEvent, this, std::placeholders::_1));
}

void Console::handleKeyPressedEvent(Events::KeyPressedEvent& keyPressedEvent)
{
	switch (keyPressedEvent.KeyCode)
	{
	case GLFW_KEY_F1:
	{
		shouldDraw_ = !shouldDraw_;
		break;
	}
	}
}

void Console::addFloatVar(std::string command, float variableDefault, CallbackFunc func)
{
	CommandVar commandVar = { CommandType_float, variableDefault, func };
	commandDataMap_.insert({ command, commandVar });
}

float Console::getFloatVar(std::string command)
{
	auto found = commandDataMap_.find(command);
	if (found != commandDataMap_.end())
	{
		return found->second.data.floatData;
	}

	return 0.0f;
}

void Console::addIntVar(std::string command, int variableDefault, CallbackFunc func)
{
    CommandVar commandVar = { CommandType_int, variableDefault, func };
    commandDataMap_.insert({ command, commandVar });
}

int Console::getIntVar(std::string command)
{
    auto found = commandDataMap_.find(command);
    if (found != commandDataMap_.end())
    {
        return found->second.data.intData;
    }

    return 0;
}

void Console::addBoolVar(std::string command, bool variableDefault, CallbackFunc func)
{
    CommandVar commandVar = { CommandType_bool, variableDefault, func};
    commandDataMap_.insert({ command, commandVar });
}

bool Console::getBoolVar(std::string command)
{
    auto found = commandDataMap_.find(command);
    if (found != commandDataMap_.end())
    {
        return found->second.data.boolData;
    }
    return false;
}

void Console::addVec3Var(std::string command, glm::vec3 variableDefault, CallbackFunc func)
{
	CommandVar commandVar{};
	commandVar.type = CommandType_vec3;
	commandVar.data.vec3Data = variableDefault;
	commandVar.func = func;
	commandDataMap_.insert({ command, commandVar });
}

glm::vec3 Console::getVec3Var(std::string command)
{
	return commandDataMap_[command].data.vec3Data;
}


void Console::update()
{
	if (shouldDraw_)
	{
		drawConsole();
	}
}

static int staticInputTextCallback(ImGuiInputTextCallbackData* data)
{
    Console::getInstance()->inputTextCallback(data);
    return 0;
}

void Console::inputTextCallback(ImGuiInputTextCallbackData* data)
{
    static bool lastCallbackWasCompletion = false;
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion:
    {
        bool replace = false;
        std::string currString(data->Buf);
        if (!lastCallbackWasCompletion)
        {
            getAllCandidates(data);
            if (candidates_.size() > 0)
            {
                replace = true;
                currCandidate_ = 0;
            }
        }
        else
        {
            if (candidates_.size() > 0)
            {
                currCandidate_ = ++currCandidate_ % (candidates_.size());
                replace = true;
            }
        }

        if(replace)
        {
            data->DeleteChars(0, currString.size());
            data->InsertChars(0, candidates_[currCandidate_].c_str());
        }

        lastCallbackWasCompletion = true;
        break;
    }
    case ImGuiInputTextFlags_CallbackEdit:
    {
        lastCallbackWasCompletion = false;
        candidates_.clear();
        break;
    }
	case ImGuiInputTextFlags_CallbackHistory:
	{
        std::string currString(data->Buf);
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (history_[currHistory_] != "")
			{
				data->DeleteChars(0, currString.size());
				data->InsertChars(0, history_[currHistory_].c_str());
				currHistory_ = (currHistory_ - 1);
				if (currHistory_ < 0) currHistory_ = HISTORY_SIZE - 1;
			}
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			int checkIndex = (currHistory_ + 1) % HISTORY_SIZE;
			if (history_[checkIndex] != "")
			{
				data->DeleteChars(0, currString.size());
				data->InsertChars(0, history_[checkIndex].c_str());
				currHistory_ = checkIndex;
			}
		}
		break;
	}
    }
}

void Console::drawConsole()
{
    //ImGui::ShowDemoWindow();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(EngineSettings::getSingleton().windowWidth, EngineSettings::getSingleton().windowHeight / 2.0f), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    if (!ImGui::Begin("Console", nullptr, flags))
    {
        ImGui::End();
    }

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
    if (ImGui::BeginPopupContextWindow())
    {
        ImGui::EndPopup();
    }
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
    for (auto item : consoleOutput_)
    {
        ImVec4 color;
        bool has_color = false;
        if (item == "[error]") { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
        if (has_color)
            ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(item.c_str());
        if (has_color)
            ImGui::PopStyleColor();
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackHistory;
    char inputBuf[256];
    memset(inputBuf, 0, sizeof(inputBuf));
    ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("Input", inputBuf, IM_ARRAYSIZE(inputBuf), inputTextFlags, &staticInputTextCallback, nullptr))
    {
        std::string str(inputBuf);
		history_[historyHead_] = str;
		currHistory_ = historyHead_;
		historyHead_ = (historyHead_ + 1) % HISTORY_SIZE;
        bool printVal = false;
        consoleOutput_.insert(consoleOutput_.begin(), inputBuf);
        std::string commandStr;
        std::string val;
        size_t found = str.find(' ');
        if (found == std::string::npos || found == str.size() - 1)//print out value of command
        {
            commandStr = inputBuf;
            printVal = true;
        }
        else
        {
            commandStr = str.substr(0, found);
            val = str.substr(found + 1);
        }

        for (auto& command : commandDataMap_)
        {
            if (strcmp(command.first.c_str(), commandStr.c_str()) == 0)
            {
                switch (command.second.type)
                {
                case CommandType_bool:
                    if (printVal)
                    {
                        consoleOutput_.insert(consoleOutput_.begin() , command.second.data.boolData ? "true" : "false");
                    }
                    else
                    {
                        if (val == "true")
                        {
                            command.second.data.boolData = true;
                        }
                        else if (val == "false")
                        {
                            command.second.data.boolData = false;
                        }
                    }
                    break;
                case CommandType_float:
                    if (printVal)
                    {
                        consoleOutput_.insert(consoleOutput_.begin(), std::to_string(command.second.data.floatData));
                    }
                    else
                    {
                        command.second.data.floatData = std::stof(val);
                    }
                    break;
                case CommandType_int:
                    if (printVal)
                    {
                        consoleOutput_.insert(consoleOutput_.begin(), std::to_string(command.second.data.intData));
                    }
                    else
                    {
                        command.second.data.intData = std::stoi(val);
                    }
                    break;
				case CommandType_vec3:
					if (printVal)
					{
						std::string output = "";
						const glm::vec3& val = command.second.data.vec3Data;
						output = "x: " + std::to_string(val.x) + " y: " + std::to_string(val.y) + " z: " + std::to_string(val.z);
						consoleOutput_.insert(consoleOutput_.begin(), output);
					}
					else
					{
						glm::vec3 setVal = { 0.0f, 0.0f, 0.0f };
						int valIdx = 0;
						std::string valStr;
						for (int charIdx = 0; charIdx < val.size(); charIdx++)
						{
							if (val[charIdx] != ' ')
							{
								valStr += val[charIdx];
								continue;
							}
							setVal[valIdx++] = std::stof(valStr);
							valStr.clear();
						}
						if (valIdx < 3)
						{
							setVal[valIdx] = std::stof(valStr);
						}
						command.second.data.vec3Data = setVal;
					}
					break;
                }

				if (!printVal && command.second.func)
				{
					command.second.func(command.first, &command.second);
				}
            }
        }
    }

	ImGui::End();

}

void Console::getAllCandidates(ImGuiInputTextCallbackData* data)
{
    std::string input(data->Buf);
    for (auto entry : commandDataMap_)
    {
        const std::string& currentEntry = entry.first;
        int i = 0;
        while (i < currentEntry.size() && i < input.size() && input[i] == currentEntry[i])
        {
            i++;
        }

        if (i == input.size())
        {
            candidates_.push_back(currentEntry);
        }
    }

}
