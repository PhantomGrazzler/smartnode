#include "Listener.hpp"
#include "Session.hpp"
#include "ConsolePrinter.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <memory>

namespace sns
{

Listener::Listener(
    boost::asio::io_context& ioc,
    const std::string& serverName,
    const std::shared_ptr<MessageEngine>& pMsgEngine )
    : m_ioc( ioc )
    , m_acceptor( ioc )
    , m_serverName( serverName )
    , m_pMsgEngine( pMsgEngine )
{}

void Listener::Listen( const boost::asio::ip::tcp::endpoint& endpoint )
{
    boost::beast::error_code ec;

    // Open the acceptor
    m_acceptor.open( endpoint.protocol(), ec );
    if ( ec )
    {
        return PrintError( "open: ", ec.message() );
    }

    // Allow address reuse
    m_acceptor.set_option( boost::asio::socket_base::reuse_address( true ), ec );
    if ( ec )
    {
        return PrintError( "set_option: ", ec.message() );
    }

    // Bind to the server address
    m_acceptor.bind( endpoint, ec );
    if ( ec )
    {
        return PrintError( "bind: ", ec.message() );
    }

    // Start listening for connections
    m_acceptor.listen( boost::asio::socket_base::max_listen_connections, ec );
    if ( ec )
    {
        return PrintError( "listen: ", ec.message() );
    }
}

void Listener::Run()
{
    DoAccept();
}

void Listener::DoAccept()
{
    // The new connection gets its own strand
    m_acceptor.async_accept(
        boost::asio::make_strand( m_ioc ),
        boost::beast::bind_front_handler( &Listener::OnAccept, shared_from_this() ) );
}

void Listener::OnAccept( boost::beast::error_code ec, boost::asio::ip::tcp::socket socket )
{
    if ( ec )
    {
        PrintError( "OnAccept: ", ec.message() );
    }
    else
    {
        // clang-format off
        PrintInfo(
            "New connection from ",
            socket.remote_endpoint().address(), ':', socket.remote_endpoint().port(),
            " bound to ",
            socket.local_endpoint().address(), ':', socket.local_endpoint().port() );
        // clang-format on

        // Create the session and run it
        std::make_shared<Session>( std::move( socket ), m_serverName, m_pMsgEngine )->Run();
    }

    // Accept another connection
    DoAccept();
}

} // namespace sns
