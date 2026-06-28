#ifndef TIME_HPP_INCLUDED
#define TIME_HPP_INCLUDED

#include "storage.hpp"

namespace rtc
{
// Sync the clock to the current time; this is NOT thread safe
bool sync(struct storage::WiFiDetails* wifi, bool print_errors);

// Get whether the time has been configured yet; thread safe
bool ready();

// Get the current time; thread safe
unsigned long get();
} // namespace rtc

#endif
