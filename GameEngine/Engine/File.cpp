#include "File.h"

Engine::File::File(std::string fileName)
{
	//in: input operations
	//out: output operations
	//app: append to end of file 
	//if file already existed when it was opened delete the previous content and replace with new ones
	file_.open(fileName, std::ios::out | std::ios::in | std::ios::trunc | std::ios::ate);
}

bool Engine::File::writeToFile(std::string data)
{
	//if file was not successfully opened then return false;
	if (!file_.is_open())
	{
		return false;
	}
	else
	{
		file_ << data;
	}
	return true;

}

bool Engine::File::readFile(std::string& output)
{
	if (!file_.is_open())
	{
		return false;
	}
	else
	{
		std::string out;
		std::string currLine;
		while (std::getline(file_, currLine))
		{
			out.append(currLine);
		}
		output = out;
	}
	return true;
}

Engine::File::~File()
{
	file_.close();
}
