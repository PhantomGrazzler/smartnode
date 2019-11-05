#include "Session.h"
#include "FailPrinters.h"

// Standard library
#include <iostream>

namespace sns
{

Session::Session(
    boost::asio::ip::tcp::socket&& socket,
    const std::string& serverName )
    : m_ws( std::move( socket ) )
    , m_buffer()
    , m_serverName(serverName)
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

void Session::OnAccept( boost::beast::error_code ec )
{
    if( ec )
    {
        return fail( ec, "OnAccept" );
    }

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
        // TODO_IZG: Figure out why this throws an exception on disconnect
        //
        // std::cout << m_ws.next_layer().socket().remote_endpoint().address() << ":" << m_ws.next_layer().socket().remote_endpoint().port() << " disconnected.\n";

        return;
    }
    else if( ec )
    {
        fail( ec, "read" );
    }

    std::cout << "Received " << bytes_transferred << " bytes: " << boost::beast::make_printable( m_buffer.data() ) << "\n";

    // Echo the message
    m_ws.text( m_ws.got_text() );
    m_ws.async_write(
        m_buffer.data(),
        boost::beast::bind_front_handler(
            &Session::OnWrite,
            shared_from_this() ) );
}

void Session::OnWrite(
    const boost::beast::error_code& ec,
    std::size_t bytes_transferred )
{
    boost::ignore_unused( bytes_transferred );

    if( ec )
    {
        return fail( ec, "write" );
    }

    // Clear the buffer
    m_buffer.consume( m_buffer.size() );

    // Do another read
    DoRead();
}

}
