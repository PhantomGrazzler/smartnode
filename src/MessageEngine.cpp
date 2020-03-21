#include "MessageEngine.hpp"

#include "ConsolePrinter.hpp"
#include "IdTypes.hpp"
#include "Session.hpp"

#include <nlohmann/json.hpp>
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

                ForwardMessageToUIs( message );
            }
        }
        else if ( msgType == "input_update" )
        {
            const NodeId& id = msg.at( "NodeId" );
            const IOId& ioId = msg.at( "IOId" );
            const auto value = msg.at( "Value" );

            PrintDebug( id, " updated ", ioId, " to ", value );

            ForwardMessageToUIs( msg.dump() );
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

} // namespace sns
