#include "Listener.h"
#include "Session.h"
#include "FailPrinters.h"

// Third-party
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

// Standard library
#include <iostream>
#include <memory>

namespace sns
{

Listener::Listener(
    boost::asio::io_context& ioc,
    boost::asio::ip::tcp::endpoint endpoint,
    const std::string& serverName )
    : m_ioc( ioc )
    , m_acceptor( ioc )
    , m_serverName( serverName )
{
    boost::beast::error_code ec;

    // Open the acceptor
    m_acceptor.open( endpoint.protocol(), ec );
    if( ec )
    {
        fail( ec, "open" );
        return;
    }

    // Allow address reuse
    m_acceptor.set_option( boost::asio::socket_base::reuse_address( true ), ec );
    if( ec )
    {
        fail( ec, "set_option" );
        return;
    }

    // Bind to the server address
    m_acceptor.bind( endpoint, ec );
    if( ec )
    {
        fail( ec, "bind" );
        return;
    }

    // Start listening for connections
    m_acceptor.listen(
        boost::asio::socket_base::max_listen_connections, ec );
    if( ec )
    {
        fail( ec, "listen" );
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
        fail( ec, "accept" );
    }
    else
    {
        std::cout << "New connection from " << socket.remote_endpoint().address() << ":" << socket.remote_endpoint().port()
            << " bound to " << socket.local_endpoint().address() << ":" << socket.local_endpoint().port() << "\n";

        // Create the session and run it
        std::make_shared<Session>( std::move( socket ), m_serverName )->Run();
    }

    // Accept another connection
    DoAccept();
}

}
