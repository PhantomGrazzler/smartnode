#include "session.hpp"
#include "message_builder.hpp"
#include "parser.hpp"

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
//  -> Find a better way to read/populate imgui text boxes
//  -> Make UI Id input read-only until connection completes
//  -> Check out IP address input (https://github.com/ocornut/imgui/issues/388)
//  -> Add spdlog

constexpr auto decimalInputOptions = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal |
                                     ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsNoBlank;
constexpr auto textInputOptions = ImGuiInputTextFlags_::ImGuiInputTextFlags_AlwaysInsertMode;
constexpr auto textOutputOptions = ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly;
const ImVec4 red( 1, 0, 0, 1 );
const ImVec4 green( 0, 1, 0, 1 );

const sf::Color sf_red( 255, 0, 0 );
const sf::Color sf_green( 0, 255, 0 );

class ConnectionState
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

class NodeStates
{
private:
    std::vector<sn::Node> m_nodes;
    std::mutex m_mutex;

public:
    void Reset()
    {
        std::lock_guard l( m_mutex );
        m_nodes.clear();
    }

    void SetNodeStates( const std::vector<sn::Node>& nodes )
    {
        std::lock_guard l( m_mutex );
        m_nodes = nodes;
    }

    void NodeConnected( const sn::Node& node )
    {
        std::lock_guard l( m_mutex );

        auto nodeIter =
            std::find_if( begin( m_nodes ), end( m_nodes ), [node]( const auto& existingNode ) {
                return node.id == existingNode.id;
            } );

        if ( nodeIter == end( m_nodes ) )
        {
            m_nodes.push_back( node );
        }
        else
        {
            *nodeIter = node;
        }
    }

    void NodeDisconnected( const sn::Node& node )
    {
        std::lock_guard l( m_mutex );

        auto nodeIter =
            std::find_if( begin( m_nodes ), end( m_nodes ), [node]( const auto& existingNode ) {
                return node.id == existingNode.id;
            } );

        if ( nodeIter != end( m_nodes ) )
        {
            m_nodes.erase( nodeIter );
        }
    }

    void NodeUpdated( const sn::Node& node )
    {
        std::lock_guard l( m_mutex );

        auto nodeIter =
            std::find_if( begin( m_nodes ), end( m_nodes ), [node]( const auto& existingNode ) {
                return node.id == existingNode.id;
            } );

        if ( nodeIter != end( m_nodes ) )
        {
            for ( const auto& io : node.io )
            {
                const auto& existingIoBegin = begin( ( *nodeIter ).io );
                const auto& existingIoEnd = end( ( *nodeIter ).io );
                auto existingIoIter =
                    std::find_if( existingIoBegin, existingIoEnd, [io]( const auto& existingIo ) {
                        return io.id == existingIo.id;
                    } );

                if ( existingIoIter != existingIoEnd )
                {
                    *existingIoIter = io;
                }
                else
                {
                    std::cout << "Update for unknown IO " << io.id << '\n';
                }
            }
        }
        else
        {
            std::cout << "Update for unknown Node " << node.id << '\n';
        }
    }

    void DrawNodes()
    {
        std::lock_guard l( m_mutex );
        const auto font_size = ImGui::GetFontSize();
        const sf::FloatRect float_rect( 0.0, 0.0, font_size, font_size );

        for ( const auto& node : m_nodes )
        {
            ImGui::Begin( sn::to_string( node.id ).c_str() );

            for ( const auto& io : node.io )
            {
                std::ostringstream oss;
                oss << io.type << ' ' << io.id;
                ImGui::Text( oss.str().c_str() );
                ImGui::SameLine();
                ImGui::Text( "Value: %d", io.value );

                if ( io.type == "di" || io.type == "do" )
                {
                    ImGui::SameLine();

                    if ( io.value == 0 )
                    {
                        ImGui::DrawRectFilled( float_rect, sf_red );
                    }
                    else
                    {
                        ImGui::DrawRectFilled( float_rect, sf_green );
                    }

                    ImGui::NewLine();
                }
                else if ( io.type == "ai" || io.type == "ao" )
                {
                    ImGui::SameLine();
                    const auto value = std::to_string( io.value );
                    ImGui::ProgressBar( io.value / 255.0, ImVec2( -1, 0 ), value.c_str() );
                }
            }

            ImGui::End();
        }
    }
};

int main()
{
    ConnectionState connection_state;
    NodeStates node_states;

    boost::asio::io_context ioc;
    auto work_guard = boost::asio::make_work_guard( ioc );
    std::thread commsThread( [&ioc]() { ioc.run(); } );
    std::shared_ptr<session> pSession;

    sf::RenderWindow window( sf::VideoMode( 1024, 768 ), "SmartNodeUI" );
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
        ImGui::TextColored(
            connection_state.ConnectionColour(),
            "%s",
            connection_state.ConnectionText().c_str() );
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

        if ( !connection_state.IsConnected() )
        {
            if ( ImGui::Button( "Connect" ) )
            {
                std::ostringstream ipAddress;
                ipAddress << static_cast<int>( v[0] ) << '.' << static_cast<int>( v[1] ) << '.'
                          << static_cast<int>( v[2] ) << '.' << static_cast<int>( v[3] );

                std::cout << "Connecting to " << ipAddress.str() << ':' << port << '\n';

                pSession = std::make_shared<session>( ioc );
                pSession->register_connect_callback( [uiId, pSession]() {
                    pSession->async_write( sn::BuildUiConnect( static_cast<sn::UIId>( uiId ) ) );
                } );
                pSession->register_read_callback( [&msg_out, &node_states]( std::string readMsg ) {
                    std::size_t index = 0;
                    while ( index < readMsg.size() )
                    {
                        msg_out[index] = readMsg[index];
                        ++index;
                    }
                    msg_out[index] = '\0';

                    try
                    {
                        const auto parsedMsg = sn::parse_ui_message( readMsg );
                        if ( parsedMsg.type == sn::MessageType::FullState )
                        {
                            node_states.SetNodeStates( parsedMsg.nodes );
                        }
                        else if ( parsedMsg.type == sn::MessageType::NodeConnect )
                        {
                            node_states.NodeConnected( parsedMsg.nodes.front() );
                        }
                        else if ( parsedMsg.type == sn::MessageType::NodeDisconnect )
                        {
                            node_states.NodeDisconnected( parsedMsg.nodes.front() );
                        }
                        else if ( parsedMsg.type == sn::MessageType::NodeUpdate )
                        {
                            node_states.NodeUpdated( parsedMsg.nodes.front() );
                        }
                    }
                    catch ( const std::exception& e )
                    {
                        std::cout << "Failed to parse incoming message: " << e.what() << '\n';
                    }
                } );
                pSession->run( ipAddress.str(), std::to_string( port ) );

                connection_state.Connected();
            }
        }
        else
        {
            if ( ImGui::Button( "Disconnect" ) )
            {
                pSession->async_close();
                std::cout << "Disconnect\n";

                connection_state.Disconnected();
                node_states.Reset();
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

        if ( connection_state.IsConnected() )
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

        node_states.DrawNodes();

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
