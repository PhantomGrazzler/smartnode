#pragma once

// Disable MSVC warning 4265 for boost headers. The following warning is generated:
// warning C4265: 'boost::exception_detail::error_info_container': class has virtual functions, but
// destructor is not virtual.
//
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4265 )
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/error.hpp>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

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

    void OnAccept( boost::beast::error_code ec, boost::asio::ip::tcp::socket socket );

private: // data
    boost::asio::io_context& m_ioc;
    boost::asio::ip::tcp::acceptor m_acceptor;
    const std::string m_serverName;
    const std::shared_ptr<MessageEngine> m_pMsgEngine;
};

} // namespace sns
