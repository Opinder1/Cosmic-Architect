#pragma once

#include <chrono>

// Use this clock for timings that should not be effected if the user changes their system time
using Clock = std::chrono::steady_clock;

// Use this clock for real life time based on the users computer time
using SystemClock = std::chrono::system_clock;

using namespace std::chrono_literals;