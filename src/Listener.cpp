#include "Listener.hpp"
#include "Session.hpp"
#include "ConsolePrinter.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <iostream>
#include <memory>

namespace sns
{

Listener::Listener(
    boost::asio::io_context& ioc,
    boost::asio::ip::tcp::endpoint endpoint,
    const std::string& serverName,
    const std::shared_ptr<MessageEngine>& pMsgEngine )
    : m_ioc( ioc )
    , m_acceptor( ioc )
    , m_serverName( serverName )
    , m_pMsgEngine( pMsgEngine )
{
    boost::beast::error_code ec;

    // Open the acceptor
    m_acceptor.open( endpoint.protocol(), ec );
    if( ec )
    {
        PrintError( "open: ", ec.message() );
        return;
    }

    // Allow address reuse
    m_acceptor.set_option( boost::asio::socket_base::reuse_address( true ), ec );
    if( ec )
    {
        PrintError( "set_option: ", ec.message() );
        return;
    }

    // Bind to the server address
    m_acceptor.bind( endpoint, ec );
    if( ec )
    {
        PrintError( "bind: ", ec.message() );
        return;
    }

    // Start listening for connections
    m_acceptor.listen(
        boost::asio::socket_base::max_listen_connections, ec );
    if( ec )
    {
        PrintError( "listen: ", ec.message() );
        return;
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
        boost::beast::bind_front_handler(
            &Listener::OnAccept,
            shared_from_this() ) );
}

void Listener::OnAccept(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::socket socket )
{
    if( ec )
    {
        PrintError( "OnAccept: ", ec.message() );
    }
    else
    {
        PrintInfo( "New connection from ",
            socket.remote_endpoint().address(), ':',
            socket.remote_endpoint().port(),
            " bound to ",
            socket.local_endpoint().address(), ':',
            socket.local_endpoint().port() );

        // Create the session and run it
        std::make_shared<Session>( std::move( socket ), m_serverName, m_pMsgEngine )->Run();
    }

    // Accept another connection
    DoAccept();
}

}
