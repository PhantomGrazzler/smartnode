#pragma once

#include "connection.hpp"

#include <nlohmann/json.hpp>

#include <set>
#include <memory>
#include <string>
#include <map>

namespace sn
{

class Session;
enum class UIId;
enum class NodeId;
enum class IOId;

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

    /*!
        @brief Builds an "all_nodes_state" message.
     */
    std::string BuildAllNodesState() const;

    /*!
        @brief Returns true if the node with the provided ID is currently connected.
        @param[in] id The node ID to test.
     */
    bool IsNodeConnected( const NodeId id ) const;

    /*!
        @brief Updates the value of the specified IO in the local cache.
        @param[in] nodeId The node for which the value should be updated.
        @param[in] ioId The IO in the node for which the value should be updated.
        @param[in] value The new value to set for the specified IO.
     */
    template<typename T>
    void UpdateIOCache( const NodeId nodeId, const IOId ioId, const T value );

    /*!
        @brief Sends a message to the specified node.
        @param[in] nodeId The node to which the message should be sent.
        @param[in] message The message to send.
     */
    void SendMessageToNode( const NodeId nodeId, const std::string& message );

private: // data
    std::set<Connection<UIId>> m_uiConnections;
    std::set<Connection<NodeId>> m_nodeConnections;
    std::map<NodeId, nlohmann::json> m_nodeStates;
};

} // namespace sn
