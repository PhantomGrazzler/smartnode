#include "Session.h"
#include "Listener.h"
#include "FailPrinters.h"
#include "MessageEngine.h"

// Third-party
#include <boost/beast/core.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <rang.hpp>

// Standard library
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

//------------------------------------------------------------------------------
const std::string programName( "smartnode-server" );

int main( int argc, char* argv[] )
{
    std::string addressStr;
    std::string portStr;

    // Check command line arguments.
    if( argc != 3 )
    {
        std::cerr << rang::fg::yellow << "\nProvided " << (argc - 1) << " arguments, expected 2.\n\n";
        std::cerr << rang::fg::reset << "Usage: " << programName << " <address> <port>\n"
            << "Example:\n"
            << "    " << programName << " 127.0.0.1 8080\n\n";

        std::cerr << rang::fg::yellow << "Starting assuming server address of 127.0.0.1 and port 8080.\n\n" << rang::fg::reset;
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

    if( ec )
    {
        sns::fail( ec, "Failed to parse address" );
        return EXIT_FAILURE;
    }

    const auto pMsgEngine = std::make_shared<sns::MessageEngine>();

    // The io_context is required for all I/O
    boost::asio::io_context ioc{};

    // Create and launch a listening port
    std::make_shared<sns::Listener>( ioc, boost::asio::ip::tcp::endpoint{ address, port }, programName, pMsgEngine )->Run();

    // Run the I/O service on a single thread
    std::cout << rang::fg::cyan << "Starting websocket server on " << address << ":" << port << '\n' << rang::fg::reset;
    ioc.run();

    return EXIT_SUCCESS;
}
