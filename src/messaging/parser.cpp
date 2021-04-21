// This is disgusting parsing code... :(
// TODO: After cleaning up the copious amounts of vomit, also clean up this parsing mess.

#include "parser.hpp"

#include <boost/algorithm/string.hpp>

#include <stdexcept>

namespace sn
{

const char startOfMsg = '<';
const char endOfMsg = '>';
const std::size_t minMsgSize = 5;

bool is_message_framed( const std::string& msg )
{
    return msg.front() == startOfMsg && msg.back() == endOfMsg;
}

MessageType get_message_type( const char msgType )
{
    switch ( msgType )
    {
    case 'c':
        return MessageType::NodeConnect;
    case 'u':
        return MessageType::Update;
    case 'a':
        return MessageType::Ack;
    case 'n':
        return MessageType::Nak;
    default:
        throw std::runtime_error( "Invalid message type." );
    }
}

NodeId get_node_id( const std::vector<std::string>& components )
{
    return static_cast<NodeId>( std::stoi( components.at( 1 ) ) );
}

ParsedMessage parse_ack( const MessageType msgType, const std::vector<std::string>& components )
{
    if ( components.size() != 2 )
    {
        throw std::runtime_error( "Invalid ACK/NAK message size." );
    }

    ParsedMessage parsedMsg{ msgType };
    parsedMsg.node.id = get_node_id( components );

    return parsedMsg;
}

ParsedMessage parse_node_connect( const std::vector<std::string>& components )
{
    if ( ( components.size() - 2 ) % 3 != 0 )
    {
        throw std::runtime_error( "Invalid number of segments for node connect message." );
    }

    ParsedMessage parsedMsg( MessageType::NodeConnect );
    parsedMsg.node.id = get_node_id( components );

    for ( std::size_t segmentNum = 2; segmentNum < components.size(); segmentNum += 3 )
    {
        const auto& ioType = components.at( segmentNum );
        const auto ioId = static_cast<IOId>( std::stoi( components.at( segmentNum + 1 ) ) );
        const auto value = std::stoi( components.at( segmentNum + 2 ) );
        parsedMsg.node.io.emplace_back( ioId, ioType, value );
    }

    return parsedMsg;
}

ParsedMessage parse_node_update( const std::vector<std::string>& components )
{
    ParsedMessage parsedMsg( MessageType::Update );
    parsedMsg.node.id = get_node_id( components );

    for ( std::size_t segmentNum = 2; segmentNum < components.size(); segmentNum += 2 )
    {
        const auto ioId = static_cast<IOId>( std::stoi( components.at( segmentNum ) ) );
        const auto value = std::stoi( components.at( segmentNum + 1 ) );
        parsedMsg.node.io.emplace_back( ioId, "Existing", value );
    }

    return parsedMsg;
}

void remove_framing( std::string& msg )
{
    msg.erase( msg.find( endOfMsg ) );
    msg.erase( msg.find( startOfMsg ) );
}

ParsedMessage parse( std::string msg )
{
    if ( msg.size() < minMsgSize )
    {
        throw std::runtime_error( "Message is too short." );
    }
    else if ( !is_message_framed( msg ) )
    {
        throw std::runtime_error( "Message is not framed correctly." );
    }
    else
    {
        remove_framing( msg );
        std::vector<std::string> components;
        boost::algorithm::split( components, msg, boost::is_any_of( "_" ) );

        const auto msgType = get_message_type( components.front().front() );

        if ( msgType == MessageType::Ack || msgType == MessageType::Nak )
        {
            return parse_ack( msgType, components );
        }
        else if ( msgType == MessageType::NodeConnect )
        {
            return parse_node_connect( components );
        }
        else if ( msgType == MessageType::Update )
        {
            return parse_node_update( components );
        }
        else
        {
            throw std::runtime_error( "Unhandled message type encountered." );
        }
    }
}

} // namespace sn
