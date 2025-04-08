#include "restClient.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <string>

namespace restClient
{
    void parseUrl(const std::string& url,
                  std::string &protocol,
                  std::string &host,
                  std::string &port,
                  std::string &path)
    {
        // Very simple parsing (doesn't handle all edge cases)
        // Example: "http://127.0.0.1:8080/getStatus"
        // protocol = "http"
        // host     = "127.0.0.1"
        // port     = "8080"
        // path     = "/getStatus"

        protocol.clear();
        host.clear();
        port.clear();
        path.clear();

        const std::string protEnd = "://";
        auto protPos = url.find(protEnd);
        if (protPos != std::string::npos)
        {
            protocol = url.substr(0, protPos);
        }

        auto hostStart = (protPos == std::string::npos) ? 0 : protPos + protEnd.size();
        auto portPos = url.find(':', hostStart);
        auto pathPos = url.find('/', hostStart);

        // If we found a ':' and it's before a '/', treat that as the port separator
        if (portPos != std::string::npos && (pathPos == std::string::npos || portPos < pathPos))
        {
            host = url.substr(hostStart, portPos - hostStart);
            if (pathPos != std::string::npos)
            {
                port = url.substr(portPos + 1, pathPos - (portPos + 1));
            }
            else
            {
                port = url.substr(portPos + 1);
            }
        }
        else
        {
            // No port was specified
            if (pathPos != std::string::npos)
            {
                host = url.substr(hostStart, pathPos - hostStart);
            }
            else
            {
                host = url.substr(hostStart);
            }
        }

        if (pathPos != std::string::npos)
        {
            path = url.substr(pathPos);
        }

        // Default port if none found
        if (port.empty()) port = "80";
    }

    std::string doGet(const std::string& url)
    {
        try
        {
            std::string protocol, host, port, path;
            parseUrl(url, protocol, host, port, path);

            // I/O context
            boost::asio::io_context ioc;
            // Resolver
            boost::asio::ip::tcp::resolver resolver(ioc);
            // Socket
            boost::asio::ip::tcp::socket socket(ioc);

            // Resolve the host
            auto const results = resolver.resolve(host, port);
            // Connect
            boost::asio::connect(socket, results.begin(), results.end());

            // Create HTTP request
            boost::beast::http::request<boost::beast::http::string_body> req{
                boost::beast::http::verb::get, path.empty() ? "/" : path, 11 };
            req.set(boost::beast::http::field::host, host);
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            // Send request
            boost::beast::http::write(socket, req);

            // Receive response
            boost::beast::flat_buffer buffer;
            boost::beast::http::response<boost::beast::http::string_body> res;
            boost::beast::http::read(socket, buffer, res);

            // Gracefully close the connection
            boost::system::error_code ec;
            socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            // Return response body
            return res.body();
        }
        catch(...)
        {
            return "Error: Unable to complete GET request.";
        }
    }
}
