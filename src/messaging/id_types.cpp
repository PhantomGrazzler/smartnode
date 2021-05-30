#include "id_types.hpp"

#include <cstdint>
#include <ostream>

namespace sn
{

std::string to_string( const UIId id )
{
    return "UI " + std::to_string( static_cast<uint32_t>( id ) );
}

std::string to_string( const NodeId id )
{
    return "Node " + std::to_string( static_cast<uint32_t>( id ) );
}

std::string to_string( const IOId id )
{
    return "IO " + std::to_string( static_cast<uint32_t>( id ) );
}

std::ostream& operator<<( std::ostream& os, const UIId id )
{
    return os << static_cast<uint32_t>( id );
}

std::ostream& operator<<( std::ostream& os, const NodeId id )
{
    return os << static_cast<uint32_t>( id );
}

std::ostream& operator<<( std::ostream& os, const IOId id )
{
    return os << static_cast<uint32_t>( id );
}

} // namespace sn
