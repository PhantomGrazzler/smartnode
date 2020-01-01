
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
        @brief Adds a new connection to one of the collections of active connections, if not already present.
     */
    void AddConnection(
        std::weak_ptr<Session>&& pSession,
        const UIId id );

    /*!
        @brief Adds a new connection to one of the collections of active connections, if not already present.
     */
    void AddConnection(
        std::weak_ptr<Session>&& pSession,
        const NodeId id );

    /*!
        @brief Removes expired sessions from the provided container.
     */
    template<typename T>
    void RemoveExpiredSessions( T& container );

private: // data
    std::set<Connection<UIId>>      m_uiConnections;
    std::set<Connection<NodeId>>    m_nodeConnections;
};

}
