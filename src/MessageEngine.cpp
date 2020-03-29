#include "MessageEngine.hpp"
#include "ConsolePrinter.hpp"
#include "IdTypes.hpp"
#include "Session.hpp"

#include <set>
#include <variant>

namespace sns
{
/*!
    @brief Builds an error message with the provided error message.
    @param[in] errorMsg The error message to include in the response.
 */
std::string BuildErrorResponse( const std::string& errorMsg )
{
    nlohmann::json errorResp;

    errorResp["MsgType"] = "error";
    errorResp["ErrorMsg"] = errorMsg;

    return errorResp.dump( 2 );
}

const auto alreadyConnectedResponse =
    BuildErrorResponse( "Already connected with a different ID." );

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
            PrintInfo( "Peer ", id, " disconnected." );
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

bool MessageEngine::PeerAlreadyConnected( const std::shared_ptr<Session>& pSession ) const
{
    return SessionInContainer( pSession, m_uiConnections ) ||
           SessionInContainer( pSession, m_nodeConnections );
}

void MessageEngine::MessageReceived( std::weak_ptr<Session>&& pSession, const std::string& message )
{
    try
    {
        const nlohmann::json msg = nlohmann::json::parse( message );
        const auto& msgType = msg.at( "MsgType" );
        PrintDebug( "Message Type is ", msgType );
        const auto pLockedSession = pSession.lock();

        if ( pLockedSession == nullptr )
        {
            return;
        }

        if ( msgType == "ui_connect" )
        {
            const UIId& id = msg.at( "UIId" );

            if ( PeerAlreadyConnected( pLockedSession ) )
            {
                pLockedSession->SendMessage( alreadyConnectedResponse );

                PrintWarning( pLockedSession->PeerIdAsString(), " attempting to connect as ", id );
            }
            else
            {
                pLockedSession->SetPeerId( id );
                AddConnection( std::move( pSession ), id );
                PrintInfo( id, " connected" );

                pLockedSession->SendMessage( BuildAllNodesState() );
            }
        }
        else if ( msgType == "node_connect" )
        {
            const NodeId& id = msg.at( "NodeId" );

            if ( PeerAlreadyConnected( pLockedSession ) )
            {
                pLockedSession->SendMessage( alreadyConnectedResponse );

                PrintWarning( pLockedSession->PeerIdAsString(), " attempting to connect as ", id );
            }
            else
            {
                pLockedSession->SetPeerId( id );
                AddConnection( std::move( pSession ), id );
                PrintInfo( id, " connected" );

                m_nodeStates.emplace( id, msg["Capabilities"] );

                ForwardMessageToUIs( message );
            }
        }
        else if ( msgType == "input_update" )
        {
            const NodeId nodeId = msg.at( "NodeId" );
            const IOId ioId = msg.at( "IOId" );
            const auto newValue = msg.at( "Value" );

            UpdateIOCache( nodeId, ioId, newValue );

            PrintDebug( nodeId, " updated ", ioId, " to ", newValue );

            ForwardMessageToUIs( msg.dump( 2 ) );
        }
        else if ( msgType == "output_update" )
        {
            const NodeId nodeId = msg.at( "NodeId" );
            const IOId ioId = msg.at( "IOId" );
            const auto newValue = msg.at( "Value" );

            UpdateIOCache( nodeId, ioId, newValue );

            const auto uiId = pLockedSession->PeerIdAsString();
            PrintDebug( uiId, " updated ", ioId, " to ", newValue, " on ", nodeId );

            SendMessageToNode( nodeId, message );
            ForwardMessageToUIs( message );
        }
        else
        {
            PrintDebug( "Unknown message received: ", message );
        }
    }
    catch ( const nlohmann::json::exception& e )
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
    std::ostringstream& oss, const T& container, const std::string& prefix, const char suffix )
{
    for ( const auto& item : container )
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

std::string MessageEngine::BuildAllNodesState() const
{
    nlohmann::json allNodesState;
    allNodesState["MsgType"] = "all_nodes_state";
    allNodesState["States"] = nlohmann::json::array();
    auto& states = allNodesState["States"];

    for ( const auto& [nodeId, nodeState] : m_nodeStates )
    {
        nlohmann::json nodeDescription;
        nodeDescription["NodeId"] = nodeId;
        nodeDescription["IsOnline"] = IsNodeConnected( nodeId );
        nodeDescription["Capabilities"] = nodeState;

        states.insert( states.begin(), nodeDescription );
    }

    return allNodesState.dump( 2 );
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
void MessageEngine::UpdateIOCache( const NodeId nodeId, const IOId ioId, const T value )
{
    if ( m_nodeStates.find( nodeId ) != m_nodeStates.cend() )
    {
        auto& nodeJson = m_nodeStates.at( nodeId );
        for ( auto& [key, io] : nodeJson.items() )
        {
            for ( auto& [key2, ioDescription] : io.items() )
            {
                if ( ioDescription["IOId"] == ioId )
                {
                    ioDescription["Value"] = value;
                    break;
                }
            }
        }
    }
}

} // namespace sns
