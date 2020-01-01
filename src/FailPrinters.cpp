
#include "FailPrinters.h"

// Third-party
#include <rang.hpp>

// Standard library
#include <iostream>

namespace sns
{

// Report a failure
void fail(
    const boost::beast::error_code& ec,
    char const* what )
{
    std::cerr << rang::fg::red << what << ": " << ec.message() << rang::fg::reset << '\n' ;
}

}
