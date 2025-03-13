#pragma once

namespace Enola
{
	typedef struct
	{
		int x, y, w, h;
		int IsIn(int _x, int _y)
		{
			return _x >= x && _x <= x + w && _y >= y && _y <= y + h;
		}
	}Rectangle;

	typedef struct
	{
		int x, y;
	}Positive;
}