#pragma once

#include <cstdint>

typedef unsigned int  uint;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef __int64       int64;

// Some useful constants
static const size_t KILOBYTE = 1024;
static const size_t MEGABYTE = KILOBYTE * 1024;
static const size_t GIGABYTE = MEGABYTE * 1024;

class NON_COPYABLE
{
private:
	NON_COPYABLE() = default;
	~NON_COPYABLE() = default;

	NON_COPYABLE(const NON_COPYABLE& rhs) = delete;
	NON_COPYABLE& operator=(const NON_COPYABLE& rhs) = delete;
};