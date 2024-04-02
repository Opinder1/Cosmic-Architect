#pragma once

struct Nocopy
{
	Nocopy() {}

	Nocopy(Nocopy&) = delete;
	Nocopy(Nocopy&&) = default;

	Nocopy& operator=(Nocopy&) = delete;
	Nocopy& operator=(Nocopy&&) = default;
};

struct Nomove
{
	Nomove() {}
	
	Nomove(Nomove&) = default;
	Nomove(Nomove&&) = delete;

	Nomove& operator=(Nomove&) = default;
	Nomove& operator=(Nomove&&) = delete;
};