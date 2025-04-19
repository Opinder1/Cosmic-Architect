#pragma once

// structs that can be inherited from to make objects non copyable or movable

struct Nocopy
{
	Nocopy() {}

	Nocopy(const Nocopy&) = delete;
	Nocopy(Nocopy&&) = default;

	Nocopy& operator=(const Nocopy&) = delete;
	Nocopy& operator=(Nocopy&&) = default;
};

struct Nomove
{
	Nomove() {}
	
	Nomove(const Nomove&) = default;
	Nomove(Nomove&&) = delete;

	Nomove& operator=(const Nomove&) = default;
	Nomove& operator=(Nomove&&) = delete;
};