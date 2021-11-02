#include "message_engine.hpp"
#include "console_printer.hpp"
#include "id_types.hpp"
#include "session.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include "message_builder.hpp"

#include <set>
#include <variant>

namespace sn
{

/*!
    @brief Removes any expired sessions from the provided container.
    @param[in] container The container from which to remove expired sessions.
 */
template<typename T>
void RemoveExpiredSessions( T& container )
{
    auto it = container.begin();
    while ( it != container.end() )
    {
        if ( ( *it ).Session().expired() )
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
void RemoveDisconnectedPeer( T& container, const U id )
{
    for ( const auto& element : container )
    {
        if ( element.Id() == id )
        {
            container.erase( element );
            PrintInfo( id, " disconnected." );
            return;
        }
    }
}

template<typename T>
bool SessionInContainer( const std::shared_ptr<Session>& pSession, const T& container )
{
    return std::find_if(
               container.cbegin(),
               container.cend(),
               [&pSession]( const auto& connection ) {
                   return connection.Session().lock() == pSession;
               } ) != container.cend();
}

bool MessageEngine::PeerConnected( const std::shared_ptr<Session>& pSession ) const
{
    return SessionInContainer( pSession, m_uiConnections ) ||
           SessionInContainer( pSession, m_nodeConnections );
}

void MessageEngine::MessageReceived( std::weak_ptr<Session>&& pSession, const std::string& message )
{
    try
    {
        const auto pLockedSession = pSession.lock();
        const auto msg = parse( message, pLockedSession->GetPeerType() );
        Log( spdlog::level::debug, "Message Type is {}", msg.type );

        if ( pLockedSession == nullptr )
        {
            return;
        }

        switch ( msg.type )
        {

        case MessageType::NodeConnect:
        {
            const auto nodeId = msg.node.id;
            const auto nodeIdStr = to_string( nodeId );

            if ( PeerConnected( pLockedSession ) )
            {
                pLockedSession->SendMessage( BuildNak( msg ) );
                const auto peerId = pLockedSession->PeerIdAsString();

                Log( spdlog::level::warn, "{} attempting to connect as {}", peerId, nodeIdStr );
                PrintWarning( peerId, " attempting to connect as ", nodeIdStr );
            }
            else if ( IsNodeConnected( nodeId ) )
            {
                pLockedSession->SendMessage( BuildNak( msg ) );

                Log( spdlog::level::warn, "New Node attempting to connect as {}", nodeIdStr );
                PrintWarning( "New Node attempting to connect as ", nodeIdStr );
            }
            else
            {
                pLockedSession->SetPeerId( nodeId );
                AddConnection( std::move( pSession ), nodeId );

                Log( spdlog::level::info, "{} connected", nodeIdStr );
                PrintInfo( nodeIdStr, " connected" );

                m_nodeStates.push_back( msg.node );

                pLockedSession->SendMessage( BuildAck( msg ) );
                ForwardMessageToUIs( message );
            }
        }
        break;

        case MessageType::NodeUpdate:
        {
            if ( PeerConnected( pLockedSession ) )
            {
                const auto nodeId = msg.node.id;
                const auto nodeIdStr = to_string( nodeId );

                // TODO: Check that IO exists on the Node.
                // TODO: Check that each updated IO is an input.

                for ( const auto& io : msg.node.io )
                {
                    UpdateIOCache( nodeId, io.id, io.value );

                    Log( spdlog::level::info, "{} updated {} to {}", nodeIdStr, io.id, io.value );
                    PrintInfo( nodeIdStr, " updated ", io.id, " to ", io.value );
                }

                ForwardMessageToUIs( message );
            }
            else
            {
                Log( spdlog::level::warn, "Ignoring update from non-connected peer" );
                PrintWarning( "Ignoring update from non-connected peer" );
            }
        }
        break;

        case MessageType::Ack:
        {
            const auto nodeIdStr = to_string( msg.node.id );
            Log( spdlog::level::debug, "{} ACK", nodeIdStr );
            PrintDebug( nodeIdStr, " ACK" );
        }
        break;

        case MessageType::Nak:
        {
            const auto nodeIdStr = to_string( msg.node.id );
            Log( spdlog::level::debug, "{} NAK", nodeIdStr );
            PrintDebug( nodeIdStr, " NAK" );
        }
        break;

        case MessageType::UiConnect:
        {
            const auto uiId = msg.ui.id;
            const auto uiIdStr = to_string( uiId );

            if ( PeerConnected( pLockedSession ) )
            {
                pLockedSession->SendMessage( BuildNak( msg ) );

                Log( spdlog::level::warn,
                     "{} attempting to connect as {}",
                     pLockedSession->PeerIdAsString(),
                     uiIdStr );
                PrintWarning(
                    pLockedSession->PeerIdAsString(),
                    " attempting to connect as ",
                    uiIdStr );
            }
            else
            {
                pLockedSession->SetPeerId( uiId );
                AddConnection( std::move( pSession ), uiId );

                Log( spdlog::level::info, "{} connected", uiIdStr );
                PrintInfo( uiIdStr, " connected" );

                pLockedSession->SendMessage( BuildFullState( m_nodeStates ) );
            }
        }
        break;

        case MessageType::UiUpdate:
        {
            const auto nodeId = msg.node.id;
            const auto nodeIdStr = to_string( nodeId );
            const auto uiId = pLockedSession->PeerIdAsString();

            for ( const auto& io : msg.node.io )
            {
                UpdateIOCache( nodeId, io.id, io.value );

                Log( spdlog::level::info,
                     "{} updated {} to {} on {}",
                     uiId,
                     io.id,
                     io.value,
                     nodeIdStr );
                PrintInfo( uiId, " updated ", io.id, " to ", io.value, " on ", nodeIdStr );
            }

            SendMessageToNode( nodeId, message );

            // TODO: Do not send the message back to the UI that originally sent it.
            // TODO: Check that IO exists on the Node.
            // TODO: Check that each IO is an output.
            // ForwardMessageToUIs( message );
        }
        break;

        default:
        {
            Log( spdlog::level::warn, "Unknown message received: {}", message );
            PrintWarning( "Unknown message received: ", message );
        }

        } // switch ( msg.type )
    }
    catch ( const std::exception& e )
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
            RemoveNodeFromCache( id );

            ForwardMessageToUIs( BuildNodeDisconnect( id ) );
        }
    }

    PrintConnections();
}

template<typename T>
void PrintContainer(
    std::ostringstream& oss, const T& container, const std::string& prefix, const char suffix )
{
    for ( const auto& item : container )
    {
        oss << prefix << to_string( item.Id() ) << suffix;
    }
}

void MessageEngine::PrintConnections() const
{
    std::ostringstream oss;

    PrintContainer( oss, m_uiConnections, "  ", '\n' );
    PrintContainer( oss, m_nodeConnections, "  ", '\n' );

    PrintInfo( "Connections:\n", oss.str() );
    Log( spdlog::level::info, "Active connections:\n {}", oss.str() );
}

bool MessageEngine::IsNodeConnected( const NodeId id ) const
{
    return std::find_if(
               m_nodeConnections.cbegin(),
               m_nodeConnections.cend(),
               [id]( const auto& connection ) { return connection.Id() == id; } ) !=
           m_nodeConnections.cend();
}

void MessageEngine::SendMessageToNode( const NodeId nodeId, const std::string& message )
{
    const auto nodeConnection = std::find_if(
        m_nodeConnections.cbegin(),
        m_nodeConnections.cend(),
        [nodeId]( const auto& connection ) { return connection.Id() == nodeId; } );

    if ( nodeConnection != m_nodeConnections.cend() )
    {
        if ( const auto pLockedSession = ( *nodeConnection ).Session().lock() )
        {
            pLockedSession->SendMessage( message );
        }
        else
        {
            PrintWarning( nodeId, " is not connected" );
        }
    }
    else
    {
        PrintWarning( nodeId, " is not connected" );
    }
}

void MessageEngine::AddConnection( std::weak_ptr<Session>&& pSession, const UIId id )
{
    RemoveExpiredSessions( m_uiConnections );
    m_uiConnections.emplace( id, std::move( pSession ) );

    PrintConnections();
}

void MessageEngine::AddConnection( std::weak_ptr<Session>&& pSession, const NodeId id )
{
    RemoveExpiredSessions( m_nodeConnections );
    m_nodeConnections.emplace( id, std::move( pSession ) );

    PrintConnections();
}

void MessageEngine::ForwardMessageToUIs( const std::string& message )
{
    for ( const auto& connection : m_uiConnections )
    {
        const auto pSession = connection.Session().lock();
        if ( pSession )
        {
            pSession->SendMessage( message );
        }
    }
}

template<typename T>
void MessageEngine::UpdateIOCache( const NodeId nodeId, const IOId ioId, const T newValue )
{
    for ( auto& node : m_nodeStates )
    {
        if ( node.id == nodeId )
        {
            for ( auto& ioItem : node.io )
            {
                if ( ioItem.id == ioId )
                {
                    ioItem.value = newValue;
                }
            }
        }
    }
}

void MessageEngine::RemoveNodeFromCache( const NodeId nodeId )
{
    auto nodeIter = std::find_if(
        begin( m_nodeStates ),
        end( m_nodeStates ),
        [nodeId]( const auto& existingNode ) { return nodeId == existingNode.id; } );

    if ( nodeIter != end( m_nodeStates ) )
    {
        m_nodeStates.erase( nodeIter );
    }
}

} // namespace sn
