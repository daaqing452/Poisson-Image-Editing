#include "pimage.h"


PImage::PImage(const char *filename, string winNamePart) {
	image = cvLoadImage(filename);
	width = image->width;
	height = image->height;
	winName = "Poisson Image Editing - " + winNamePart;
}

PImage::~PImage() {
	cvDestroyWindow(winName.c_str());
	cvReleaseImage(&image);
}

void PImage::show() {
	cvShowImage(winName.c_str(), image);
}

void PImage::createWindow() {
	cvNamedWindow(winName.c_str());
	show();
}

CvScalar PImage::getColor(int x, int y) {
	return cvGet2D(image, y, x);
}

double PImage::getColor(int x, int y, int k) {
	return getColor(x, y).val[k];
}

void PImage::setColor(int x, int y, CvScalar c) {
	cvSet2D(image, y, x, c);
}



CvScalar operator + (CvScalar A, CvScalar B) {
	for (int k = 0; k < 3; ++k) A.val[k] += B.val[k];
	return A;
}

CvScalar operator - (CvScalar A, CvScalar B) {
	for (int k = 0; k < 3; ++k) A.val[k] -= B.val[k];
	return A;
}

CvScalar operator * (CvScalar A, double B) {
	for (int k = 0; k < 3; ++k) A.val[k] *= B;
	return A;
}