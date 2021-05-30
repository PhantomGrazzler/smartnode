#include "message_builder.hpp"

#include <sstream>

namespace sn
{

std::string BuildAck( const ParsedMessage& msg )
{
    if ( msg.node.id != invalide_node_id )
    {
        return startOfMessage + "a_" + std::to_string( static_cast<uint32_t>( msg.node.id ) ) +
               endOfMessage;
    }
    else
    {
        return startOfMessage + "a_" + std::to_string( static_cast<uint32_t>( msg.ui.id ) ) +
               endOfMessage;
    }
}

std::string BuildNak( const ParsedMessage& msg )
{
    if ( msg.node.id != invalide_node_id )
    {
        return startOfMessage + "n_" + std::to_string( static_cast<uint32_t>( msg.node.id ) ) +
               endOfMessage;
    }
    else
    {
        return startOfMessage + "n_" + std::to_string( static_cast<uint32_t>( msg.ui.id ) ) +
               endOfMessage;
    }
}

std::string BuildUiConnect( const UIId id )
{
    return startOfMessage + "g_" + std::to_string( static_cast<uint32_t>( id ) ) + endOfMessage;
}

std::string BuildFullState( const std::vector<Node>& nodes )
{
    std::ostringstream oss;
    oss << startOfMessage << 's' << ( nodes.empty() ? "_" : "" );

    for ( const auto& node : nodes )
    {
        oss << "_n_" << node.id;
        for ( const auto& io : node.io )
        {
            oss << '_' << io.type << '_' << io.id << '_' << io.value;
        }
    }

    oss << endOfMessage;
    return oss.str();
}

} // namespace sn
