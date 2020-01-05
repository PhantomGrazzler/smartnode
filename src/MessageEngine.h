
#include "Connection.h"

#include <set>
#include <memory>
#include <string>

namespace sns
{

class Session;

class MessageEngine final
{
public:
    MessageEngine() = default;

    /*!
        @brief Indicates that a message has been received from a remote peer.
        @param[in] pSession The session from which the message was received. This value will be moved.
        @param[in] message The message that has been received.
     */
    void MessageReceived(
        std::weak_ptr<Session>&& pSession,
        const std::string& message );

private: // methods
    /*!
        @brief Adds a new connection to one of the collections of active connections, if not already present. Will also
               remove any expired UI sessions.
        @param[in] pSession Weak pointer to the new session.
        @param[in] id The unique ID of the UI that is connecting.
     */
    void AddConnection(
        std::weak_ptr<Session>&& pSession,
        const UIId id );

    /*!
        @brief Adds a new connection to one of the collections of active connections, if not already present. Will also
               remove any expired Node sessions.
        @param[in] pSession Weak pointer to the new session.
        @param[in] id The unique ID of the Node that is connecting.
     */
    void AddConnection(
        std::weak_ptr<Session>&& pSession,
        const NodeId id );

    /*!
        @brief Synchronously forwards the provided message to all connected UIs.
        @param[in] message The message to be forwarded.
     */
    void ForwardMessageToUIs( const std::string& message );

private: // data
    std::set<Connection<UIId>>      m_uiConnections;
    std::set<Connection<NodeId>>    m_nodeConnections;
};

}
