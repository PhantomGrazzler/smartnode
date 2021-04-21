#include "message_builder.hpp"

namespace sn
{

std::string BuildNak( const ParsedMessage& msg )
{
    return startOfMessage + "n_" + std::to_string( static_cast<uint32_t>( msg.node.id ) ) +
           endOfMessage;
}

} // namespace sn
