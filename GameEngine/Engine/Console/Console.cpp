#include "Console.h"
#include "../../Application.h"
#include "../libs/imgui/imgui.h"

Console* Console::instance_ = nullptr;

Console::Console(Application* application)
{
	application->addEventCallback(ENGINE_EVENT_CALLBACK(Console::handleEvent));
    addFloatVar("a", 0.0f);
    addBoolVar("abc", false);
    addIntVar("abcd", 2);
}

void Console::handleEvent(Events::Event& currEvent)
{
	Events::EventDispatcher dispatcher(currEvent);
	dispatcher.dispatch<Events::KeyPressedEvent>(std::bind(&Console::handleKeyPressedEvent, this, std::placeholders::_1));
}

void Console::handleKeyPressedEvent(Events::KeyPressedEvent& keyPressedEvent)
{
	if (keyPressedEvent.KeyCode == GLFW_KEY_F1)
	{
		shouldDraw_ = !shouldDraw_;
	}
}

void Console::addFloatVar(std::string command, float variableDefault)
{
	CommandVar commandVar = { CommandType_float, variableDefault };
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

void Console::addIntVar(std::string command, int variableDefault)
{
    CommandVar commandVar = { CommandType_int, variableDefault };
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

void Console::addBoolVar(std::string command, bool variableDefault)
{
    CommandVar commandVar = { CommandType_bool, variableDefault };
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
    for (auto item : commandHistory_)
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

    ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackEdit;
    char inputBuf[256];
    memset(inputBuf, 0, sizeof(inputBuf));
    ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("Input", inputBuf, IM_ARRAYSIZE(inputBuf), inputTextFlags, &staticInputTextCallback, nullptr))
    {
        bool printVal = false;
        commandHistory_.insert(commandHistory_.begin(), inputBuf);
        std::string str(inputBuf);
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
                        commandHistory_.insert(commandHistory_.begin() , command.second.data.boolData ? "true" : "false");
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
                        commandHistory_.insert(commandHistory_.begin(), std::to_string(command.second.data.floatData));
                    }
                    else
                    {
                        command.second.data.floatData = std::stof(val);
                    }
                    break;
                case CommandType_int:
                    if (printVal)
                    {
                        commandHistory_.insert(commandHistory_.begin(), std::to_string(command.second.data.intData));
                    }
                    else
                    {
                        command.second.data.intData = std::stoi(val);
                    }
                    break;
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
