#include "message_builder.hpp"

namespace sn
{

std::string BuildAck( const ParsedMessage& msg )
{
    return startOfMessage + "a_" + std::to_string( static_cast<uint32_t>( msg.node.id ) ) +
           endOfMessage;
}

std::string BuildNak( const ParsedMessage& msg )
{
    return startOfMessage + "n_" + std::to_string( static_cast<uint32_t>( msg.node.id ) ) +
           endOfMessage;
}

} // namespace sn
