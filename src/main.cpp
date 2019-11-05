#include "Session.h"
#include "Listener.h"
#include "FailPrinters.h"

// Third-party
#include <boost/beast/core.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>

// Standard library
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

//------------------------------------------------------------------------------
const std::string programName( "smartnode-server" );

int main( int argc, char* argv[] )
{
    // Check command line arguments.
    if( argc != 3 )
    {
        std::cerr << "\nProvided " << (argc - 1) << " arguments, expected 2.\n\n";
        std::cerr << "Usage: " << programName << " <address> <port>\n"
            << "Example:\n"
            << "    " << programName << "127.0.0.1 8080\n";
        return EXIT_FAILURE;
    }

    boost::system::error_code ec;
    auto const address = boost::asio::ip::make_address( argv[1], ec );
    auto const port = static_cast<unsigned short>(std::atoi( argv[2] ));

    if( ec )
    {
        sns::fail( ec, "Failed to parse address" );
        return EXIT_FAILURE;
    }

    // The io_context is required for all I/O
    boost::asio::io_context ioc{};

    // Create and launch a listening port
    std::make_shared<sns::Listener>( ioc, boost::asio::ip::tcp::endpoint{ address, port }, programName )->Run();

    // Run the I/O service on a single thread
    std::cout << "Starting websocket server on " << address << ":" << port << '\n';
    ioc.run();

    return EXIT_SUCCESS;
}
