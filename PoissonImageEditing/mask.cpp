#include <cstring>
#include <algorithm>
#include "mask.h"
using namespace std;


//	four directions
int Mask::dn = 4;
int Mask::dx[4] = {0, 1, 0, -1};
int Mask::dy[4] = {-1, 0, 1, 0};


Mask::Mask(int _width, int _height) {
	width = _width;
	height = _height;
	mask = new bool*[width];
	for (int x = 0; x < width; ++x) {
		mask[x] = new bool[height];
		memset(mask[x], false, height * sizeof(bool));
	}
}

Mask::~Mask() {
	for (int x = 0; x < width; ++x) delete[] mask[x];
	delete[] mask;
}

bool Mask::get(int x, int y) {
	if (x < 0 || x >= width) return false;
	if (y < 0 || y >= height) return false;
	return mask[x][y];
}

void Mask::set(int x, int y, bool b) {
	mask[x][y] = b;
}

void Mask::getSizeInfo() {
	minx = miny = 1<<30;
	maxx = maxy = -1<<30;
	size = 0;
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y)
			if (mask[x][y] == true) {
				minx = min(minx, x);
				miny = min(miny, y);
				maxx = max(maxx, x);
				maxy = max(maxy, y);
				size++;
			}
	cx = (minx + maxx) / 2;
	cy = (miny + maxy) / 2;
}