#ifndef PIMAGE_H
#define PIMAGE_H


#include <string>
#include <opencv2/opencv.hpp>
using namespace std;


//	image package class
class PImage {
public:

	//	window name
	string winName;

	//	image
	IplImage *image;

	//	width and height of image
	int width, height;

	//	constructor and destructor
	PImage(const char *, string);
	~PImage();

	//	show image in window
	void show();

	//	create window
	void createWindow();

	//	get color of a pixel
	CvScalar getColor(int, int);

	//	get a color channel of RGB of a pixel
	double getColor(int, int, int);

	//	set color of a pixel
	void setColor(int, int, CvScalar);
};


//	CvScalar operations
CvScalar operator + (CvScalar , CvScalar );
CvScalar operator - (CvScalar , CvScalar );
CvScalar operator * (CvScalar , double );


#endif