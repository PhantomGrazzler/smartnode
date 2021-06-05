#include "messages.hpp"

#include <string>

namespace sn
{

ParsedMessage parse( std::string msg );
ParsedUiMessage parse_ui_message( std::string msg );

} // namespace sn
