#include "GenericLoad.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdint.h>
#include <iostream>

#include "WindowsWrapper.h"

#include "CommonDefines.h"
#include "Draw.h"
#include "Ending.h"
#include "PixTone.h"
#include "Sound.h"
#include "Main.h"
#include "File.h"

#include "Sound02.h"

PIXTONEPARAMETER gPtpTable[NUM_PXT] =
{
	{1, 5000, {5, 10.0, 32, 0}, {4, 4.0, 32, 0}, {0, 0.0, 63, 0}, 63, 6, 63, 45, 8, 119, 46},
	{1, 1000, {0, 4.0, 32, 0}, {3, 1.0, 63, 0}, {0, 0.0, 63, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 10000, {0, 30.0, 32, 0}, {3, 1.0, 32, 0}, {0, 0.0, 63, 0}, 0, 19, 44, 111, 13, 198, 9},
	{1, 10000, {5, 2.0, 31, 0}, {3, 1.0, 57, 219}, {0, 2.0, 32, 0}, 0, 19, 44, 111, 13, 198, 9},
	{1, 4000, {5, 0.4, 32, 0}, {3, 1.0, 53, 0}, {0, 0.0, 63, 0}, 12, 19, 63, 111, 21, 198, 18},
	{1, 1000, {1, 12.0, 32, 0}, {2, 1.0, 63, 0}, {0, 0.0, 63, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 1000, {5, 1.0, 32, 0}, {3, 1.0, 63, 0}, {0, 0.0, 63, 0}, 0, 28, 63, 53, 31, 210, 31},
	{1, 1000, {1, 5.0, 32, 0}, {3, 1.0, 63, 0}, {0, 0.0, 0, 0}, 63, 64, 63, 128, 31, 255, 0},
	{1, 3000, {1, 17.0, 34, 0}, {3, 2.0, 40, 0}, {4, 1.0, 31, 0}, 63, 64, 63, 225, 63, 255, 0},
	{1, 6000, {1, 930.0, 22, 0}, {0, 0.7, 53, 0}, {0, 7.0, 32, 0}, 63, 64, 63, 202, 63, 255, 0},
	{1, 6000, {1, 918.0, 23, 0}, {0, 0.7, 53, 0}, {0, 7.0, 32, 0}, 63, 64, 63, 202, 63, 255, 0},
	{1, 10000, {2, 200.0, 32, 0}, {0, 1.0, 51, 0}, {1, 20.0, 31, 0}, 63, 64, 63, 89, 30, 208, 28},
	{1, 10000, {5, 23.0, 16, 0}, {0, 1.0, 58, 0}, {1, 17.0, 32, 0}, 63, 64, 63, 96, 51, 202, 31},
	{1, 20000, {2, 100.0, 21, 0}, {0, 1.0, 46, 0}, {1, 40.0, 32, 0}, 63, 64, 63, 128, 63, 162, 0},
	{1, 20000, {5, 5.0, 21, 0}, {0, 1.0, 51, 0}, {1, 40.0, 32, 0}, 63, 64, 63, 128, 63, 162, 28},
	{1, 10000, {5, 20.0, 23, 0}, {3, 0.7, 26, 235}, {5, 1.0, 9, 0}, 63, 38, 55, 87, 32, 98, 29},
	{1, 2000, {1, 20.0, 32, 0}, {3, 2.0, 63, 0}, {4, 1.0, 29, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 15000, {5, 10.0, 32, 0}, {3, 0.7, 26, 235}, {0, 0.0, 63, 0}, 63, 38, 55, 187, 15, 255, 0},
	{1, 4000, {1, 20.0, 32, 0}, {3, 2.0, 63, 0}, {4, 1.0, 29, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 22000, {5, 6.0, 32, 0}, {3, 0.7, 26, 246}, {0, 0.0, 63, 0}, 63, 38, 55, 187, 15, 255, 0},
	{1, 8000, {0, 20.0, 32, 0}, {3, 2.0, 63, 0}, {4, 1.0, 29, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 10000, {4, 400.0, 13, 0}, {0, 0.8, 63, 0}, {4, 8.0, 31, 0}, 63, 64, 63, 191, 32, 255, 0},
	{1, 10000, {4, 800.0, 14, 0}, {0, 0.8, 63, 0}, {4, 8.0, 30, 125}, 63, 64, 63, 128, 63, 166, 0},
	{1, 5000, {2, 50.0, 39, 0}, {3, 0.5, 40, 217}, {1, 0.0, 32, 0}, 63, 64, 63, 128, 34, 198, 32},
	{1, 5000, {5, 10.0, 39, 0}, {3, 0.5, 24, 217}, {1, 4.0, 32, 0}, 0, 4, 63, 128, 34, 198, 32},
	{1, 40000, {5, 10.0, 32, 0}, {3, 1.0, 32, 241}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 40000, {5, 20.0, 32, 0}, {0, 0.0, 32, 0}, {5, 0.1, 32, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 30000, {2, 400.0, 32, 0}, {3, 0.3, 60, 250}, {0, 20.0, 32, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 10000, {4, 400.0, 13, 0}, {0, 0.8, 63, 0}, {4, 8.0, 50, 0}, 63, 64, 63, 191, 32, 255, 0},
	{1, 10000, {4, 800.0, 5, 0}, {0, 0.8, 63, 0}, {4, 8.0, 63, 125}, 63, 64, 63, 128, 63, 166, 0},
	{1, 6000, {3, 123.0, 32, 0}, {4, 1.0, 16, 222}, {4, 4.0, 37, 0}, 0, 6, 63, 104, 25, 255, 0},
	{1, 4000, {5, 4.5, 32, 0}, {3, 1.0, 46, 102}, {3, 2.5, 47, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 5000, {4, 40.0, 52, 0}, {2, 1.0, 63, 0}, {3, 10.0, 63, 0}, 63, 47, 27, 47, 29, 255, 0},
	{1, 3000, {0, 99.0, 32, 0}, {2, 1.0, 55, 197}, {5, 0.0, 0, 0}, 63, 0, 63, 164, 28, 255, 0},
	{1, 10000, {1, 601.0, 32, 0}, {4, 0.5, 15, 235}, {0, 10.0, 54, 0}, 63, 0, 63, 0, 63, 255, 0},
	{1, 8000, {5, 10.0, 15, 0}, {4, 0.5, 16, 239}, {4, 0.0, 50, 0}, 63, 0, 63, 96, 17, 255, 0},
	{1, 20000, {1, 832.0, 32, 0}, {2, 1.0, 46, 0}, {0, 27.0, 63, 0}, 63, 0, 63, 140, 10, 255, 0},
	{1, 20000, {1, 918.0, 32, 0}, {2, 1.0, 46, 0}, {0, 21.0, 63, 0}, 63, 0, 63, 140, 10, 255, 0},
	{1, 4000, {1, 54.0, 32, 0}, {5, 0.1, 33, 0}, {0, 0.0, 32, 0}, 53, 57, 44, 128, 24, 255, 0},
	{1, 10000, {1, 246.0, 23, 0}, {4, 0.6, 22, 239}, {4, 6.0, 63, 0}, 0, 11, 63, 13, 63, 255, 0},
	{1, 10000, {1, 294.0, 23, 0}, {4, 0.6, 22, 247}, {4, 6.0, 63, 140}, 0, 15, 63, 17, 63, 255, 0},
	{1, 22050, {0, 117.0, 63, 0}, {5, 2.0, 18, 0}, {5, 0.0, 0, 0}, 63, 0, 63, 64, 19, 255, 0},
	{1, 5000, {0, 28.0, 32, 0}, {3, 3.0, 27, 0}, {5, 0.0, 0, 0}, 63, 0, 63, 0, 63, 255, 0},
	{1, 10000, {0, 322.0, 32, 0}, {2, 8.0, 37, 0}, {2, 0.0, 0, 0}, 0, 13, 63, 106, 11, 255, 0},
	{1, 10000, {5, 7.3, 32, 0}, {5, 0.2, 29, 0}, {0, 0.0, 32, 0}, 63, 91, 63, 149, 25, 255, 0},
	{1, 1000, {0, 6.0, 32, 0}, {3, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 20000, {2, 186.0, 32, 0}, {0, 4.0, 13, 98}, {3, 4.0, 5, 0}, 63, 64, 28, 255, 0, 255, 0},
	{1, 20000, {2, 285.0, 19, 0}, {3, 4.0, 21, 0}, {3, 4.0, 33, 130}, 63, 64, 63, 255, 0, 255, 0},
	{1, 10000, {0, 970.0, 32, 0}, {2, 1.0, 35, 195}, {0, 31.0, 31, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 20000, {5, 6.0, 32, 0}, {3, 1.0, 54, 239}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 35},
	{1, 40000, {5, 4.0, 32, 0}, {3, 0.0, 32, 230}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 40000, {1, 238.0, 32, 0}, {3, 1.0, 14, 0}, {4, 30.0, 32, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 3000, {2, 62.0, 32, 0}, {2, 3.0, 63, 0}, {3, 3.0, 14, 0}, 63, 0, 63, 210, 32, 255, 0},
	{1, 5000, {2, 58.0, 32, 0}, {2, 3.0, 63, 0}, {2, 3.0, 32, 0}, 63, 0, 63, 49, 27, 255, 0},
	{1, 3000, {0, 13.0, 24, 0}, {3, 2.0, 40, 0}, {4, 1.0, 31, 0}, 63, 64, 63, 225, 63, 255, 0},
	{1, 3000, {5, 6.0, 32, 0}, {5, 1.0, 32, 0}, {3, 0.0, 0, 0}, 0, 0, 63, 45, 23, 255, 0},
	{1, 20000, {1, 477.0, 40, 0}, {5, 93.0, 39, 0}, {4, 17.0, 19, 0}, 0, 64, 63, 128, 63, 255, 0},
	{1, 6000, {5, 11.0, 32, 0}, {5, 1.0, 32, 0}, {3, 3.0, 32, 0}, 63, 0, 63, 0, 63, 255, 0},
	{1, 6000, {1, 329.0, 20, 0}, {2, 2.0, 47, 77}, {3, 3.0, 63, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 8000, {4, 2000.0, 32, 0}, {2, 1.0, 0, 0}, {0, 0.0, 32, 0}, 43, 21, 7, 255, 0, 255, 0},
	{1, 5000, {1, 231.0, 32, 0}, {4, 1.0, 32, 65}, {3, 2.0, 32, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 3000, {0, 107.0, 32, 0}, {4, 1.0, 15, 0}, {0, 0.0, 17, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 20000, {5, 4.0, 32, 0}, {5, 1.0, 32, 170}, {5, 0.0, 32, 0}, 63, 38, 22, 255, 0, 255, 0},
	{1, 5000, {1, 16.0, 32, 0}, {3, 1.0, 32, 238}, {0, 0.0, 0, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 20000, {5, 4.0, 32, 0}, {0, 0.7, 61, 43}, {3, 1.0, 22, 224}, 63, 64, 63, 204, 46, 255, 0},
	{1, 5000, {0, 880.0, 19, 0}, {0, 0.0, 32, 0}, {0, 8.0, 32, 0}, 0, 11, 63, 34, 25, 255, 0},
	{1, 2000, {5, 3.0, 16, 0}, {3, 2.0, 12, 0}, {3, 1.0, 37, 0}, 0, 51, 63, 132, 24, 255, 0},
	{1, 22050, {1, 400.0, 32, 0}, {0, 20.0, 10, 0}, {3, 10.0, 8, 0}, 0, 6, 63, 60, 21, 255, 0},
	{1, 2000, {5, 2.0, 32, 0}, {2, 3.0, 54, 0}, {0, 0.0, 32, 0}, 0, 17, 63, 98, 22, 255, 0},
	{1, 8000, {1, 814.0, 32, 0}, {2, 11.0, 32, 0}, {3, 16.0, 32, 0}, 63, 23, 63, 74, 12, 255, 0},
	{1, 10000, {5, 21.0, 21, 0}, {0, 5.0, 32, 178}, {0, 3.0, 33, 181}, 63, 38, 63, 104, 20, 255, 0},
	{1, 6000, {5, 1.0, 28, 0}, {3, 6.0, 56, 0}, {0, 8.0, 32, 0}, 63, 57, 63, 98, 20, 255, 0},
	{1, 4000, {5, 6.0, 32, 0}, {3, 2.0, 32, 0}, {3, 2.0, 32, 0}, 63, 26, 30, 66, 29, 255, 0},
	{1, 22050, {5, 711.0, 32, 0}, {5, 7.0, 32, 0}, {0, 0.0, 32, 0}, 0, 13, 0, 17, 63, 255, 0},
	{1, 2000, {5, 2.0, 32, 0}, {5, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 62050, {5, 40.0, 32, 0}, {5, 1.0, 32, 0}, {5, 0.0, 0, 0}, 0, 17, 63, 36, 63, 255, 0},
	{1, 8000, {0, 77.0, 32, 0}, {3, 3.0, 56, 189}, {0, 0.0, 17, 0}, 0, 38, 63, 140, 28, 255, 0},
	{1, 8000, {5, 8.0, 32, 0}, {3, 3.0, 54, 189}, {3, 3.0, 32, 0}, 63, 38, 42, 140, 21, 255, 0},
	{1, 9050, {5, 9.0, 26, 0}, {0, 1.0, 32, 209}, {0, 0.0, 32, 0}, 63, 64, 63, 132, 63, 255, 0},
	{1, 9050, {2, 43.0, 32, 0}, {3, 1.0, 47, 172}, {0, 0.0, 32, 0}, 63, 64, 63, 198, 63, 255, 0},
	{1, 22050, {1, 754.0, 32, 0}, {2, 0.5, 14, 126}, {4, 18.0, 17, 0}, 0, 64, 63, 128, 63, 255, 0},
	{1, 22050, {0, 597.0, 12, 0}, {2, 0.5, 14, 126}, {4, 18.0, 17, 0}, 0, 64, 40, 128, 41, 255, 0},
	{1, 6000, {2, 217.0, 32, 0}, {0, 0.7, 16, 0}, {3, 0.0, 32, 0}, 0, 32, 63, 49, 15, 255, 0},
	{1, 5000, {5, 1.0, 32, 0}, {3, 1.0, 32, 0}, {0, 0.0, 32, 0}, 0, 64, 63, 81, 28, 255, 0},
	{1, 10000, {5, 11.0, 32, 0}, {2, 4.0, 32, 0}, {0, 3.0, 32, 0}, 0, 64, 63, 128, 30, 255, 0},
	{1, 10000, {5, 1.0, 32, 0}, {4, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 32, 128, 33, 255, 0},
	{1, 10000, {2, 168.0, 32, 0}, {0, 0.5, 29, 173}, {0, 0.0, 32, 0}, 0, 13, 63, 68, 35, 255, 0},
	{1, 10000, {2, 50.0, 32, 0}, {0, 0.5, 29, 173}, {2, 100.0, 32, 0}, 0, 13, 63, 68, 35, 255, 0},
	{1, 4000, {5, 11.0, 25, 0}, {3, 3.0, 32, 0}, {0, 0.0, 32, 0}, 0, 9, 63, 128, 14, 255, 0},
	{1, 4000, {5, 3.0, 27, 0}, {3, 3.0, 32, 0}, {0, 0.0, 32, 0}, 0, 9, 63, 128, 14, 255, 0},
	{1, 3000, {4, 9.0, 20, 0}, {3, 3.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 3000, {4, 18.0, 20, 0}, {3, 3.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 0},
	{1, 3000, {5, 4.0, 32, 0}, {3, 1.0, 32, 0}, {0, 0.0, 32, 0}, 0, 64, 63, 128, 63, 255, 0},
	{1, 12000, {5, 2.0, 32, 0}, {3, 1.7, 53, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 162, 25, 255, 0},
	{1, 12000, {3, 77.0, 32, 0}, {3, 1.0, 61, 200}, {0, 19.0, 22, 0}, 63, 64, 63, 172, 25, 255, 0},
	{1, 5000, {0, 16.0, 63, 0}, {3, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 166, 35, 255, 0},
	{1, 1000, {5, 1.0, 16, 0}, {0, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 91, 28, 255, 0},
	{1, 5000, {0, 20.0, 30, 0}, {3, 1.0, 44, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 111, 19, 255, 0},
	{1, 10000, {5, 14.0, 41, 0}, {5, 3.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 18, 91, 12, 255, 0},
	{1, 1000, {5, 48.0, 30, 0}, {5, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 166, 27, 255, 0},
	{1, 10000, {5, 48.0, 30, 0}, {5, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 43, 166, 41, 255, 7},
	{1, 4000, {5, 35.0, 30, 0}, {3, 35.0, 32, 0}, {0, 0.0, 32, 0}, 63, 53, 21, 166, 13, 255, 0},
	{1, 10000, {1, 63.0, 32, 0}, {3, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 39, 91, 20, 255, 0},
	{1, 22050, {5, 52.0, 22, 0}, {5, 1.0, 32, 0}, {0, 0.0, 32, 0}, 3, 23, 23, 57, 10, 255, 0},
	{1, 22050, {5, 80.0, 22, 0}, {5, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 23, 23, 57, 10, 255, 0},
	{1, 44100, {5, 54.0, 12, 0}, {5, 1.0, 32, 0}, {5, 1.0, 27, 0}, 40, 64, 63, 128, 38, 255, 63},
	{1, 44100, {5, 119.0, 9, 0}, {5, 2.0, 32, 0}, {0, 0.0, 32, 0}, 61, 64, 39, 128, 60, 255, 28},
	{1, 4000, {5, 11.0, 32, 0}, {5, 1.0, 17, 197}, {0, 0.0, 32, 0}, 51, 100, 63, 100, 0, 255, 0},
	{1, 4000, {5, 21.0, 32, 0}, {5, 1.0, 17, 197}, {0, 0.0, 32, 0}, 0, 119, 0, 117, 63, 255, 0},
	{1, 5000, {2, 143.0, 25, 0}, {3, 0.5, 40, 217}, {1, 0.0, 32, 0}, 63, 64, 63, 128, 34, 198, 32},
	{1, 5000, {5, 10.0, 23, 0}, {3, 0.5, 24, 217}, {1, 4.0, 32, 0}, 0, 4, 63, 128, 34, 198, 32},
	{1, 4000, {5, 6.0, 32, 0}, {3, 2.0, 32, 0}, {3, 2.0, 32, 0}, 63, 26, 30, 66, 29, 255, 0},
	{1, 4000, {0, 150.0, 32, 0}, {0, 0.0, 32, 0}, {3, 2.0, 32, 0}, 63, 26, 30, 66, 29, 255, 0},
	{1, 40050, {4, 100.0, 32, 0}, {3, 8.0, 22, 0}, {3, 8.0, 8, 0}, 63, 28, 44, 81, 23, 255, 0},
	{1, 40050, {4, 150.0, 32, 0}, {3, 8.0, 22, 0}, {3, 8.0, 8, 0}, 63, 28, 44, 81, 23, 255, 0},
	{1, 40050, {4, 294.0, 32, 0}, {3, 8.0, 22, 0}, {3, 8.0, 8, 0}, 63, 28, 44, 81, 23, 255, 0},
	{1, 5000, {3, 621.0, 32, 0}, {2, 2.0, 22, 0}, {3, 6.0, 32, 0}, 0, 4, 63, 77, 30, 255, 0},
	{1, 30000, {5, 28.0, 44, 0}, {3, 1.0, 45, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 208, 63, 255, 0},
	{1, 30000, {2, 101.0, 44, 0}, {2, 0.5, 63, 118}, {0, 0.0, 32, 0}, 63, 64, 63, 176, 63, 255, 0},
	{1, 30000, {2, 86.0, 44, 0}, {2, 0.5, 63, 118}, {0, 0.0, 32, 0}, 63, 64, 63, 176, 63, 255, 0},
	{1, 2000, {5, 1.0, 32, 0}, {0, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 2000, {3, 99.0, 12, 0}, {3, 1.0, 32, 0}, {0, 0.0, 32, 0}, 63, 64, 63, 128, 63, 255, 63},
	{1, 6000, {3, 388.0, 22, 0}, {0, 0.7, 53, 0}, {0, 7.0, 32, 0}, 25, 64, 63, 202, 63, 255, 0},
	{1, 6000, {1, 918.0, 23, 0}, {0, 0.7, 53, 0}, {0, 7.0, 32, 0}, 63, 64, 63, 202, 63, 255, 0},
	{1, 10000, {5, 118.0, 19, 0}, {3, 1.0, 63, 0}, {0, 3.0, 32, 0}, 0, 19, 63, 66, 41, 255, 0},
	{1, 10000, {5, 6.0, 24, 0}, {3, 1.0, 32, 208}, {2, 8.0, 12, 0}, 63, 64, 63, 128, 38, 255, 0},
	{1, 400, {1, 20.0, 12, 0}, {0, 0.0, 32, 0}, {0, 0.0, 32, 0}, 0, 43, 63, 193, 63, 255, 0},
	{1, 400, {1, 30.0, 12, 0}, {0, 0.0, 32, 0}, {0, 0.0, 32, 0}, 0, 43, 63, 193, 63, 255, 0},
	{1, 400, {1, 40.0, 12, 0}, {0, 0.0, 32, 0}, {0, 0.0, 32, 0}, 0, 43, 63, 193, 63, 255, 0},
	{1, 8000, {3, 800.0, 24, 0}, {0, 8.0, 4, 0}, {0, 0.0, 32, 0}, 0, 13, 63, 138, 63, 255, 0},
	{1, 8000, {5, 53.0, 12, 0}, {5, 1.0, 32, 0}, {0, 0.0, 32, 0}, 0, 0, 63, 68, 63, 255, 0},
	{1, 8000, {3, 400.0, 24, 0}, {0, 8.0, 4, 0}, {0, 0.0, 32, 0}, 0, 13, 63, 138, 63, 255, 0},
	{1, 8000, {5, 53.0, 12, 0}, {5, 1.0, 32, 0}, {0, 0.0, 32, 0}, 0, 0, 63, 68, 63, 255, 0},
	{1, 8000, {3, 200.0, 32, 0}, {0, 8.0, 4, 0}, {0, 0.0, 32, 0}, 0, 13, 63, 138, 63, 255, 0},
	{1, 8000, {5, 25.0, 17, 0}, {3, 1.0, 32, 0}, {0, 0.0, 32, 0}, 0, 0, 63, 68, 63, 255, 0},
	{1, 8000, {4, 800.0, 32, 0}, {4, 2.0, 21, 0}, {4, 8.0, 32, 0}, 0, 106, 63, 130, 27, 255, 0},
	{1, 3000, {4, 31.0, 10, 0}, {3, 2.0, 40, 0}, {4, 1.0, 31, 0}, 0, 9, 63, 94, 63, 255, 0},
	{1, 5000, {2, 168.0, 32, 0}, {0, 0.5, 29, 173}, {0, 0.0, 32, 0}, 0, 13, 63, 68, 35, 255, 0},
	{1, 1000, {0, 20.0, 0, 0}, {0, 0.0, 0, 0}, {0, 0.0, 0, 0}, 0, 64, 0, 128, 0, 255, 0}
};

BOOL LoadSurfaces(void)
{
	if (!MakeSurface_File("Nicalis", SURFACE_ID_NICALIS))
		return FALSE;
	if (!MakeSurface_File("MyChar", SURFACE_ID_MY_CHAR))
		return FALSE;
	if (!MakeSurface_File("Title", SURFACE_ID_TITLE))
		return FALSE;
	if (!MakeSurface_File("ArmsImage", SURFACE_ID_ARMS_IMAGE))
		return FALSE;
	if (!MakeSurface_File("UI", SURFACE_ID_UI))
		return FALSE;
	if (!MakeSurface_File("Arms", SURFACE_ID_ARMS))
		return FALSE;
	if (!MakeSurface_File("ItemImage", SURFACE_ID_ITEM_IMAGE))
		return FALSE;
	if (!MakeSurface_File("StageImage", SURFACE_ID_STAGE_ITEM))
		return FALSE;
	if (!MakeSurface_File("Npc/NpcSym", SURFACE_ID_NPC_SYM))
		return FALSE;
	if (!MakeSurface_File("Npc/NpcRegu", SURFACE_ID_NPC_REGU))
		return FALSE;
	if (!MakeSurface_File("TextBox", SURFACE_ID_TEXT_BOX))
		return FALSE;
	if (!MakeSurface_File("Caret", SURFACE_ID_CARET))
		return FALSE;
	if (!MakeSurface_File("Bullet", SURFACE_ID_BULLET))
		return FALSE;
	if (!MakeSurface_File("Face", SURFACE_ID_FACE))
		return FALSE;
	if (!MakeSurface_File("Fade", SURFACE_ID_FADE))
		return FALSE;
	if (!MakeSurface_Resource("CREDIT01", SURFACE_ID_CREDITS_IMAGE))
		return FALSE;
	return TRUE;
}

void MakeGenericSurfaces()
{
	MakeSurface_Generic(WINDOW_WIDTH, WINDOW_HEIGHT, SURFACE_ID_SCREEN_GRAB, TRUE);
	MakeSurface_Generic(WINDOW_WIDTH, WINDOW_HEIGHT, SURFACE_ID_LEVEL_BACKGROUND, FALSE);
	MakeSurface_Generic(WINDOW_WIDTH, WINDOW_HEIGHT, SURFACE_ID_MAP, TRUE);
	MakeSurface_Generic(320, 240, SURFACE_ID_CASTS, FALSE);
	MakeSurface_Generic(256, 256, SURFACE_ID_LEVEL_TILESET, FALSE);
	MakeSurface_Generic(40, 240, SURFACE_ID_VALUE_VIEW, FALSE);
	MakeSurface_Generic(320, 240, SURFACE_ID_LEVEL_SPRITESET_1, FALSE);
	MakeSurface_Generic(320, 240, SURFACE_ID_LEVEL_SPRITESET_2, FALSE);
}

void LoadSounds()
{
	MakePixToneObject(&gPtpTable[0], 2, 32);
	MakePixToneObject(&gPtpTable[2], 2, 33);
	MakePixToneObject(&gPtpTable[4], 2, 34);
	MakePixToneObject(&gPtpTable[6], 1, 15);
	MakePixToneObject(&gPtpTable[7], 1, 24);
	MakePixToneObject(&gPtpTable[8], 1, 23);
	MakePixToneObject(&gPtpTable[9], 2, 50);
	MakePixToneObject(&gPtpTable[11], 2, 51);
	MakePixToneObject(&gPtpTable[33], 1, 1);
	//MakePixToneObject(&gPtpTable[38], 1, 2);
	MakePixToneObject(&gPtpTable[38], 2, 2);
	MakePixToneObject(&gPtpTable[56], 1, 29);
	MakePixToneObject(&gPtpTable[61], 1, 43);
	MakePixToneObject(&gPtpTable[62], 3, 44);
	MakePixToneObject(&gPtpTable[65], 1, 45);
	MakePixToneObject(&gPtpTable[66], 1, 46);
	MakePixToneObject(&gPtpTable[68], 1, 47);
	MakePixToneObject(&gPtpTable[49], 3, 35);
	MakePixToneObject(&gPtpTable[52], 3, 39);
	MakePixToneObject(&gPtpTable[13], 2, 52);
	MakePixToneObject(&gPtpTable[28], 2, 53);
	MakePixToneObject(&gPtpTable[15], 2, 70);
	MakePixToneObject(&gPtpTable[17], 2, 71);
	MakePixToneObject(&gPtpTable[19], 2, 72);
	MakePixToneObject(&gPtpTable[30], 1, 5);
	MakePixToneObject(&gPtpTable[32], 1, 11);
	MakePixToneObject(&gPtpTable[35], 1, 4);
	MakePixToneObject(&gPtpTable[46], 2, 25);
	MakePixToneObject(&gPtpTable[48], 1, 27);
	MakePixToneObject(&gPtpTable[54], 2, 28);
	MakePixToneObject(&gPtpTable[39], 1, 14);
	MakePixToneObject(&gPtpTable[23], 2, 16);
	MakePixToneObject(&gPtpTable[25], 3, 17);
	MakePixToneObject(&gPtpTable[34], 1, 18);
	MakePixToneObject(&gPtpTable[36], 2, 20);
	MakePixToneObject(&gPtpTable[31], 1, 22);
	MakePixToneObject(&gPtpTable[41], 2, 26);
	MakePixToneObject(&gPtpTable[43], 1, 21);
	MakePixToneObject(&gPtpTable[44], 2, 12);
	MakePixToneObject(&gPtpTable[57], 2, 38);
	MakePixToneObject(&gPtpTable[59], 1, 31);
	MakePixToneObject(&gPtpTable[60], 1, 42);
	MakePixToneObject(&gPtpTable[69], 1, 48);
	MakePixToneObject(&gPtpTable[70], 2, 49);
	MakePixToneObject(&gPtpTable[72], 1, 100);
	MakePixToneObject(&gPtpTable[73], 3, 101);
	MakePixToneObject(&gPtpTable[76], 2, 54);
	MakePixToneObject(&gPtpTable[78], 2, 102);
	MakePixToneObject(&gPtpTable[80], 2, 103);
	MakePixToneObject(&gPtpTable[81], 1, 104);
	MakePixToneObject(&gPtpTable[82], 1, 105);
	MakePixToneObject(&gPtpTable[83], 2, 106);
	MakePixToneObject(&gPtpTable[85], 1, 107);
	MakePixToneObject(&gPtpTable[86], 1, 30);
	MakePixToneObject(&gPtpTable[87], 1, 108);
	MakePixToneObject(&gPtpTable[88], 1, 109);
	MakePixToneObject(&gPtpTable[89], 1, 110);
	MakePixToneObject(&gPtpTable[90], 1, 111);
	MakePixToneObject(&gPtpTable[91], 1, 112);
	MakePixToneObject(&gPtpTable[92], 1, 113);
	MakePixToneObject(&gPtpTable[93], 2, 114);
	MakePixToneObject(&gPtpTable[95], 2, 150);
	MakePixToneObject(&gPtpTable[97], 2, 151);
	MakePixToneObject(&gPtpTable[99], 1, 152);
	MakePixToneObject(&gPtpTable[100], 1, 153);
	MakePixToneObject(&gPtpTable[101], 2, 154);
	MakePixToneObject(&gPtpTable[111], 2, 155);
	MakePixToneObject(&gPtpTable[95], 2, 156);
	MakePixToneObject(&gPtpTable[97], 2, 157);
	MakePixToneObject(&gPtpTable[103], 2, 56);
	MakePixToneObject(&gPtpTable[105], 2, 40);
	MakePixToneObject(&gPtpTable[105], 2, 41);
	MakePixToneObject(&gPtpTable[107], 2, 37);
	MakePixToneObject(&gPtpTable[109], 2, 57);
	MakePixToneObject(&gPtpTable[113], 3, 115);
	MakePixToneObject(&gPtpTable[116], 1, 104);
	MakePixToneObject(&gPtpTable[117], 3, 116);
	MakePixToneObject(&gPtpTable[120], 2, 58);
	MakePixToneObject(&gPtpTable[122], 2, 55);
	MakePixToneObject(&gPtpTable[124], 2, 117);
	MakePixToneObject(&gPtpTable[126], 1, 59);
	MakePixToneObject(&gPtpTable[127], 1, 60);
	MakePixToneObject(&gPtpTable[128], 1, 61);
	MakePixToneObject(&gPtpTable[129], 2, 62);
	MakePixToneObject(&gPtpTable[131], 2, 63);
	MakePixToneObject(&gPtpTable[133], 2, 64);
	MakePixToneObject(&gPtpTable[135], 1, 65);
	MakePixToneObject(&gPtpTable[136], 1, 3);
	MakePixToneObject(&gPtpTable[137], 1, 6);
	MakePixToneObject(&gPtpTable[138], 1, 7);
	
	if (lpSECONDARYBUFFER[2] != NULL)
		AudioBackend_DestroySound(lpSECONDARYBUFFER[2]);
	lpSECONDARYBUFFER[2] = AudioBackend_CreateSound(22050, sound02_data, sound02_size);
}
