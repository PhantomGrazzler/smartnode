
#include "MessageEngine.h"
#include "Session.h"
#include "PeerIdTypes.hpp"

// Third-party
#include <nlohmann/json.hpp>
#include <rang.hpp>

// Standard library
#include <variant>
#include <set>

namespace sns
{

/*!
    @brief Removes any expired sessions from the provided container.
    @param[in] container The container from which to remove expired sessions.
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

template<typename T, typename U>
void RemoveDisconnectedPeer(T& container, const U id)
{
    for ( const auto& element : container )
    {
        if ( element.Id() == id )
        {
            container.erase( element );
            std::cout << rang::fg::cyan << "Peer with " << id << " disconnected.\n" << rang::fg::reset;
            return;
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
            const UIId& id = msg.at( "UIId" );
            std::cout << id << " connected" << "\n\n";
            pSession.lock()->SetPeerId( id );
            AddConnection( std::move( pSession ), id );
        }
        else if( msgType == "node_connect" )
        {
            const NodeId& id = msg.at( "NodeId" );
            std::cout << id << " connected" << "\n\n";
            pSession.lock()->SetPeerId( id );
            AddConnection( std::move( pSession ), id );

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
    if ( pLockedSession )
    {
        const auto peerId = pLockedSession->GetPeerId();

        if ( std::holds_alternative<UIId>( peerId ) )
        {
            const auto id = std::get<UIId>( peerId );
            RemoveDisconnectedPeer( m_uiConnections, id );
        }
        else if ( std::holds_alternative<NodeId>( peerId ) )
        {
            const auto id = std::get<NodeId>( peerId );
            RemoveDisconnectedPeer( m_nodeConnections, id );
        }
    }

    PrintConnections();
}

void MessageEngine::PrintConnections() const
{
    std::cout << "Connections:\n";

    for ( const auto& connection : m_uiConnections )
    {
        std::cout << connection.Id() << "\n";
    }

    for ( const auto& connection : m_nodeConnections )
    {
        std::cout << connection.Id() << "\n";
    }
}

void MessageEngine::AddConnection(
    std::weak_ptr<Session>&& pSession,
    const UIId id )
{
    RemoveExpiredSessions( m_uiConnections );
    m_uiConnections.emplace( id, std::move( pSession ) );

    PrintConnections();
}

void MessageEngine::AddConnection(
    std::weak_ptr<Session>&& pSession,
    const NodeId id )
{
    RemoveExpiredSessions( m_nodeConnections );
    m_nodeConnections.emplace( id, std::move( pSession ) );

    PrintConnections();
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
