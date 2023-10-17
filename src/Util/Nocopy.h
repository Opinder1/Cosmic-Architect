#pragma once

// No copying but movable
class nocopy
{
protected:
	constexpr nocopy() = default;
	~nocopy() = default;

	nocopy(const nocopy&) = delete;
	nocopy& operator=(const nocopy&) = delete;

	nocopy(nocopy&&) = default;
	nocopy& operator=(nocopy&&) = default;
};

// No copying or moving
class nomove
{
protected:
	constexpr nomove() = default;
	~nomove() = default;

	nomove(const nomove&) = delete;
	nomove& operator=(const nomove&) = delete;

	nomove(nomove&&) = delete;
	nomove& operator=(nomove&&) = delete;
};