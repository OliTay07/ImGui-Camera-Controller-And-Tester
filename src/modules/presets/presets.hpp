#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <vector>
#include <string>


namespace presets
{
	struct commandValues 
	{
		std::string feature{};
		std::string id{};
		std::string action{};
		std::string parameter{};

	};

	struct testResults 
	{
		int selectedOn{-1};
	};

	std::vector<std::string> load(const std::string path);
	void save(const std::string filePath, const commandValues path);
	//void export (std::string filePath);
}