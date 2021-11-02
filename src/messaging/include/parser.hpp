#include "messages.hpp"

#include <string>

namespace sn
{

ParsedMessage parse( std::string msg, const PeerType peerType );
ParsedUiMessage parse_ui_message( std::string msg );

} // namespace sn
