
#include "Listener.hpp"
#include "ConsolePrinter.hpp"
#include "MessageEngine.hpp"

#include <boost/beast/core.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cstdlib>
#include <memory>
#include <string>

//------------------------------------------------------------------------------
const std::string programName( "smartnode-server" );

int main( int argc, char* argv[] )
{
    std::string addressStr;
    std::string portStr;

    // Check command line arguments.
    if ( argc != 3 )
    {
        sns::PrintWarning( "\nProvided ", ( argc - 1 ), " arguments, expected 2." );
        sns::PrintWarning( "Starting assuming server address of 127.0.0.1 and port 8080." );
        // clang-format off
        sns::PrintInfo(
            "\nUsage: ", programName, " <address> <port>\n",
            "Example:\n",
            "    ", programName, " 127.0.0.1 8080\n" );
        // clang-format on

        addressStr = "127.0.0.1";
        portStr = "8080";
    }
    else
    {
        addressStr = argv[1];
        portStr = argv[2];
    }

    boost::system::error_code ec;
    auto const address = boost::asio::ip::make_address( addressStr, ec );
    auto const port = static_cast<unsigned short>( std::atoi( portStr.c_str() ) );

    if ( ec )
    {
        sns::PrintError( "Failed to parse address: ", ec.message() );
        return EXIT_FAILURE;
    }

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // Create and launch a listening port
    const auto pListener =
        std::make_shared<sns::Listener>( ioc, programName, std::make_shared<sns::MessageEngine>() );
    pListener->Listen( boost::asio::ip::tcp::endpoint{ address, port } );
    pListener->Run();

    // Run the I/O service on a single thread (this one).
    sns::PrintInfo( "Starting websocket server on ", address, ':', port, '\n' );
    ioc.run();

    return EXIT_SUCCESS;
}
