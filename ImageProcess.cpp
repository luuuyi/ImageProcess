#include <opencv2\opencv.hpp>
#include "ImageProcess.h"

#define MAX_POLY_POINT_NUMBER 20
#define MAX_POLY_NUMBER 10

using namespace std;
using namespace cv;

static vector<vector<CvPoint>> tmpMouseClickPoint;
static char *windowNameForMouseOperate;
static bool increaseVectorFlag = true;
static bool moveFlag = false;

/*********************************************************************
mouseDrawContour--鼠标控制事件函数
*********************************************************************/
void mouseDrawContour(int event, int x, int y, int flags, void *param)
{
	bool moveIndex = false;
	CvPoint clickPoint;
	IplImage *tmpMouseFrame = NULL;
	CvPoint pt1, pt2;

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		tmpMouseFrame = (IplImage *)cvClone(param);
		clickPoint.x = x;
		clickPoint.y = y;
		vector<CvPoint> tmpVector;
		tmpVector.push_back(clickPoint);
		if (increaseVectorFlag)
		{
			tmpMouseClickPoint.push_back(tmpVector);
		}
		else
		{
			tmpMouseClickPoint[tmpMouseClickPoint.size()-1].push_back(clickPoint);
		}

		for (int i = 0; (i < tmpMouseClickPoint[tmpMouseClickPoint.size()-1].size() - 1) && (tmpMouseClickPoint[tmpMouseClickPoint.size()-1].size() > 1); i++)
		{
			pt1 = tmpMouseClickPoint[tmpMouseClickPoint.size()-1][i];
			pt2 = tmpMouseClickPoint[tmpMouseClickPoint.size()-1][i+1];
			cvLine(tmpMouseFrame, pt1, pt2, CV_RGB(255, 0, 0), 1, 8, 0);
		}

		increaseVectorFlag = false;
		moveFlag = true;
		cvShowImage(windowNameForMouseOperate, tmpMouseFrame);
		cvReleaseImage(&tmpMouseFrame);
	}

	if (event == CV_EVENT_MOUSEMOVE && moveFlag)
	{
		tmpMouseFrame = (IplImage *)cvClone(param);
		for (int i = 0; (i < tmpMouseClickPoint[tmpMouseClickPoint.size()-1].size() - 1) && (tmpMouseClickPoint[tmpMouseClickPoint.size()-1].size() > 1); i++)
		{
			pt1 = tmpMouseClickPoint[tmpMouseClickPoint.size()-1][i];
			pt2 = tmpMouseClickPoint[tmpMouseClickPoint.size()-1][i+1];
			cvLine(tmpMouseFrame, pt1, pt2, CV_RGB(255, 0, 0), 1, 8, 0);
		}
		cvLine(tmpMouseFrame, tmpMouseClickPoint[tmpMouseClickPoint.size()-1][tmpMouseClickPoint[tmpMouseClickPoint.size()-1].size() - 1], cvPoint(x, y), CV_RGB(255, 0, 0), 1, 8, 0);
		cvLine(tmpMouseFrame, tmpMouseClickPoint[tmpMouseClickPoint.size()-1][0], cvPoint(x, y), CV_RGB(255, 0, 0), 1, 8, 0);

		cvShowImage(windowNameForMouseOperate, tmpMouseFrame);
		cvReleaseImage(&tmpMouseFrame);
	}

	if (event == CV_EVENT_RBUTTONDOWN && moveFlag)
	{
		increaseVectorFlag = true;
		moveFlag = false;
	}
}

ImageProcess::ImageProcess(IplImage *frame)
{
	imageSize = cvGetSize(frame);
}

ImageProcess::~ImageProcess()
{

}

/*********************************************************************
drawPolyContour--画多边形区域
*********************************************************************/
void ImageProcess::drawPolyContour(char* windowName, IplImage *frame)
{
	char keyCode;
	bool keyValueCorrect = false;
	IplImage *frameTmp = NULL;
	windowNameForMouseOperate = windowName;

	frameTmp = (IplImage *)cvClone(frame);
	cvShowImage(windowName, frameTmp);
	cvSetMouseCallback(windowName, mouseDrawContour, (void *)frameTmp);

	keyCode = cvWaitKey(0);
	while (!keyValueCorrect)
	{
		if (keyCode == '\n' || keyCode == '\r' || keyCode == '\n\r')
		{
			polyPoint = tmpMouseClickPoint;
			keyValueCorrect = true;
		}
		else
		{
			keyCode = cvWaitKey(0);
		}
	}

	cvSetMouseCallback(windowName, NULL, NULL);
	cvReleaseImage(&frameTmp);
}

/*********************************************************************
getPolyPoint--返回多边形区域各点的坐标
*********************************************************************/
vector<vector<CvPoint>> ImageProcess::getPolyPoint()
{
	return polyPoint;

}

/*********************************************************************
createMaskImage--生成一副区域掩膜图像并返回，图像为单通道二值化图像
*********************************************************************/
IplImage *ImageProcess::createMaskImage()
{
	IplImage *maskAreaImageColor = NULL;
	IplImage *maskAreaImageGray = NULL;
	CvPoint **polyPointTmp = new CvPoint* [MAX_POLY_NUMBER];
	int npts[MAX_POLY_NUMBER];
	int polyNum;

	for (int i = 0; i < MAX_POLY_NUMBER; i++)
	{
		polyPointTmp[i] = new CvPoint[MAX_POLY_POINT_NUMBER];
	}

	for (int i = 0; i < polyPoint.size(); i++)
	{
		npts[i] = polyPoint[i].size();
		for (int j = 0; j  < polyPoint[i].size(); j ++)
		{
			polyPointTmp[i][j] = polyPoint[i][j];
		}
	}
	polyNum = polyPoint.size();

	maskAreaImageColor = cvCreateImage(imageSize, IPL_DEPTH_8U, 3);
	maskAreaImageGray = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);
	cvFillPoly(maskAreaImageColor, polyPointTmp, npts, polyNum, CV_RGB(255, 255, 255), 8);
	cvCvtColor(maskAreaImageColor, maskAreaImageGray, CV_RGB2GRAY);
	cvThreshold(maskAreaImageGray, maskAreaImageGray, 250, 255, CV_THRESH_BINARY);

	for (int i = 0; i < MAX_POLY_NUMBER; i++)
	{
		delete []polyPointTmp[i];
	}
	delete []polyPointTmp;

	cvReleaseImage(&maskAreaImageColor);
	return maskAreaImageGray;
}


/*********************************************************************
combineMaskAreaAndFG--将区域掩膜图像与前景图像相与并返回
*********************************************************************/
IplImage *ImageProcess::combineMaskAreaAndFG(IplImage *areaMaskImage, IplImage *FGImage)
{
	IplImage *combinedImage = NULL;

	combinedImage = cvCreateImage(cvSize(areaMaskImage->width, areaMaskImage->height), areaMaskImage->depth, areaMaskImage->nChannels);
	for (int i = 0; i < areaMaskImage->height; i++)
	{
		for (int j = 0; j < areaMaskImage->width; j++)
		{
			if (areaMaskImage->imageData[i*areaMaskImage->widthStep+j] != 0 && FGImage->imageData[i*FGImage->widthStep+j] != 0)
			{
				combinedImage->imageData[i*combinedImage->widthStep+j] = 255;
			}
			else
			{
				combinedImage->imageData[i*combinedImage->widthStep+j] = 0;
			}
		}
	}

	return combinedImage;
}

/*********************************************************************
checkObjectInMaskArea--检查运动对象是否大部分在区域掩膜中（可以再进一步约束）
*********************************************************************/
bool ImageProcess::checkObjectInMaskArea(Rect BBox, IplImage *combinedImage)
{
	cvSetImageROI(combinedImage, BBox);
	CvScalar s = cvSum(combinedImage);

	if (s.val[0] > BBox.height*BBox.width*255/3)
	{
		cvResetImageROI(combinedImage);
		return true;
	}
	else
	{
		cvResetImageROI(combinedImage);
		return false;
	}
}

/*********************************************************************
getBoundingBox--得到一幅前景图像中包含各个前景的方框
*********************************************************************/
void ImageProcess::getBoundingBox(IplImage *maskImage, vector<Rect>&BBox)
{
	IplImage *tempImage = NULL;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = NULL;

	tempImage = (IplImage *)cvClone(maskImage);
	cvFindContours(tempImage, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
	for (; contour != NULL; contour = contour->h_next)
	{
		Rect tempRect = cvBoundingRect(contour, 0);
		BBox.push_back(tempRect);
	}

	cvReleaseMemStorage(&storage);
	cvReleaseImage(&tempImage);
}

/*********************************************************************
saveImage--将当前的图像保存到指定路径中
*********************************************************************/
void ImageProcess::saveImage(IplImage *savedImage, char direction[128])
{
	//char keyCode;
	IplImage *tempImage = NULL;
	static int imageNumber = 0;
	char fileName[64];                                             //sprintf函数需要一个被定义的字符变量
	string tempDirection;

	if (direction)
	{
		tempDirection.append(direction);
	}

	tempImage = cvCloneImage(savedImage);
	imageNumber++;
	sprintf(fileName, "%d.jpg", imageNumber);
	if (!direction)
	{
		cvSaveImage(fileName, tempImage);
		cout << "Saving the frame done!" << endl;
	}
	else
	{
		tempDirection.append(fileName);
		cvSaveImage(tempDirection.data(), tempImage);
		cout << "Saving the frame done!" << endl;
	}

	cvReleaseImage(&tempImage);
}