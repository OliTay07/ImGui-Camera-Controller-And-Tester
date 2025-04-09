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

	std::vector<std::string> load(const std::string path);
}