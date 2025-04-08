#pragma once
// No changes in this file
// ...existing code...
#include <string>

namespace wsClient
{
    // Synchronously connects to a WebSocket server
    bool connect(const std::string& url);

    // Sends a message through the connected WebSocket
    bool send(const std::string& message);

    // Closes the WebSocket connection
    void close();

    // New function to retrieve the latest response
    std::string getLastResponse();
}
