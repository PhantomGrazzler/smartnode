#pragma once

#include <cstdint>
#include <iostream>

namespace sns
{

enum class UIId{};
enum class NodeId{};

std::ostream& operator<<(std::ostream& os, const UIId id)
{
    return os << "UI ID " << static_cast<uint32_t>(id);
}

std::ostream& operator<<(std::ostream& os, const NodeId id)
{
    return os << "Node ID " << static_cast<uint32_t>(id);
}

}
