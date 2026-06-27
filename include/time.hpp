#ifndef TIME_HPP_INCLUDED
#define TIME_HPP_INCLUDED

namespace rtc
{
// Sync the clock to the current time
bool sync(struct storage::WiFiDetails& wifi);

// Get whether the time has been configured yet
bool ready();

// Get the current time
unsigned long get();
} // namespace rtc

#endif
