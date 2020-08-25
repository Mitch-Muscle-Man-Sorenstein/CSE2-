#pragma once
#include "SDL_endian.h"

#pragma pack(push)
#pragma pack(1)
template <typename T, typename TU, typename TL>
union FixedPoint
{
	#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		struct
		{
			TL lower;
			TU upper;
		} fixed;
	#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
		struct
		{
			TU upper;
			TL lower;
		} fixed;
	#endif
	T value;
};
#pragma pack(pop)
