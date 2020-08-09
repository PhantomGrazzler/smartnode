#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h> // to allow picking up overloaded << operators for custom types.

#include <string>

namespace sns
{
const std::string loggerName( "snsLogger" );

template<typename... Args>
void Log( spdlog::level::level_enum level, spdlog::string_view_t fmt, Args&&... args )
{
    spdlog::get( loggerName )->log( level, fmt, std::forward<Args>( args )... );
}

void Log( spdlog::level::level_enum level, const std::string& message );

} // namespace sns
