#pragma once

#include "id_types.hpp"

#include <vector>
#include <string>

namespace sn
{

static const std::string startOfMessage = "<";
static const std::string endOfMessage = ">";

enum class MessageType : char
{
    NodeConnect = 'c',
    Update = 'u',
    Ack = 'a',
    Nak = 'n'
};

enum class IOType
{
    DigitalInput,
    DigitalOutput,
    AnalogueInput,
    AnalogueOutput,
    Existing
};

struct IO
{
    IO( const IOId id, const std::string& ioType, const int value )
        : id( id )
        , ioType( ioType )
        , value( value )
    {}

    IOId id;
    std::string ioType; // TODO: Change this to use the enum IOType
    int value;
};

struct Node
{
    NodeId id;
    std::vector<IO> io;
};

struct UI
{
    UIId id;
};

struct ParsedMessage
{
    ParsedMessage( const MessageType msgType )
        : type( msgType )
        , node()
    {}

    MessageType type;
    Node node;
};

} // namespace sn
