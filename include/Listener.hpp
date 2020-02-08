#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/error.hpp>

#include <memory>
#include <string>

namespace sns
{

class MessageEngine;

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener>
{
public:
    Listener(
        boost::asio::io_context& ioc,
        const std::string& serverName,
        const std::shared_ptr<MessageEngine>& pMsgEngine );

    /*!
        @brief Start listening for incoming connections on the provided
               endpoint.
        @param[in] endpoint The endpoint to listen on.
     */
    void Listen( const boost::asio::ip::tcp::endpoint& endpoint );

    /*!
        @brief Start accepting incoming connections.
     */
    void Run();

private: // methods
    void DoAccept();

    void OnAccept(
        boost::beast::error_code ec,
        boost::asio::ip::tcp::socket socket );

private: // data
    boost::asio::io_context& m_ioc;
    boost::asio::ip::tcp::acceptor m_acceptor;
    const std::string m_serverName;
    const std::shared_ptr<MessageEngine> m_pMsgEngine;
};

}
