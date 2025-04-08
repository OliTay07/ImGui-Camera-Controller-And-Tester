#pragma once

#include <string>

namespace restClient
{
    void parseUrl(const std::string& url,
                  std::string &protocol,
                  std::string &host,
                  std::string &port,
                  std::string &path);

    std::string doGet(const std::string& url);
}
