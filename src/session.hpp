//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket client, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void fail( beast::error_code ec, char const* what )
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string send_msg_;
    std::function<void( std::string )> read_callback_;

public:
    // Resolver and socket require an io_context
    explicit session( net::io_context& ioc )
        : resolver_( net::make_strand( ioc ) )
        , ws_( net::make_strand( ioc ) )
    {}

    void register_read_callback( std::function<void( std::string )> callback )
    {
        read_callback_ = callback;
    }

    // Start the asynchronous operation
    void run( const std::string& host, const std::string& port )
    {
        // Save these for later
        host_ = host;

        // Look up the domain name
        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler( &session::on_resolve, shared_from_this() ) );
    }

    void on_resolve( beast::error_code ec, tcp::resolver::results_type results )
    {
        if ( ec )
            return fail( ec, "resolve" );

        // Set the timeout for the operation
        beast::get_lowest_layer( ws_ ).expires_after( std::chrono::seconds( 30 ) );

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer( ws_ ).async_connect(
            results,
            beast::bind_front_handler( &session::on_connect, shared_from_this() ) );
    }

    void on_connect( beast::error_code ec, tcp::resolver::results_type::endpoint_type )
    {
        if ( ec )
            return fail( ec, "connect" );

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer( ws_ ).expires_never();

        // Set suggested timeout settings for the websocket
        ws_.set_option( websocket::stream_base::timeout::suggested( beast::role_type::client ) );

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option( websocket::stream_base::decorator( []( websocket::request_type& req ) {
            req.set(
                http::field::user_agent,
                std::string( BOOST_BEAST_VERSION_STRING ) + " websocket-client-async" );
        } ) );

        // Perform the websocket handshake
        ws_.async_handshake(
            host_,
            "/",
            beast::bind_front_handler( &session::on_handshake, shared_from_this() ) );
    }

    void async_read()
    {
        ws_.async_read(
            buffer_,
            beast::bind_front_handler( &session::on_read, shared_from_this() ) );
    }

    void async_write( const std::string& msg )
    {
        send_msg_ = msg;
        ws_.async_write(
            net::buffer( send_msg_ ),
            beast::bind_front_handler( &session::on_write, shared_from_this() ) );
    }

    void on_handshake( beast::error_code ec )
    {
        if ( ec )
            return fail( ec, "handshake" );

        async_read();
    }

    void on_write( beast::error_code ec, std::size_t bytes_transferred )
    {
        if ( ec )
            return fail( ec, "write" );

        std::cout << "Wrote " << bytes_transferred << " bytes\n";
    }

    void on_read( beast::error_code ec, std::size_t bytes_transferred )
    {
        if ( ec )
            return fail( ec, "read" );

        std::cout << "Read " << bytes_transferred << " bytes\n";
        std::ostringstream oss;
        oss << beast::make_printable( buffer_.data() );

        if ( read_callback_ )
        {
            read_callback_( oss.str() );
        }

        buffer_.consume( buffer_.size() );

        async_read();
    }

    void async_close()
    {
        // Close the WebSocket connection
        ws_.async_close(
            websocket::close_code::normal,
            beast::bind_front_handler( &session::on_close, shared_from_this() ) );
    }

    void on_close( beast::error_code ec )
    {
        if ( ec )
            return fail( ec, "close" );

        std::cout << "Session closed\n";
    }
};
