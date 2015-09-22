#pragma once
#ifndef IMAGEPROCESS_H_
#define IMAGEPROCESS_H_

#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

class ImageProcess
{
public:
	ImageProcess(IplImage *frame);
	~ImageProcess();
	void drawPolyContour(char* windowName, IplImage *frame);
	IplImage *createMaskImage();
	IplImage *combineMaskAreaAndFG(IplImage *areaMaskImage, IplImage *FGImage);
	vector<vector<CvPoint>> getPolyPoint();
	bool checkObjectInMaskArea(Rect BBox, IplImage *combinedImage);
	void getBoundingBox(IplImage *maskImage, vector<Rect>&BBox);
	void saveImage(IplImage *savedImage, char *direction = NULL);
private:
	vector<vector<CvPoint>> polyPoint;
	CvSize imageSize;
};
#endif              //IMAGEPROCESS_H_