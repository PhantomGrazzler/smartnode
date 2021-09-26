#pragma once

#include "messages.hpp"

namespace sn
{

std::string BuildAck( const ParsedMessage& msg );
std::string BuildNak( const ParsedMessage& msg );
std::string BuildUiConnect( const UIId id );
std::string BuildFullState( const std::vector<Node>& nodes );
std::string BuildNodeDisconnect( const NodeId id );

} // namespace sn
