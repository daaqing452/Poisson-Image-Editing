#include <vector>
#include <windows.h>
#include "gsi.h"
#include "mask.h"
#include "pimage.h"
using namespace std;

//	remove command window
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

//	maximum image size
#define MAX_IMAGE_WIDTH 1200
#define MAX_IMAGE_HEIGHT 800

//	blending gradient or blending pixel
#define BLENDING_GRADIENT

//	get coordinate in target using coordinate in source
#define TARX(x) (posx-msk->cx+(x))
#define TARY(y) (posy-msk->cy+(y))

//	blending coefficient
#define SOURCE_RATIO 0.7

//	get inverse of a color
#define INVCOLOR(x) (CV_RGB(255, 255,255)-x)


//	source and target image
PImage *source;
PImage *target;

//	mask of source image
Mask *msk;

//	if editing region is selected
bool ringed = false;

//	if poisson image editing is finished
bool poissonImageEditinged = false;

//	start coordinate of editing region selecting
int ringStartX, ringStartY;

//	last coordinate while editing region selecting
int ringLastX = -1, ringLastY = -1;

//	boundary path of editing region
vector<pair<int,int>> ringPath;

//	if a pixel is visited
bool **visit;

//	BFS queue
pair<int,int> *visitQue;

//	if now connected block include outside 
bool visitOut;

//	pixels of now connected block
vector<pair<int,int>> visitRegion;

//	matrix for id of a pixel while poisson image editing
int **id;

//	color matrix
CvScalar **c;


//	open a file
char* openFile() {
	char szFileName[MAX_PATH] = {0};
	OPENFILENAME openFileName = {0};
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.nMaxFile = MAX_PATH;
	openFileName.lpstrFilter = "图片文件(*.png;*.jpg;*.bmp)\0*.png;*.jpg;*.bmp\0所有文件(*.*)\0*.*\0\0";
	openFileName.lpstrFile = szFileName;
	openFileName.nFilterIndex = 1;
	openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (!::GetOpenFileName(&openFileName)) return NULL;
	char *fileName = new char[999];
	strcpy(fileName, openFileName.lpstrFile);
	return fileName;
}

//	poisson image editing
void poissonImageEditing(int posx, int posy) {

	//	get information of mask
	msk->getSizeInfo();

	//	enumerate channels of RGB
	for (int k = 0; k < 3; ++k) {
		
		//	initialize Gauss-Seidel iteration method
		GSIMethod gsim(msk->size);
		
		//	mark ids to pixels in editing region
		int idCnt = 0;
		for (int x = 0; x < source->width; ++x)
			for (int y = 0; y < source->height; ++y)
				if (msk->get(x, y) == true)
					id[x][y] = idCnt++;

		//	enumerate every pixels in editing region
		for (int x = 0; x < source->width; ++x)
			for (int y = 0; y < source->height; ++y)
				if (msk->get(x, y) == true) {

					//	new an equation
					gsim.addEquation(id[x][y]);

					//	add GSIUnit of now pixel
					gsim.addGSIUnit(id[x][y], 4);

					//	equation right
					double b = 0;

					//	enumerate directions
					for (int d = 0; d < Mask::dn; ++d) {

						//	coordinate of adjacent pixel
						int xx = x + Mask::dx[d];
						int yy = y + Mask::dy[d];

						if (msk->get(xx, yy) == true) {

							//	add GSIUnit if adjacent pixel is in editing region
							gsim.addGSIUnit(id[xx][yy], -1);

							//	calculate right considering blending
#ifdef BLENDING_GRADIENT
							b += ( source->getColor(x, y, k) - source->getColor(xx, yy, k) ) * SOURCE_RATIO;
							b += ( target->getColor(TARX(x), TARY(y), k) - target->getColor(TARX(xx), TARY(yy), k) ) * (1 - SOURCE_RATIO);
#else
							b += source->getColor(x, y, k) - source->getColor(xx, yy, k);
#endif
						} else {

							//	calculate right if adjacent pixel is not in editing region
							b += target->getColor(TARX(x), TARY(y), k);
						}
					}

					//	add right of now equation
					gsim.addB(b);
				}
		
		// solve equations
		gsim.solve(0.1, 0);
		
		//	get colors of pixels in editing region
		for (int x = 0; x < source->width; ++x)
			for (int y = 0; y < source->height; ++y)
				if (msk->get(x, y) == true)
					c[x][y].val[k] = gsim.ans[id[x][y]];
	}

	//	renew target image
	for (int x = 0; x < source->width; ++x)
		for (int y = 0; y < source->height; ++y)
			if (msk->get(x, y)) {
				CvScalar newColor;
				for (int k = 0; k < 3; ++k) newColor.val[k] = c[x][y].val[k];
#ifndef BLENDING_GRADIENT
				newColor = newColor * SOURCE_RATIO + target->getColor(TARX(x), TARY(y)) * (1 - SOURCE_RATIO);
#endif
				target->setColor(TARX(x), TARY(y), newColor);
			}
}

//	draw a line based on two end points
void ringDrawLine(int lx, int ly, int nx, int ny) {
	
	//	out
	if (lx == -1) return;
	
	//	just on point
	if (lx == nx && ly == ny) {
		source->setColor(lx, ly, c[lx][ly]);
		return;
	}
	
	//	compare length of directions of x-axis and y-axis and enumerate the larger one
	if (abs(lx - nx) < abs(ly - ny)) {
		double dx = 1.0 * (nx - lx) / abs(ny - ly);
		int dy = ly < ny ? 1 : -1;
		for (int cnt = 0; ly != ny + dy; ly += dy, cnt++) {
			int tx = (int)(lx + dx * cnt + 0.5);
			source->setColor(tx, ly, c[tx][ly]);
			ringPath.push_back(make_pair(tx, ly));
		}
	} else {
		double dy = 1.0 * (ny - ly) / abs(nx - lx);
		int dx = lx < nx ? 1 : -1;
		for (int cnt = 0; lx != nx + dx; lx += dx, cnt++) {
			int ty = (int)(ly + dy * cnt + 0.5);
			source->setColor(lx, ty, c[lx][ty]);
			ringPath.push_back(make_pair(lx, ty));
		}
	}
}

//	mouse event function of source image
void sourceMouseEvent(int mevent, int posx, int posy, int flags, void *ustc) {
	
	//	exit if poisson image editing is finished
	if (poissonImageEditinged) return;

	//	cancel the ring if right button down
	if (mevent == CV_EVENT_RBUTTONDOWN) {
		
		//	restore the color of ring in source image
		for (int i = 0; i < ringPath.size(); ++i) {
			int x = ringPath[i].first;
			int y = ringPath[i].second;
			source->setColor(x, y, INVCOLOR(c[x][y]));
		}

		//	restore variables
		ringed = false;
		ringLastX = ringLastY = -1;
		ringPath.clear();
		source->show();
		return;
	}

	//	start the ring if left button dwon
	if (!ringed && mevent == CV_EVENT_LBUTTONDOWN) {
		ringStartX = posx;
		ringStartY = posy;
	}

	//	draw lines if left button move
	if (!ringed && flags == CV_EVENT_FLAG_LBUTTON) {
		ringDrawLine(ringLastX, ringLastY, posx, posy);
		ringLastX = posx;
		ringLastY = posy;
		source->show();
	}

	//	draw the line of start and end points if left button up
	if (!ringed && mevent == CV_EVENT_LBUTTONUP) {
		ringDrawLine(posx, posy, ringStartX, ringStartY);
		ringed = true;
		source->show();
	}
}

//	use BFS to find the connected block
void visitBFS(int sx, int sy) {
	int tail = 0;
	visitQue[tail] = make_pair(sx, sy);
	for (int head = 0; head <= tail; ++head) {
		int x = visitQue[head].first;
		int y = visitQue[head].second;
		for (int d = 0; d < Mask::dn; ++d) {
			int xx = x + Mask::dx[d];
			int yy = y + Mask::dy[d];
			if (xx < 0 || xx >= source->width || yy < 0 || yy >= source->height) {
				visitOut = true;
				continue;
			}
			if (msk->get(xx, yy) || visit[xx][yy]) continue;
			visit[xx][yy] = true;
			visitRegion.push_back(make_pair(xx, yy));
			visitQue[++tail] = make_pair(xx, yy);
		}
	}
}

//	mouse event function of target image
void targetMouseEvent(int mevent, int posx, int posy, int flags, void *ustc) {

	//	 start poisson image editing if ringed and left button down
	if (ringed && mevent == CV_EVENT_LBUTTONDOWN) {

		//	judge if editing region out of target image
		int minx = 1<<30, miny = 1<<30;
		int maxx = -1<<30, maxy = -1<<30;
		for (int i = 0; i < ringPath.size(); ++i) {
			int x = ringPath[i].first;
			int y = ringPath[i].second;
			minx = min(minx, x);
			miny = min(miny, y);
			maxx = max(maxx, x);
			maxy = max(maxy, y);
		}
		int cx = (minx + maxx) / 2;
		int cy = (miny + maxy) / 2;

		if (posx - cx + minx <= 0 || posx - cx + maxx >= target->width - 1 || posy - cy + miny <= 0 || posy - cy + maxy >= target->height - 1) {
			::MessageBoxA(NULL, "Region exceeds the boundary!", "Error", 0x00000030);
			return;
		}
		
		//	set boundary in mask
		for (int i = 0; i < ringPath.size(); ++i) {
			msk->set(ringPath[i].first, ringPath[i].second, true);
		}

		//	find every connected blocks inside and set them in mask 
		for (int x = 0; x < source->width; ++x)
			for (int y = 0; y < source->height; ++y) {
				if (!msk->get(x, y) && !visit[x][y]) {
					visitOut = false;
					visitRegion.clear();
					visitBFS(x, y);
					if (!visitOut) {
						for (int i = 0; i < visitRegion.size(); ++i) {
							msk->set(visitRegion[i].first, visitRegion[i].second, true);
						}
					}
				}
			}
		
		//	restore the color of source image
		for (int i = 0; i < ringPath.size(); ++i) {
			int x = ringPath[i].first;
			int y = ringPath[i].second;
			source->setColor(x, y, INVCOLOR(c[x][y]));
		}

		//	poisson image editing
		poissonImageEditing(posx, posy);
		poissonImageEditinged = true;

		//	modify the color of editing region in source image
		for (int x = 0; x < source->width; ++x)
			for (int y = 0; y < source->height; ++y)
				if (msk->get(x, y)) {
					source->setColor(x, y, source->getColor(x, y) * 0.5 + CV_RGB(255, 255, 255) * 0.5);
				}
				
		source->show();
		target->show();
		::MessageBoxA(NULL, "Poisson image editing completed!", "OK", 0x00000040);
	}
}

int main() {
	//	choose file
	::MessageBoxA(NULL, "Choose the source image!", "Begin", 0x00000040);
	char *sourceFileName = openFile();
	if (sourceFileName == NULL) return 0;
	::MessageBoxA(NULL, "Choose the target image!", "Begin", 0x00000040);
	char *targetFileName = openFile();
	if (targetFileName == NULL) return 0;

	//	initialize
	source = new PImage(sourceFileName, "Source");
	target = new PImage(targetFileName, "Target");
	msk = new Mask(source->width, source->height);
	id = new int*[source->width];
	c = new CvScalar*[source->width];
	visit = new bool*[source->width];
	for (int x = 0; x < source->width; ++x) {
		id[x] = new int[source->height];
		memset(id[x], 0, source->height * sizeof(int));
		c[x] = new CvScalar[source->height];
		for (int y = 0; y < source->height; ++y) {
			c[x][y] = INVCOLOR(source->getColor(x, y));
		}
		visit[x] = new bool[source->height];
		memset(visit[x], 0, source->height * sizeof(bool));
	}
	visitQue = new pair<int,int>[source->width * source->height];

	source->createWindow();
	target->createWindow();
	
	cvSetMouseCallback(source->winName.c_str(), sourceMouseEvent, NULL);
	cvSetMouseCallback(target->winName.c_str(), targetMouseEvent, NULL);

	//	matrix c store the inverse color of source image
	for (int x = 0; x < source->width; ++x)
		for (int y = 0; y < source->height; ++y)
			c[x][y] = INVCOLOR(source->getColor(x, y));

	cvWaitKey();

	return 0;
}