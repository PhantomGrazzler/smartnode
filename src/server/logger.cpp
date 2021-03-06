
#include "logger.hpp"

namespace sn
{

void Log( spdlog::level::level_enum level, const std::string& message )
{
    spdlog::get( loggerName )->log( level, message );
}

} // namespace sn
