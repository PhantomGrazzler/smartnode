// This code is essentially the same as the asynchronous websocket server example from the boost::beast documentation.
// See https://www.boost.org/doc/libs/1_70_0/libs/beast/example/websocket/server/async/websocket_server_async.cpp

#include "Connection.h"

// Third-party
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

// Standard library
#include <memory>
#include <variant>

namespace sns
{

class MessageEngine;

class Session final : public std::enable_shared_from_this<Session>
{
public:
    // Take ownership of the socket
    explicit Session(
        boost::asio::ip::tcp::socket&& socket,
        const std::string& serverName,
        const std::shared_ptr<MessageEngine>& pMsgEngine );

    // Start the asynchronous operation
    void Run();

    /*!
        @brief Synchronously send the provided message to the remote peer.
        @param[in] message The message to be sent to the remote peer.
     */
    void SendMessage( const std::string& message );

    /*!
        @brief TODO
     */
    template<typename T>
    void SetPeerId( const T id );

private:
    void OnAccept( boost::beast::error_code ec );

    void DoRead();

    void OnRead(
        boost::beast::error_code ec,
        std::size_t bytes_transferred );

private:
    boost::beast::websocket::stream<boost::beast::tcp_stream> m_ws;
    boost::beast::multi_buffer m_buffer;
    const std::string m_serverName;
    const std::shared_ptr<MessageEngine> m_pMsgEngine;
    boost::asio::ip::address m_peerAddress;
    unsigned short m_peerPort;
    std::variant<UIId, NodeId> m_peerId;
};

} 
