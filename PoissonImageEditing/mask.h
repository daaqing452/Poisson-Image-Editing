#ifndef MASK_H
#define MASK_H


//	describing editing region image class
class Mask {
public:

	//	directions and every steps
	static int dn;
	static int dx[4];
	static int dy[4];

	//	matrix of mask
	bool **mask;

	//	width and height of source image
	int width, height;

	//	minimum and maximum x and y coordinate in editing region
	int minx, miny, maxx, maxy;
	
	//	center coordinate of editing region
	int cx, cy;

	//	pixels number in editing region
	int size;

	//	constructor and destructor
	Mask(int , int );
	~Mask();

	//	get in or not in editing region of a pixel
	bool get(int, int);

	//	set in or not in editing region of a pixel
	void set(int, int, bool);

	//	get mask information
	void getSizeInfo();
};


#endif