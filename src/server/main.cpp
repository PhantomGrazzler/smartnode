
#include "Listener.hpp"
#include "console_printer.hpp"
#include "message_engine.hpp"
#include "logger.hpp"

#include <boost/beast/core.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

//------------------------------------------------------------------------------
const std::string programName( "smartnode-server" );
constexpr auto maxLogSize_bytes = 1024 * 1024 * 5; // 5 MiB
const auto numLogFiles = 20;
const bool rotateOnAppStart = true;
const std::string exitMessage( "Ctrl-C caught, exiting program." );
std::atomic<bool> shouldKeepRunning( true );

#ifdef _WIN32
BOOL WINAPI CtrlHandler( DWORD fdwCtrlType )
{
    switch ( fdwCtrlType )
    {
    // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
        sns::PrintInfo( exitMessage );
        sns::Log( spdlog::level::info, exitMessage );
        shouldKeepRunning.store( false, std::memory_order_relaxed );

        return TRUE;

    default:
        return FALSE;
    }
}

bool InstallCtrlHandler()
{
    return SetConsoleCtrlHandler( CtrlHandler, TRUE );
}
#endif

int main( int argc, char* argv[] )
{
#ifdef _WIN32
    if ( !InstallCtrlHandler() )
    {
        sns::PrintError( "Failed to install Ctrl-C handler!" );
    }
#endif

    try
    {
        spdlog::rotating_logger_mt(
            sns::loggerName,
            "logs/debug.log",
            maxLogSize_bytes,
            numLogFiles,
            rotateOnAppStart );
        spdlog::flush_every( std::chrono::seconds( 5 ) );
    }
    catch ( const spdlog::spdlog_ex& ex )
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::string addressStr;
    std::string portStr;

    // Check command line arguments.
    if ( argc != 3 )
    {
        // clang-format off
        sns::PrintInfo(
            "Usage: ", programName, " <address> <port>\n",
            "Example:\n",
            "    ", programName, " 127.0.0.1 8080\n" );
        // clang-format on

        addressStr = "127.0.0.1";
        portStr = "8080";

        sns::PrintWarning( "Starting assuming server address of 127.0.0.1 and port 8080." );
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
    sns::Log( spdlog::level::debug, "Starting with address {} and port {}", addressStr, portStr );
    sns::PrintInfo( "Starting websocket server on ", address, ':', port );
    sns::PrintInfo( "Press CTRL+C to exit" );

    std::thread serverThread( [&ioc]() { ioc.run(); } );

    while ( shouldKeepRunning.load() )
    {
        // We wait here for the user to exit.
    }

    ioc.stop();

    if ( serverThread.joinable() )
    {
        serverThread.join();
    }

    sns::Log( spdlog::level::debug, "Exiting ", programName );
    spdlog::drop_all();
    spdlog::shutdown();

    return EXIT_SUCCESS;
}
