//#include "Mandelbrot.h"
//
//unsigned int mandelbrot(long double cr, long double ci)
//{
//	long double zr(0);
//	long double zi(0);
//	long double temp(0);
//	for (int i = 0; i < MAX_ITERATIONS; i++)
//	{
//		temp = zr * zr - zi * zi + cr;
//		zi = 2 * zr * zi + ci;
//		zr = temp;
//		if (zr * zr + zi * zi >= long double(25))
//		{
//			return i;
//		}
//	}
//
//	return NOT_IN_SET;
//}
