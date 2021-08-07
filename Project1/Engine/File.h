#pragma once
#include <string>
#include <fstream>

namespace Engine
{
	class File
	{
	public:
		File(std::string fileName);

		~File();

		bool writeToFile(std::string data);
		bool readFile(std::string& output);

		std::fstream file_;
	};
}
