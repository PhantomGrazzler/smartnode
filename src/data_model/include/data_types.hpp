#pragma once

#include <vector>
#include <string>

namespace sn
{

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
    IO( const IOId ioId, const std::string& ioType, const int ioValue )
        : id( ioId )
        , type( ioType )
        , value( ioValue )
    {}

    IO( const IO& other )
    {
        id = other.id;
        value = other.value;

        if ( type != "Existing" )
        {
            type = other.type;
        }
    }

    IO& operator=( IO rhs )
    {
        id = rhs.id;
        value = rhs.value;

        if ( rhs.type != "Existing" )
        {
            type = rhs.type;
        }

        return *this;
    }

    IOId id;
    std::string type; // TODO: Change this to use the enum IOType
    int value;
};

struct Node
{
    Node()
        : id( invalid_node_id )
        , io()
    {}

    Node( const NodeId& nodeId )
        : id( nodeId )
        , io()
    {}

    Node( const NodeId& nodeId, const std::vector<IO>& ios )
        : id( nodeId )
        , io( ios )
    {}

    NodeId id;
    std::vector<IO> io;
};

struct UI
{
    UIId id;
};

} // namespace sn
