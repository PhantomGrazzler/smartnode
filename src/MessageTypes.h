// This contains a list of all message types that this server can process.

#pragma once

#include <string>
#include <vector>

namespace sns
{

const std::vector<std::string> uiMsgTypes =
{
    "ui_connect",
    "output_update"
};

const std::vector<std::string> nodeMsgTypes =
{
    "node_connect",
    "input_update",
    "heartbeat"
};

const std::vector<std::string> serverMsgTypes =
{
    "error",
    "all_nodes_state"
};

}
