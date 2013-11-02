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
#include <boost/regex.hpp>
#include <X11/Xutil.h>
#include <fstream>

#define TRAINING_PATH "training"
#define IMAGE_REGEX "eng.salt.exp([0-9]+).tif"


const char* debugWindow = "debug";

using namespace std;
using namespace cv;
using namespace boost;

WindowMonitor::WindowMonitor() :
    _debugWindow(),
    _prevRed(),
    _prevBlue(),
    _imgNum(0),
    _tess(),
    _server(){
        _tess.Init(NULL,"eng");
        _server.asyncRun();
//        _tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
//        _tess.SetVariable("tessedit_char_whitelist","ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
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
    bool waitingForWinner = false;
    boost::array<string,2> names;
    int winner;

    for(;;) {
        FILE *f = fopen("matchhistory.txt","a");

        if(image != NULL){
            cvReleaseImageHeader(&iplImage);
            XDestroyImage(image);
        }

        image = XGetImage(_display,*_window,attr.x,attr.y,attr.width,attr.height,AllPlanes,ZPixmap);
        iplImage = cvCreateImageHeader(cvSize(image->width,image->height),
                IPL_DEPTH_8U,
                image->bits_per_pixel/8);
        iplImage->widthStep = image->bytes_per_line;
        if(image->data != NULL){
            iplImage->imageData = image->data;
        }

        Mat cvMat(iplImage);
        if(!waitingForWinner) {
            names = readNames(cvMat);
            waitingForWinner = !names[0].empty() && !names[1].empty();
            if(waitingForWinner){
                _server.publishNames(names[0],names[1]);
            }
        } else {
            winner = findWinner(cvMat);
            if(winner != -1){
                cout << "Player " << winner + 1 << " wins!\n" << endl;
                waitingForWinner = false;
                fprintf(f,"%s\t%s\t%d\n",names[0].c_str(),names[1].c_str(),winner);
            }
        }
        fclose(f);

        waitKey(5000);

    }
}



Display* WindowMonitor::getDisplay() const {
    return _display;
}

void WindowMonitor::setDisplay(Display* display) {
    _display = display;
}

boost::array<std::string,2> WindowMonitor::readNames(Mat mat) {
    mat = cropToVideo(mat);
    Rect player1NameBounds(200,12,mat.cols-200,44);
    Rect player2NameBounds(0,603,mat.cols,44); //UNCOMMENT FOR FIREFOX
//    Rect player2NameBounds(0,578,mat.cols,44); //UNCOMMENT FOR IMAGE VIEWER
    Mat player1Mat = mat(player1NameBounds);
    Mat player2Mat = mat(player2NameBounds);
    NameReturn ret;
    boost::array<string,2> names;

    ret = readName(player1Mat,true);
    names[0] = ret.name;
    imshow("Player 1",ret.image);
    if(!ret.name.empty() && ret.name != _prevRed){
        _imgNum++;
        _prevRed = ret.name;
        cout << "Player 1: " << ret.name << endl;
//        saveImage(ret.image);
    }

    ret = readName(player2Mat,false);
    imshow("Player 2",ret.image);
    names[1] = ret.name;

    if(!ret.name.empty() && ret.name != _prevBlue){
        _imgNum++;
        _prevBlue = ret.name;
        cout << "Player 2: " << ret.name << endl;
//        saveImage(ret.image);
    }
    return names;
}

Mat WindowMonitor::cropToVideo(cv::Mat in) {
    Rect boundRect(350,100,1100,699);
    return in(boundRect);
}

NameReturn WindowMonitor::readName(cv::Mat in, bool isRed) {
    NameReturn ret;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
//    if(isRed){
//        imshow("red1", in);
//    } else {
//        imshow("red2", in);
//    }
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3,3), Point(1,1));


    if(isRed){
//        inRange(in,Scalar(25,55,165),Scalar(80,85,245),in);
//        inRange(in,Scalar(25,50,160),Scalar(80,90,245),in);
        inRange(in,Scalar(22,48,145),Scalar(83,93,245),in);

    } else {
//        inRange(in,Scalar(185,135,28),Scalar(255,170,100),in);
        inRange(in,Scalar(180,117,26),Scalar(255,173,103),in);
    }

    findContours(in.clone(),contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, Point(0,0));

    //shave off edge pixels
    rectangle(in,Point(0,0),Point(in.cols-1,in.rows-1),Scalar(0,0,0),1,8,0);

    for(unsigned int i = 0; i < contours.size(); i++){
        double size = contourArea(contours[i]);
        vector<vector<Point> > poly;
        Rect rect = boundingRect(Mat(contours[i]));

        //remove noise
        if(     rect.area() > 2000 || rect.area() < 80 ||
                rect.width > 40||
                rect.height > 35 || rect.height < 28 ||
                rect.y < 3 || rect.y + rect.height  > in.rows - 3 ||
                size < 25 || size > 1000) {
            //erase matches
            drawContours(in,contours,i,Scalar(0),-1);
        }

    }
//    Rect r(1,1,in.cols - 2, in.rows - 2);
//    in = in(r);

    _tess.SetImage((uchar*)in.data, in.cols,in.rows, 1, in.cols);
    string unformattedName = _tess.GetUTF8Text();
    stringstream strstream;
    for(unsigned int i = 0; i < unformattedName.size(); i++){
        if(unformattedName.at(i) != '\n'){
            strstream << unformattedName.at(i);
        }
    }
    ret.name = strstream.str();
    ret.image = in;
    return ret;
}

void WindowMonitor::saveImage(Mat image){
    char cwd[200];
    getcwd(cwd, 200);
    sprintf(cwd,"%s/training/",cwd);
    vector<string> files = Directory::GetListFiles(cwd);

    cmatch matches;
    regex regex(IMAGE_REGEX);
    long fileNum = -1;
    for(unsigned int i = 0; i < files.size(); i++){
        if(regex_match(files[i].c_str(),matches,regex)){
            string str(matches[1].first,matches[1].second);
            long currentNum = atol(str.c_str());
            if(currentNum > fileNum) fileNum = currentNum;
        }
    }

    stringstream fileName;
    fileName << cwd << "eng.salt.exp" << ++fileNum << ".tif";

    cout << "Saving image " << fileName.str() << "..." << endl;
    imwrite(fileName.str(), image);
}

int WindowMonitor::findWinner(cv::Mat in){
    Rect r(810,810,280,30);
    in = in(r);

    //check blue
    Mat blue, red;
    inRange(in,Scalar(240,150,40),Scalar(255,160,60),blue);
    inRange(in,Scalar(100,100,200),Scalar(150,150,255),red);

//    imshow("blueview",blue);
//    imshow("redview",red);
//    imshow("winnerview",in);

    if(countNonZero(red) > 0) return 0;
    if(countNonZero(blue) > 0) return 1;

    return -1;
}







