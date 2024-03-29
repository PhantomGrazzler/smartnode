#pragma once

#include <iosfwd>
#include <string>

namespace sn
{

enum class UIId
{};
enum class NodeId
{};
enum class IOId
{};

std::string to_string( const UIId id );
std::string to_string( const NodeId id );
std::string to_string( const IOId id );

std::ostream& operator<<( std::ostream& os, const UIId id );
std::ostream& operator<<( std::ostream& os, const NodeId id );
std::ostream& operator<<( std::ostream& os, const IOId id );

const UIId invalid_ui_id = UIId{};
const NodeId invalid_node_id = NodeId{};
const IOId invalid_io_id = IOId{};

} // namespace sn
