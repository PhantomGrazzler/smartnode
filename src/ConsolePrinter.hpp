#pragma once

#include <rang.hpp>

#include <ostream>

namespace sns
{
/*!
    @brief Prints a warning message to the console.
    @param[in] items The items to print with warning styling.
 */
template<typename... T>
void PrintWarning( T&&... items )
{
    ( ( std::cout << rang::fg::yellow << "[WARNING] " ) << ... << items )
        << rang::fg::reset << '\n';
}

/*!
    @brief Prints an error message to the console.
    @param[in] items The items to print with error styling.
 */
template<typename... T>
void PrintError( T&&... items )
{
    ( ( std::cout << rang::fg::red << "[ERROR] " ) << ... << items ) << rang::fg::reset << '\n';
}

/*!
    @brief Prints an informational message to the console.
    @param[in] items The items to print with info styling.
 */
template<typename... T>
void PrintInfo( T&&... items )
{
    ( ( std::cout << rang::fg::cyan << "[INFO] " ) << ... << items ) << rang::fg::reset << '\n';
}

/*!
    @brief Prints a debug message to the console.
    @param[in] items The items to print with debug styling.
 */
template<typename... T>
void PrintDebug( T&&... items )
{
    ( ( std::cout << "[DEBUG] " ) << ... << items ) << '\n';
}

} // namespace sns
