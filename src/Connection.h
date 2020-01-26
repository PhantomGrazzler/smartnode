
#include <memory>
#include <type_traits>

namespace sns
{

class Session;

enum class UIId : uint32_t {};
enum class NodeId : uint32_t {};

template<
    typename T,
    typename = std::enable_if<std::is_integral_v<T>>
>
class Connection final
{
public:
    Connection(
        const T& id,
        std::weak_ptr<sns::Session> pSession )
        : m_id( id )
        , m_pSession( std::move( pSession ) )
    { }

    const T& Id() const { return m_id; }
    const std::weak_ptr<sns::Session>& Session() const { return m_pSession; }

    bool operator<( const Connection& rhs ) const
    {
        return m_id < rhs.m_id;
    }

private:
    const T m_id;
    const std::weak_ptr<sns::Session> m_pSession;
};

}
