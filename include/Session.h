// This code is essentially the same as the asynchronous websocket server example from the boost::beast documentation.
// See https://www.boost.org/doc/libs/1_70_0/libs/beast/example/websocket/server/async/websocket_server_async.cpp

// Third-party
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

// Standard library
#include <memory>

namespace sns
{

class Session : public std::enable_shared_from_this<Session>
{
public:
    // Take ownership of the socket
    explicit Session(
        boost::asio::ip::tcp::socket&& socket,
        const std::string& serverName);

    // Start the asynchronous operation
    void Run();

    void OnAccept( boost::beast::error_code ec );

    void DoRead();

    void OnRead(
        boost::beast::error_code ec,
        std::size_t bytes_transferred );

    void OnWrite(
        const boost::beast::error_code& ec,
        std::size_t bytes_transferred );

private:
    boost::beast::websocket::stream<boost::beast::tcp_stream> m_ws;
    boost::beast::multi_buffer m_buffer;
    const std::string m_serverName;
};

} 
