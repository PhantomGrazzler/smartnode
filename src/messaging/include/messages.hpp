#pragma once

#include "id_types.hpp"
#include "data_types.hpp"

#include <string>

namespace sn
{

static const std::string startOfMessage = "<";
static const std::string endOfMessage = ">";

enum class MessageType : char
{
    NodeConnect = 'c',
    NodeUpdate = 'u',
    Ack = 'a',
    Nak = 'n',
    UiConnect = 'g',
    FullState = 's',
    NodeDisconnect = 'd',
    UiUpdate = 'v'
};

struct ParsedMessage
{
    ParsedMessage( const MessageType msgType )
        : type( msgType )
        , node( Node{} )
        , ui( UI{} )
    {}

    MessageType type;
    Node node;
    UI ui;
};

struct ParsedUiMessage
{
    MessageType type;
    std::vector<Node> nodes;
};

} // namespace sn
