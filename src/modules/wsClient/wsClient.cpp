// ...existing includes...
#include "wsClient.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <spdlog/spdlog.h>

namespace
{
    // Simple URL parsing (adapted from restClient)
    void parseUrl(const std::string& url,
        std::string& protocol,
        std::string& host,
        std::string& port,
        std::string& path)
    {
        protocol.clear();
        host.clear();
        port.clear();
        path.clear();

        const std::string protEnd = "://";
        auto protPos = url.find(protEnd);
        if (protPos != std::string::npos)
            protocol = url.substr(0, protPos);

        auto hostStart = (protPos == std::string::npos) ? 0 : protPos + protEnd.size();
        auto portPos = url.find(':', hostStart);
        auto pathPos = url.find('/', hostStart);

        if (portPos != std::string::npos && (pathPos == std::string::npos || portPos < pathPos))
        {
            host = url.substr(hostStart, portPos - hostStart);
            if (pathPos != std::string::npos)
                port = url.substr(portPos + 1, pathPos - (portPos + 1));
            else
                port = url.substr(portPos + 1);
        }
        else
        {
            if (pathPos != std::string::npos)
                host = url.substr(hostStart, pathPos - hostStart);
            else
                host = url.substr(hostStart);
        }

        if (pathPos != std::string::npos)
            path = url.substr(pathPos);

        // Default port for ws/wss
        if (port.empty())
            port = (protocol == "wss") ? "443" : "80";
        if (path.empty())
            path = "/";
    }
}

namespace wsClient
{
    static boost::asio::io_context ioc;
    static std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws;
    static std::string lastResponse; // Used to store the most recent response

    bool connect(const std::string& url)
    {
        auto logger = spdlog::get("IMGUI_LOGGER");

        std::string protocol, host, port, path;
        parseUrl(url, protocol, host, port, path);

        // Make sure host/port are valid
        if (host.empty() || port.empty())
        {
            if (logger) logger->error("wsClient::connect failed: empty host or port");
            return false;
        }

        try
        {
            // Reset the io_context in case of previous runs
            ioc.restart();

            // Resolve and connect
            boost::asio::ip::tcp::resolver resolver(ioc);
            auto const results = resolver.resolve(host, port);

            ws = std::make_unique<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>(ioc);
            boost::asio::connect(ws->next_layer(), results.begin(), results.end());

            // Perform WebSocket handshake
            ws->handshake(host, path);
            return true;
        }
        catch (const std::exception& e)
        {
            if (logger) logger->error("wsClient::connect exception: {}", e.what());
            ws.reset();
            return false;
        }
        catch (...)
        {
            if (logger) logger->error("wsClient::connect unknown exception");
            ws.reset();
            return false;
        }
    }

    bool send(const std::string& message)
    {
        if (!ws)
            return false;

        try
        {
            ws->write(boost::asio::buffer(message));
            // Immediately read the server's response
            boost::beast::flat_buffer buffer;
            ws->read(buffer);
            lastResponse = boost::beast::buffers_to_string(buffer.data());
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void close()
    {
        if (!ws)
            return;
        try
        {
            ws->close(boost::beast::websocket::close_code::normal);
        }
        catch (...)
        {
            // ignore any close errors
        }
        ws.reset();
    }

    std::string getLastResponse()
    {
        return lastResponse;
    }
}
