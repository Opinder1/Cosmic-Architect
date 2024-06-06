#pragma once

#include <algorithm>

template<template<class...> class C, class T, class I>
void VectorErase(C<T>& vector, const I& item)
{
	auto it = std::find(vector.begin(), vector.end(), item);

	if (it != vector.end())
	{
		vector.erase(it);
	}
}

template<template<class...> class C, class T>
void UnorderedErase(C<T>& vector, C<T>::iterator it)
{
	std::swap(*it, vector.back());

	vector.pop_back();
}

template<template<class...> class C, class T, class I>
void UnorderedErase(C<T>& vector, const I& item)
{
	auto it = std::find(vector.begin(), vector.end(), item);

	if (it != vector.end())
	{
		UnorderedErase(vector, it);
	}
}