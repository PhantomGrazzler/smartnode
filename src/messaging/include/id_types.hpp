#pragma once

#include <iosfwd>

namespace sn
{

enum class UIId
{
};
enum class NodeId
{
};
enum class IOId
{
};

std::ostream& operator<<( std::ostream& os, const UIId id );
std::ostream& operator<<( std::ostream& os, const NodeId id );
std::ostream& operator<<( std::ostream& os, const IOId id );

} // namespace sn
