#include "CheckerBitmap.h"

#define ASSEMBLE_COLOR(a, r, g, b) ( (a << 24) | (r << 16) | (g << 8) | b )

CheckerBitmap::CheckerBitmap(uint32 size)
	: BBitmap(BRect(0,0, (size * 2) - 1, (size * 2) - 1), B_RGBA32)
{
	uint32 * bits = (uint32 *) Bits();
	uint32 height = 2 * size, width = 2 * size;
	
	uint32 light = ASSEMBLE_COLOR(255, 255, 255, 255);
	uint32 dark = ASSEMBLE_COLOR(255, 230, 230, 230);
	
	for (uint32 y = 0; y < height; y++)
		{
		for (uint32 x = 0; x < width; x++)
			{
			if (	((y < size) && (x < size)) || ((y >= size) && (x >= size))	)
				*bits = light;
			else
				*bits = dark;
				
			bits++;
			};
		};
		
}

CheckerBitmap::~CheckerBitmap()
{
}

