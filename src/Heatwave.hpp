#pragma once

#include <cstdint>

#ifdef _MSC_VER
#	define HW_FORCE_INLINE __forceinline
#else
#	define HW_FORCE_INLINE inline
#endif // _MSC_VER

namespace hw {
	// Standard sized types
	typedef int8_t s8;
	typedef uint8_t u8;
	typedef int16_t s16;
	typedef uint16_t u16;
	typedef int32_t s32;
	typedef uint32_t u32;
	typedef int64_t s64;
	typedef uint64_t u64;
}
