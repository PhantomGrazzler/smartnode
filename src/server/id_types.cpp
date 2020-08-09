#include "id_types.hpp"

#include <cstdint>
#include <ostream>

namespace sns
{

std::ostream& operator<<( std::ostream& os, const UIId id )
{
    return os << "UI " << static_cast<uint32_t>( id );
}

std::ostream& operator<<( std::ostream& os, const NodeId id )
{
    return os << "Node " << static_cast<uint32_t>( id );
}

std::ostream& operator<<( std::ostream& os, const IOId id )
{
    return os << "IO " << static_cast<uint32_t>( id );
}

} // namespace sns
