#pragma once

#include "Heatwave.hpp"

namespace util {

// Implements a compile-time FNV-1a string hash
// Thanks to http://altdevblogaday.com/2011/10/27/quasi-compile-time-string-hashing/

/*

template <unsigned int N, unsigned int I>
HW_FORCE_INLINE u32 fnv_hash(const char (&str)[N])
{
	return (fnv_hash<N, I-1>(str) ^ str[I-1]) * 16777619u;
}

template <unsigned int N>
HW_FORCE_INLINE u32 fnv_hash<N, 1>(const char (&str)[N])
{
	return (2166136261u ^ str[0]) * 16777619u;
}

*/

template <unsigned int N>
HW_FORCE_INLINE u32 fnv_hash(const char (&str)[N])
{
	return (((((fnv_hash<N-3>((const char (&)[N-3])str) ^ str[N-3]) * 16777619u) ^ str[N-2]) * 16777619u) ^ str[N-1]) * 16777619u;
}

template<>
HW_FORCE_INLINE u32 fnv_hash<3>(const char (&str)[3])
{
	return (((((2166136261u ^ str[0]) * 16777619u) ^ str[1]) * 16777619u) ^ str[2]) * 16777619u;
}

template<>
HW_FORCE_INLINE u32 fnv_hash<2>(const char (&str)[2])
{
	return (((2166136261u ^ str[0]) * 16777619u) ^ str[1]) * 16777619u;
}

template<>
HW_FORCE_INLINE u32 fnv_hash<1>(const char (&str)[1])
{
	return (2166136261u ^ str[0]) * 16777619u;
}

inline u32 fnv_hash_runtime(const char *str)
{
	u32 hash = 2166136261u;
	do {
		hash = (hash ^ *str) * 16777619u;
	} while (*str++ != '\0');
	return hash;
}

inline u32 fnv_hash_runtime(const char* begin, const char* end) {
	u32 hash = 2166136261u;
	for (; begin != end; ++begin) {
		hash = (hash ^ *begin) * 16777619u;
	}
	return hash * 16777619u;
}

} // namespace util
