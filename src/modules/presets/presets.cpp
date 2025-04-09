#include "presets.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <vector>
#include <string>

using json = nlohmann::json;

namespace presets
{
    //TODO
    //want to have a function that reads the json file and creates a vector of commands and returns that vector
    //std::vector<std::string> load(const std::string path)
    //{
    // //get the file at a given location
    // //read the file into nlohman json object
    // //[ush each entry onto a vector of strings
    // //return the vector of strings
    // /*std::vector<std::string> result;
    // return  result;*/


    std::vector<std::string> load(const std::string path)
    {
        nlohmann::json j;

        std::ifstream in(path);
        
        in >> j;

        std::vector<std::string> cmds{};

        if (j.contains("commands"))
        {
            for (const auto& item : j["commands"])
            {
                cmds.push_back(item.dump());
            }
        }

        // now 'push back' onto the std::vector<std::string> unsing emplace_back()
        //std::vector<std::string> result;
        //esult.emplace_back(test);


        //if (j.contains("commands"))
        //{
        //    for (size_t i = 0; i < length; i++)
        //    {

        //    }
        //}
        return cmds;


        ////////////////////////////

        // Get Topology from the project file
        //if (j.contains("Topology"))
        //{
        //    // Clear the existing topology
        //    project.topology.clear();

        //    // Iterate over Topology
        //    auto& topArray = j["Topology"];
        //    if (!topArray.is_array())
        //    {
        //        throw std::runtime_error("'Topology' is not an array");
        //    }

        //    for (auto& topVal : topArray)
        //    {
        //        Config tp;
        //        jsonToConfig(topVal, tp);
        //        project.topology.emplace_back(tp);
        //    }
        //}
    }

}