/*
 * WindowMonitor.cpp
 *
 *  Created on: Oct 14, 2013
 *      Author: stamptd
 */

#include "WindowMonitor.h"
#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <unistd.h>
#include <stdlib.h>
#include <tesseract/baseapi.h>

#define TRAINING_PATH "training"
#define IMAGE_REGEX "eng.salt.exp([0-9]+).tif"


const char* debugWindow = "debug";

using namespace std;
using namespace cv;

WindowMonitor::WindowMonitor() :
	_debugWindow(),
	_prevRed(),
	_prevBlue(),
	_imgNum(0),
	_tess(){
		_tess.Init(NULL,"eng",tesseract::OEM_DEFAULT);
		_tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
}

WindowMonitor::~WindowMonitor() {
	// TODO Auto-generated destructor stub
}

Window* WindowMonitor::getWindow() const {
	return _window;
}

void WindowMonitor::setWindow(Window* window) {
	_window = window;
}


void WindowMonitor::begin() {

	XWindowAttributes attr;
	XGetWindowAttributes(_display,*_window,&attr);
	XImage *image = NULL;
	IplImage *iplImage = NULL;

	for(;;) {
		if(image != NULL){
			delete image;
			delete iplImage;
		}

		XImage *image = XGetImage(_display,*_window,attr.x,attr.y,attr.width,attr.height,AllPlanes,ZPixmap);
		IplImage *iplImage = cvCreateImageHeader(cvSize(image->width,image->height),
				IPL_DEPTH_8U,
				image->bits_per_pixel/8);
		iplImage->widthStep = image->bytes_per_line;
		if(image->data != NULL){
			iplImage->imageData = image->data;
		}

		Mat cvMat(iplImage);
		readNames(cvMat);

		waitKey(10000);
	}
}



Display* WindowMonitor::getDisplay() const {
	return _display;
}

void WindowMonitor::setDisplay(Display* display) {
	_display = display;
}

void WindowMonitor::readNames(Mat mat) {
	mat = cropToVideo(mat);
	Rect player1NameBounds(200,13,mat.cols-200,44);
	Rect player2NameBounds(0,604,mat.cols,44);
	Mat player1Mat = mat(player1NameBounds);
	Mat player2Mat = mat(player2NameBounds);
	NameReturn ret;

	ret = readName(player1Mat,true);
	imshow("Player 1",ret.image);
	if(!ret.name.empty() && ret.name != _prevRed){
		_imgNum++;
		_prevRed = ret.name;
		cout << "Player 1: " << ret.name;
		saveImage(ret.image);
	}

	ret = readName(player2Mat,false);
	imshow("Player 2",ret.image);

	if(!ret.name.empty() && ret.name != _prevBlue){
		_imgNum++;
		_prevBlue = ret.name;
		cout << "Player 2: " << ret.name;
		saveImage(ret.image);
	}
}

Mat WindowMonitor::cropToVideo(cv::Mat in) {
	Rect boundRect(350,100,1100,699);
	return in(boundRect);
}

NameReturn WindowMonitor::readName(cv::Mat in, bool isRed) {
	NameReturn ret;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3,3), Point(1,1));


	if(isRed){
		inRange(in,Scalar(25,55,165),Scalar(80,85,245),in);

	} else {
		inRange(in,Scalar(185,135,28),Scalar(255,170,100),in);
	}

	findContours(in.clone(),contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0,0));

	for(unsigned int i = 0; i < contours.size(); i++){
		double size = contourArea(contours[i]);
		vector<vector<Point> > poly;
		Rect rect = boundingRect(Mat(contours[i]));

		//remove noise
		if(     rect.area() > 2000 || rect.area() < 80 ||
				rect.width > 40||
				rect.height > 30 || rect.height < 20 ||
				size < 25 || size > 1000) {
			//erase matches
			drawContours(in,contours,i,Scalar(0),-1);
		}
	}

	_tess.SetImage((uchar*)in.data, in.cols,in.rows, 1, in.cols);
	ret.name = _tess.GetUTF8Text();
	ret.image = in;
	return ret;
}

void WindowMonitor::saveImage(Mat image){
	char cwd[200];
	getcwd(cwd, 200);
	sprintf(cwd,"%s/training/",cwd);
	vector<string> files = Directory::GetListFiles(cwd);
	for(unsigned int i = 0; i < files.size(); i++){
//		if(regex_match(files[i],regex(IMAGE_REGEX,regex_constants::basic))){
//			cout << files[i];
//		}
	}
	cout << endl;
}







