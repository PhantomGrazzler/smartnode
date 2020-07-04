#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>

constexpr auto decimalInputOptions = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal |
                                     ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsNoBlank;

int main()
{
    sf::RenderWindow window( sf::VideoMode( 600, 600 ), "SmartNodeUI" );
    window.setFramerateLimit( 60 );
    ImGui::SFML::Init( window );
    sf::Clock deltaClock;

    std::uint8_t v[4] = { 127, 0, 0, 1 };
    int port = 1234;

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
        ImGui::End();

        ImGui::Begin( "Connectivity" );
        ImGui::Text( "IP Address: " );
        ImGui::InputScalarN(
            "",
            ImGuiDataType_::ImGuiDataType_U8,
            v,
            4,
            nullptr,
            nullptr,
            "%d",
            decimalInputOptions );
        ImGui::Text( "Port: " );
        ImGui::InputScalar(
            "",
            ImGuiDataType_::ImGuiDataType_U16,
            &port,
            nullptr,
            nullptr,
            "%d",
            decimalInputOptions );
        if ( ImGui::Button( "Connect", { 60, 20 } ) )
        {
            std::cout << "Connect to " << static_cast<int>( v[0] ) << '.'
                      << static_cast<int>( v[1] ) << '.' << static_cast<int>( v[2] ) << '.'
                      << static_cast<int>( v[3] ) << ':' << port << '\n';
        }
        ImGui::End();

        window.clear();
        ImGui::SFML::Render( window );
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
