
#include "MessageEngine.h"
#include "Session.h"

// Third-party
#include <nlohmann/json.hpp>
#include <rang.hpp>

namespace sns
{

/*!
    @brief Removes any expired sessions from the provided container.
 */
template<typename T>
void RemoveExpiredSessions( T& container )
{
    auto it = container.begin();
    while( it != container.end() )
    {
        if( (*it).Session().expired() )
        {
            it = container.erase( it );
        }
        else
        {
            ++it;
        }
    }
}

void MessageEngine::MessageReceived(
    std::weak_ptr<Session>&& pSession,
    const std::string& message )
{
    try
    {
        const nlohmann::json msg = nlohmann::json::parse( message );
        const auto& msgType = msg.at( "MsgType" );
        std::cout << "Message Type is " << msgType << "\n";

        if( msgType == "ui_connect" )
        {
            const auto& id = msg.at( "UIId" );
            std::cout << "UI ID is " << id << "\n\n";
            pSession.lock()->SetPeerId( id );
            AddConnection( std::move( pSession ), UIId( id ) );
        }
        else if( msgType == "node_connect" )
        {
            const auto& id = msg.at( "NodeId" );
            std::cout << "Node ID is " << id << "\n\n";
            pSession.lock()->SetPeerId( id );
            AddConnection( std::move( pSession ), NodeId( id ) );

            ForwardMessageToUIs( message );
        }
    }
    catch( const nlohmann::json::exception & e )
    {
        std::cout << rang::fg::yellow << "Failed to parse incoming message.\n" << e.what() << rang::fg::reset << '\n';
        std::cout << rang::fg::green << "Message: \n" << message << rang::fg::reset << "\n\n";
    }
}

void MessageEngine::PeerDisconnected( std::weak_ptr<Session>&& pSession )
{
    const auto pLockedSession = pSession.lock();

    
}

void MessageEngine::AddConnection(
    std::weak_ptr<Session>&& pSession,
    const UIId id )
{
    RemoveExpiredSessions( m_uiConnections );
    m_uiConnections.emplace( id, std::move( pSession ) );

    std::cout << "UI Connections:\n";
    for( const auto& connection : m_uiConnections )
    {
        std::cout << static_cast<uint32_t>( connection.Id() ) << "\n";
    }
}

void MessageEngine::AddConnection(
    std::weak_ptr<Session>&& pSession,
    const NodeId id )
{
    RemoveExpiredSessions( m_nodeConnections );
    m_nodeConnections.emplace( id, std::move( pSession ) );

    std::cout << "Node Connections:\n";
    for( const auto& connection : m_nodeConnections )
    {
        std::cout << static_cast<uint32_t>( connection.Id() ) << "\n";
    }
}

void MessageEngine::ForwardMessageToUIs( const std::string& message )
{
    for( const auto& connection : m_uiConnections )
    {
        const auto pSession = connection.Session().lock();
        if( pSession )
        {
            pSession->SendMessage( message );
        }
    }
}

}
