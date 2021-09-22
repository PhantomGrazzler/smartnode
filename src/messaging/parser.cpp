// This is disgusting parsing code... :(
// TODO: After cleaning up the copious amounts of vomit, also clean up this parsing mess.

#include "parser.hpp"

#include <boost/algorithm/string.hpp>

#include <stdexcept>
#include <regex>

namespace sn
{

const char startOfMsg = '<';
const char endOfMsg = '>';
const std::size_t minMsgSize = 5;
const std::size_t minUiMsgSize = 4;
const std::regex msgTypeRegex( R"(<(\w)_.*>)" );
const std::regex fullStateRegex( R"(<s(_n_\d+(_(\w{2})_(\d+)_(\d+))*)+>)" );
const std::regex nodeRegex( R"(n_(\d+)((_\w{2}_\d+_\d+)*))" );
const std::regex ioRegex( R"((\w{2})_(\d+)_(\d+))" );
const std::string emptyFullStateMsg( "<s_>" );

bool is_message_framed( const std::string& msg )
{
    return msg.front() == startOfMsg && msg.back() == endOfMsg;
}

MessageType get_message_type( const std::string& msg )
{
    std::smatch msg_type_match;

    if ( std::regex_match( msg, msg_type_match, msgTypeRegex ) )
    {
        const char msg_type = msg_type_match.str( 1 ).front();
        switch ( msg_type )
        {
        case 'c':
            return MessageType::NodeConnect;
        case 'u':
            return MessageType::Update;
        case 'a':
            return MessageType::Ack;
        case 'n':
            return MessageType::Nak;
        case 'g':
            return MessageType::UiConnect;
        case 's':
            return MessageType::FullState;
        default:
            throw std::runtime_error( "Invalid message type: " + std::to_string( msg_type ) );
        }
    }
    else
    {
        throw std::runtime_error( "Failed to parse message type." );
    }
}

template<typename IdType>
IdType get_id( const std::vector<std::string>& components )
{
    return static_cast<IdType>( std::stoi( components.at( 1 ) ) );
}

ParsedMessage parse_ack( const MessageType msgType, const std::vector<std::string>& components )
{
    if ( components.size() != 2 )
    {
        throw std::runtime_error( "Invalid ACK/NAK message size." );
    }

    ParsedMessage parsedMsg{ msgType };
    parsedMsg.node.id = get_id<NodeId>( components );

    return parsedMsg;
}

ParsedMessage parse_node_connect( const std::vector<std::string>& components )
{
    if ( ( components.size() - 2 ) % 3 != 0 )
    {
        throw std::runtime_error( "Invalid number of segments for node connect message." );
    }

    ParsedMessage parsedMsg( MessageType::NodeConnect );
    parsedMsg.node.id = get_id<NodeId>( components );

    if ( parsedMsg.node.id == invalid_node_id )
    {
        throw std::runtime_error( "Invalid Node ID." );
    }

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
    if ( ( components.size() - 2 ) % 3 != 0 )
    {
        throw std::runtime_error( "Invalid number of segments for node update message." );
    }

    ParsedMessage parsedMsg( MessageType::Update );
    parsedMsg.node.id = get_id<NodeId>( components );

    if ( parsedMsg.node.id == invalid_node_id )
    {
        throw std::runtime_error( "Invalid Node ID." );
    }

    for ( std::size_t segmentNum = 2; segmentNum < components.size(); segmentNum += 2 )
    {
        const auto ioId = static_cast<IOId>( std::stoi( components.at( segmentNum ) ) );
        const auto value = std::stoi( components.at( segmentNum + 1 ) );
        parsedMsg.node.io.emplace_back( ioId, "Existing", value );
    }

    return parsedMsg;
}

ParsedMessage parse_ui_connect( const std::vector<std::string>& components )
{
    if ( components.size() != 2 )
    {
        throw std::runtime_error( "Invalid number of segments for UI connect message." );
    }

    ParsedMessage parsedMsg( MessageType::UiConnect );
    parsedMsg.ui.id = get_id<UIId>( components );

    if ( parsedMsg.ui.id == invalid_ui_id )
    {
        throw std::runtime_error( "Invalid UI ID." );
    }

    return parsedMsg;
}

std::string remove_framing( const std::string& msg )
{
    return msg.substr( 1, msg.size() - 2 );
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
        const auto msgType = get_message_type( msg );
        auto unframedMsg = remove_framing( msg );
        std::vector<std::string> components;
        boost::algorithm::split( components, unframedMsg, boost::is_any_of( "_" ) );

        if ( components.empty() )
        {
            throw std::runtime_error( "Failed to parse message." );
        }

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
        else if ( msgType == MessageType::UiConnect )
        {
            return parse_ui_connect( components );
        }
        else
        {
            throw std::runtime_error( "Unhandled message type encountered." );
        }
    }
}

std::vector<IO> parse_ios( const std::string& msg )
{
    std::vector<IO> ioCollection;
    std::smatch ioMatch;
    auto mutableMsg = msg;

    while ( std::regex_search( mutableMsg, ioMatch, ioRegex ) )
    {
        const auto& type = ioMatch.str( 1 );
        const auto& id = ioMatch.str( 2 );
        const auto& value = ioMatch.str( 3 );

        const auto ioId = static_cast<IOId>( std::stoi( id ) );
        const auto ioValue = std::stoi( value );

        ioCollection.emplace_back( ioId, type, ioValue );

        mutableMsg = ioMatch.suffix();
    }

    return ioCollection;
}

std::vector<Node> parse_nodes( const std::string& msg )
{
    std::vector<Node> nodes;
    std::smatch nodeMatch;
    auto mutableMsg = msg;

    while ( std::regex_search( mutableMsg, nodeMatch, nodeRegex ) )
    {
        const auto& id = nodeMatch.str( 1 );
        const auto& ioDetails = nodeMatch.str( 2 );

        const auto nodeId = static_cast<NodeId>( std::stoi( id ) );
        const auto ios = parse_ios( ioDetails );
        nodes.emplace_back( nodeId, ios );

        mutableMsg = nodeMatch.suffix();
    }

    return nodes;
}

ParsedUiMessage parse_full_state( const std::string& msg )
{
    ParsedUiMessage parsedMsg{ MessageType::FullState };
    std::smatch fullStateMatch;

    if ( msg == emptyFullStateMsg )
    {
        return parsedMsg;
    }
    else if ( std::regex_match( msg, fullStateMatch, fullStateRegex ) )
    {
        parsedMsg.nodes = parse_nodes( msg );
        return parsedMsg;
    }
    else
    {
        throw std::runtime_error( "Failed to parse full state message." );
    }
}

ParsedUiMessage parse_ui_message( std::string msg )
{
    if ( msg.size() < minUiMsgSize )
    {
        throw std::runtime_error( "Message is too short." );
    }
    else if ( !is_message_framed( msg ) )
    {
        throw std::runtime_error( "Message is not framed correctly." );
    }
    else
    {
        const auto msgType = get_message_type( msg );

        if ( msgType == MessageType::FullState )
        {
            return parse_full_state( msg );
        }
        else
        {
            throw std::runtime_error( "Unhandled message type encountered." );
        }
    }
}

} // namespace sn
