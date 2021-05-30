#pragma once

#include "messages.hpp"

namespace sn
{

std::string BuildAck( const ParsedMessage& msg );
std::string BuildNak( const ParsedMessage& msg );
std::string BuildUiConnect( const UIId id );

} // namespace sn
