#include "listener.hpp"
#include "session.hpp"
#include "console_printer.hpp"
#include "logger.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <memory>

namespace sn
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
        Log( spdlog::level::err, "Failed to open: {}", ec.message() );
        return PrintError( "open: ", ec.message() );
    }

    // Allow address reuse
    m_acceptor.set_option( boost::asio::socket_base::reuse_address( true ), ec );
    if ( ec )
    {
        Log( spdlog::level::err, "Failed to set_option: {}", ec.message() );
        return PrintError( "set_option: ", ec.message() );
    }

    // Bind to the server address
    m_acceptor.bind( endpoint, ec );
    if ( ec )
    {
        Log( spdlog::level::err, "Failed to bind: {}", ec.message() );
        return PrintError( "bind: ", ec.message() );
    }

    // Start listening for connections
    m_acceptor.listen( boost::asio::socket_base::max_listen_connections, ec );
    if ( ec )
    {
        Log( spdlog::level::err, "Failed to listen: {}", ec.message() );
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
        Log( spdlog::level::debug,
             "New connection from {}:{} bound to {}:{}",
             socket.remote_endpoint().address().to_string(),
             socket.remote_endpoint().port(),
             socket.local_endpoint().address().to_string(),
             socket.local_endpoint().port() );

        // Create the session and run it
        std::make_shared<Session>( std::move( socket ), m_serverName, m_pMsgEngine )->Run();
    }

    // Accept another connection
    DoAccept();
}

} // namespace sn
