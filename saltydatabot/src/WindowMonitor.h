/*
 * WindowMonitor.h
 *
 *  Created on: Oct 14, 2013
 *      Author: stamptd
 */

#ifndef WINDOWMONITOR_H_
#define WINDOWMONITOR_H_
#include "X11/Xlib.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <tesseract/baseapi.h>

typedef struct {
	std::string name;
	cv::Mat image;
} NameReturn;


class WindowMonitor {
public:
	WindowMonitor();
	virtual ~WindowMonitor();
	Window* getWindow() const;
	void setWindow(Window* window);
	void begin();
	Display* getDisplay() const;
	void setDisplay(Display* display);

private:
	void readNames(cv::Mat mat);
	cv::Mat cropToVideo(cv::Mat in);
	NameReturn readName(cv::Mat in, bool isRed);
	void saveImage(cv::Mat mat);

	Window* _window;
	Display* _display;
	Window _debugWindow;
	std::string _prevRed;
	std::string _prevBlue;
	int _imgNum;
	tesseract::TessBaseAPI _tess;
};

#endif /* WINDOWMONITOR_H_ */
