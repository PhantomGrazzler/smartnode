#include "Session.hpp"
#include "ConsolePrinter.hpp"
#include "MessageEngine.hpp"

#include <nlohmann/json.hpp>

#include <iostream>

namespace sns
{

Session::Session(
    boost::asio::ip::tcp::socket&& socket,
    const std::string& serverName,
    const std::shared_ptr<MessageEngine>& pMsgEngine )
    : m_ws( std::move( socket ) )
    , m_buffer()
    , m_serverName( serverName )
    , m_pMsgEngine( pMsgEngine )
    , m_peerAddress()
    , m_peerPort()
{
    // Nothing to do here.
}

void Session::Run()
{
    // Set suggested timeout settings for the websocket
    m_ws.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(
            boost::beast::role_type::server ) );

    // Set a decorator to change the Server of the handshake
    const std::string serverName( m_serverName );
    m_ws.set_option( boost::beast::websocket::stream_base::decorator(
        [serverName]( boost::beast::websocket::response_type & res )
        {
            res.set( boost::beast::http::field::server,
                std::string( BOOST_BEAST_VERSION_STRING ) +
                " " + serverName );
        } ) );

    // Accept the websocket handshake
    m_ws.async_accept(
        boost::beast::bind_front_handler(
            &Session::OnAccept,
            shared_from_this() ) );
}

void Session::SendMessage( const std::string& message )
{
    boost::beast::error_code ec;
    m_ws.write( boost::asio::buffer( message ), ec );

    if( ec )
    {
        PrintError( "Failed to write message:\n", message, "\n\n" );
    }
}

std::variant<UIId, NodeId> Session::GetPeerId() const
{
    return m_peerId;
}

void Session::OnAccept( boost::beast::error_code ec )
{
    if( ec )
    {
        return PrintError( "OnAccept: ", ec.message() );
    }

    m_peerAddress = m_ws.next_layer().socket().remote_endpoint().address();
    m_peerPort = m_ws.next_layer().socket().remote_endpoint().port();

    // Read a message
    DoRead();
}

void Session::DoRead()
{
    // Read a message into our buffer
    m_ws.async_read(
        m_buffer,
        boost::beast::bind_front_handler(
            &Session::OnRead,
            shared_from_this() ) );
}

void Session::OnRead(
    boost::beast::error_code ec,
    std::size_t bytes_transferred )
{
    // This indicates that the session was closed
    if( ec == boost::beast::websocket::error::closed )
    {
        m_pMsgEngine->PeerDisconnected( shared_from_this() );

        return PrintInfo( m_peerAddress, ":", m_peerPort, " disconnected.\n" );;
    }
    else if( ec )
    {
        PrintError( "OnRead", ec.message() );
    }

    PrintInfo( "Received ", bytes_transferred, " bytes" );

    std::stringstream ss;
    ss << boost::beast::make_printable( m_buffer.data() );
    m_pMsgEngine->MessageReceived( weak_from_this(), ss.str() );

    // Clear the buffer
    m_buffer.consume( m_buffer.size() );

    // Queue up another read
    DoRead();
}

}
