#pragma once

#include <iosfwd>

namespace sns
{

enum class UIId
{
};
enum class NodeId
{
};

std::ostream& operator<<( std::ostream& os, const UIId id );
std::ostream& operator<<( std::ostream& os, const NodeId id );

} // namespace sns
