#include "MessageEngine.hpp"
#include "Session.hpp"
#include "PeerIdTypes.hpp"
#include "ConsolePrinter.hpp"

// Third-party
#include <nlohmann/json.hpp>

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
            PrintInfo( "Peer ", id, " disconnected." );
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
        PrintDebug( "Message Type is ", msgType );

        if( msgType == "ui_connect" )
        {
            const UIId& id = msg.at( "UIId" );
            PrintInfo( id, " connected" );
            pSession.lock()->SetPeerId( id );
            AddConnection( std::move( pSession ), id );
        }
        else if( msgType == "node_connect" )
        {
            const NodeId& id = msg.at( "NodeId" );
            PrintInfo( id, " connected" );
            pSession.lock()->SetPeerId( id );
            AddConnection( std::move( pSession ), id );

            ForwardMessageToUIs( message );
        }
    }
    catch( const nlohmann::json::exception & e )
    {
        PrintWarning( "Failed to parse incoming message: ", e.what() );
        PrintDebug( "Message: \n", message );
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

template<typename T>
void PrintContainer(
    std::ostringstream& oss,
    const T& container,
    const std::string& prefix,
    const char suffix )
{
    for (const auto& item : container)
    {
        oss << prefix << item.Id() << suffix;
    }
}

void MessageEngine::PrintConnections() const
{
    std::ostringstream oss;

    PrintContainer( oss, m_uiConnections, "  ", '\n' );
    PrintContainer( oss, m_nodeConnections, "  ", '\n' );

    PrintInfo( "Connections:\n", oss.str() );
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
