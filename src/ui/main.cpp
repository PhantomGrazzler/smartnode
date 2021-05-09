#include "session.hpp"

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <boost/asio/io_context.hpp>

#include <iostream>
#include <memory>
#include <thread>

// TODO:
//  -> Deal with failing to connect (e.g. server not running)
//  -> Fix bug in populating server response array
//  -> Find a better way to read/populate imgui text boxes
//  -> Make UI Id input read-only until connection completes
//  -> Check out IP address input (https://github.com/ocornut/imgui/issues/388)
//  -> Add spdlog
//  -> Use messaging lib for building and parsing

constexpr auto decimalInputOptions = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal |
                                     ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsNoBlank;
constexpr auto textInputOptions = ImGuiInputTextFlags_::ImGuiInputTextFlags_AlwaysInsertMode;
constexpr auto textOutputOptions = ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly;
const ImVec4 red( 1, 0, 0, 1 );
const ImVec4 green( 0, 1, 0, 1 );

class UIState
{
public:
    bool IsConnected() const
    {
        return connected;
    }

    void Connected()
    {
        connected = true;
        textColour = green;
        connectionText = "Connected";
    }

    void Disconnected()
    {
        connected = false;
        textColour = red;
        connectionText = "Disconnected";
    }

    ImVec4 ConnectionColour() const
    {
        return textColour;
    }

    const std::string& ConnectionText() const
    {
        return connectionText;
    }

private:
    bool connected = false;
    ImVec4 textColour = red;
    std::string connectionText = "Disconnected";
};

int main()
{
    UIState state;

    boost::asio::io_context ioc;
    auto work_guard = boost::asio::make_work_guard( ioc );
    std::thread commsThread( [&ioc]() { ioc.run(); } );
    std::shared_ptr<session> pSession;

    sf::RenderWindow window( sf::VideoMode( 800, 600 ), "SmartNodeUI" );
    window.setFramerateLimit( 60 );
    ImGui::SFML::Init( window );
    sf::Clock deltaClock;

    std::uint8_t v[4] = { 127, 0, 0, 1 };
    int port = 8080;
    char msg[1024] = {};
    char msg_out[1024] = {};
    std::uint32_t uiId = 1;

    while ( window.isOpen() )
    {
        sf::Event event{};

        while ( window.pollEvent( event ) )
        {
            ImGui::SFML::ProcessEvent( event );

            if ( event.type == sf::Event::Closed )
            {
                window.close();
                break;
            }
        }

        const auto timeElapsed = deltaClock.restart();
        ImGui::SFML::Update( window, timeElapsed );

        ImGui::Begin( "Control" );
        if ( ImGui::Button( "Exit", { 50, 20 } ) )
        {
            window.close();
        }
        ImGui::Text( "UI ID: " );
        ImGui::SameLine();
        ImGui::InputScalarN(
            "##ui_id_input",
            ImGuiDataType_::ImGuiDataType_U32,
            &uiId,
            1,
            nullptr,
            nullptr,
            "%d",
            decimalInputOptions );
        ImGui::End();

        ImGui::Begin( "State" );
        ImGui::TextColored( state.ConnectionColour(), "%s", state.ConnectionText().c_str() );
        ImGui::End();

        ImGui::Begin( "Connectivity" );
        ImGui::Text( "IP Address: " );
        ImGui::SameLine();
        ImGui::InputScalarN(
            "##ip_input",
            ImGuiDataType_::ImGuiDataType_U8,
            v,
            4,
            nullptr,
            nullptr,
            "%d",
            decimalInputOptions );

        ImGui::Text( "Port: " );
        ImGui::SameLine();
        ImGui::InputScalar(
            "##port_input",
            ImGuiDataType_::ImGuiDataType_U16,
            &port,
            nullptr,
            nullptr,
            "%d",
            decimalInputOptions );

        if ( !state.IsConnected() )
        {
            if ( ImGui::Button( "Connect" ) )
            {
                std::ostringstream ipAddress;
                ipAddress << static_cast<int>( v[0] ) << '.' << static_cast<int>( v[1] ) << '.'
                          << static_cast<int>( v[2] ) << '.' << static_cast<int>( v[3] );

                std::cout << "Connecting to " << ipAddress.str() << ':' << port << '\n';

                pSession = std::make_shared<session>( ioc );
                pSession->register_connect_callback( [uiId, pSession]() {
                    std::ostringstream oss;
                    oss << R"({"MsgType": "ui_connect", "UIId": )" << uiId << '}';
                    pSession->async_write( oss.str() );
                } );
                pSession->register_read_callback( [&msg_out]( std::string readMsg ) {
                    std::size_t index = 0;
                    while ( index < readMsg.size() )
                    {
                        msg_out[index] = readMsg[index];
                        ++index;
                    }
                } );
                pSession->run( ipAddress.str(), std::to_string( port ) );

                state.Connected();
            }
        }
        else
        {
            if ( ImGui::Button( "Disconnect" ) )
            {
                pSession->async_close();
                std::cout << "Disconnect\n";

                state.Disconnected();
            }
        }

        ImGui::Text( "Message: " );
        ImGui::SameLine();
        ImGui::InputTextMultiline(
            "##msg_input",
            msg,
            IM_ARRAYSIZE( msg ),
            { 250, 200 },
            textInputOptions );

        if ( state.IsConnected() )
        {
            ImGui::SameLine();
            if ( ImGui::Button( "Send Message" ) )
            {
                pSession->async_write( msg );
            }
        }

        ImGui::Text( "Server Response: " );
        ImGui::SameLine();
        ImGui::InputTextMultiline(
            "##msg_output",
            msg_out,
            IM_ARRAYSIZE( msg_out ),
            { 250, 200 },
            textOutputOptions );

        ImGui::End();

        window.clear();
        ImGui::SFML::Render( window );
        window.display();
    }

    if ( pSession )
    {
        std::cout << "Disconnecting\n";
        pSession->async_close();
    }

    work_guard.reset();
    ioc.stop();

    if ( commsThread.joinable() )
    {
        std::cout << "Joining comms thread...\n";
        commsThread.join();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
