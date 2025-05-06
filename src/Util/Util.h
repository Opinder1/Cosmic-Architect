#pragma once

#include <string_view>
#include <vector>
#include <chrono>
#include <type_traits>

template<class Type>
struct get_method_class;

template<class Class, class Ret, class... Args>
struct get_method_class<Ret(Class::*)(Args...)>
{
	using type = Class;
};

template<class Type>
struct get_member_class;

template<class Class, class Type>
struct get_member_class<Type Class::*>
{
	using type = Class;
};

template<class Type>
struct get_member_type;

template<class Class, class Type>
struct get_member_type<Type Class::*>
{
	using type = Type;
};

template<class Type>
struct get_method_decl;

template<class Class, class Ret, class... Args>
struct get_method_decl<Ret(Class::*)(Args...)>
{
	using type = Ret(Args...);
};

template<class Class, class Ret, class... Args>
struct get_method_decl<Ret(Class::*)(Args...) const>
{
	using type = Ret(Args...);
};

// Use this clock for timings that should not be effected if the user changes their system time
using Clock = std::chrono::steady_clock;

// Use this clock for real life time based on the users computer time
using SystemClock = std::chrono::system_clock;

using namespace std::chrono_literals;

template<class E>
constexpr std::underlying_type_t<E> to_underlying(E e)
{
	return static_cast<std::underlying_type_t<E>>(e);
}

template<class T, typename U>
std::ptrdiff_t offsetof_member(U T::* member)
{
	return reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<const volatile T*>(NULL)->*member));
}

// Erase an item from a vector fast but only if we don't care about the order
template<class VectorT>
void unordered_erase_it(VectorT& vector, typename VectorT::iterator it)
{
	*it = std::move(vector.back());
	vector.pop_back();
}

template<class VectorT, class ItemT>
void unordered_erase(VectorT& vector, ItemT&& item)
{
	typename VectorT::iterator it = std::find(vector.begin(), vector.end(), item);

	if (it != vector.end())
	{
		unordered_erase_it<VectorT>(vector, it);
	}
}

template<class T>
std::string_view UUIDToData(const T& data)
{
	return std::string_view(reinterpret_cast<const char*>(&data), sizeof(T));
}