
#include <memory>
#include <type_traits>

namespace sns
{

class Session;

template <
    typename T,
    typename Parameter,
    typename = std::enable_if<std::is_integral_v<T>>
>
class NamedType
{
public:
    explicit NamedType( T const& value ) : value_( value ) {}
    T const& get() const { return value_; }
    bool operator<( const NamedType& rhs ) const { return value_ < rhs.value_; }
    bool operator==( const NamedType& rhs ) const { return value_ == rhs.value_; }
private:
    T value_;
};

using UIId = NamedType<uint32_t, struct UIIdParameter>;
using NodeId = NamedType<uint32_t, struct NodeIdParameter>;

template<typename T>
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

    bool operator==( const Connection& rhs ) const
    {
        return m_id == rhs.m_id && m_pSession == rhs.m_pSession;
    }

private:
    const T m_id;
    const std::weak_ptr<sns::Session> m_pSession;
};

}
