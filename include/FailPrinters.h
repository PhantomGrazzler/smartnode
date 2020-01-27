#pragma once

// Third-party
#include <boost/beast/core/error.hpp>

namespace sns
{

// Report a failure
void fail(
    const boost::beast::error_code& ec,
    char const* what );

}
