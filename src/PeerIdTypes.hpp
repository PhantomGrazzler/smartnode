#pragma once

#include <cstdint>
#include <ostream>

namespace sns
{

enum class UIId{};
enum class NodeId{};

std::ostream& operator<<( std::ostream& os, const UIId id )
{
    return os << "UI " << static_cast<uint32_t>(id);
}

std::ostream& operator<<( std::ostream& os, const NodeId id )
{
    return os << "Node " << static_cast<uint32_t>(id);
}

}
