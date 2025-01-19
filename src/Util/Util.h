#pragma once

#include <type_traits>
#include <chrono>

template<class E>
constexpr std::underlying_type_t<E> to_underlying(E e)
{
	return static_cast<std::underlying_type_t<E>>(e);
}

template<class Type>
struct get_method_class;

template<class Class, class Ret, class... Args>
struct get_method_class<Ret(Class::*)(Args...)>
{
	using type = Class;
};

// Use this clock for timings that should not be effected if the user changes their system time
using Clock = std::chrono::steady_clock;

// Use this clock for real life time based on the users computer time
using SystemClock = std::chrono::system_clock;

using namespace std::chrono_literals;