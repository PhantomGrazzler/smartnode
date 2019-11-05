
#include "FailPrinters.h"

// Standard library
#include <iostream>

namespace sns
{

// Report a failure
void fail(
    const boost::beast::error_code& ec,
    char const* what )
{
    std::cerr << what << ": " << ec.message() << "\n";
}

}
