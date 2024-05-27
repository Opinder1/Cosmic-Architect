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