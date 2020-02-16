#pragma once

#include "Connection.hpp"

#include <set>
#include <memory>
#include <string>

namespace sns
{

class Session;
enum class UIId;
enum class NodeId;

class MessageEngine final
{
public:
    MessageEngine() = default;

    /*!
        @brief Indicates that a message has been received from a remote peer.
        @param[in] pSession The session from which the message was received. This value will be
                   moved.
        @param[in] message The message that has been received.
     */
    void MessageReceived( std::weak_ptr<Session>&& pSession, const std::string& message );

    /*!
        @brief Indicates that the supplied peer has disconnected from the server.
        @param[in] pSession The session that is now disconnected.
     */
    void PeerDisconnected( std::weak_ptr<Session>&& pSession );

private: // methods
    /*!
        @brief Returns true if the provided session represents a peer that has already
               sent a connect message (either UI or node).
        @param[in] pSession The session to test.
     */
    bool PeerAlreadyConnected( const std::shared_ptr<Session>& pSession ) const;

    /*!
        @brief Adds a new connection to one of the collections of active connections, if not already
               present. Will also remove any expired UI sessions.
        @param[in] pSession Weak pointer to the new session.
        @param[in] id The unique ID of the UI that is connecting.
     */
    void AddConnection( std::weak_ptr<Session>&& pSession, const UIId id );

    /*!
        @brief Adds a new connection to one of the collections of active connections, if not already
               present. Will also remove any expired Node sessions.
        @param[in] pSession Weak pointer to the new session.
        @param[in] id The unique ID of the Node that is connecting.
     */
    void AddConnection( std::weak_ptr<Session>&& pSession, const NodeId id );

    /*!
        @brief Synchronously forwards the provided message to all connected UIs.
        @param[in] message The message to be forwarded.
     */
    void ForwardMessageToUIs( const std::string& message );

    /*!
        @brief Prints all the active UI and Node connections to the console.
     */
    void PrintConnections() const;

private: // data
    std::set<Connection<UIId>> m_uiConnections;
    std::set<Connection<NodeId>> m_nodeConnections;
};

} // namespace sns
