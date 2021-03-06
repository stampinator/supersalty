#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "WindowMonitor.h"

using namespace std;


std::string getWindowName(Display *disp, Window win) {
    Atom prop = XInternAtom(disp,"WM_NAME",False), type;
    int form;
    unsigned long remain, len;
    unsigned char *list;


    if (XGetWindowProperty(disp,win,prop,0,1024,False,AnyPropertyType,
                &type,&form,&len,&remain,&list) != Success) {

        return NULL;
    }

    std::string str;;

    if(list != NULL){
		str = (char*)list;
    } else {
    	str = "Unknown window";
    }

    return str;
}

Window *getWindowList(Display *disp, unsigned long *len) {
    Atom prop = XInternAtom(disp,"_NET_CLIENT_LIST",False), type;
    int form;
    unsigned long remain;
    unsigned char *list;

    if (XGetWindowProperty(disp,XDefaultRootWindow(disp),prop,0,1024,False,XA_WINDOW,
                &type,&form,len,&remain,&list) != Success) {
        return 0;
    }

    return (Window*)list;

}

Window getWindowChoiceFromUser(Display *disp){
    cout << "Choose a window:" << endl;
    Window *list;
    unsigned long len;

    list = getWindowList(disp,&len);
    for(unsigned int i = 0; i < len; ++i){
    	cout << i << ": " << getWindowName(disp,list[i]) << endl;
    }

    cout << "enter choice: ";
    int windowChoice;
//  windowChoice = 9;
    cin >> windowChoice;
    delete list;
    return list[windowChoice];
}

int main(int argc, char* argv[]){
    Display *disp = XOpenDisplay(NULL);
	Window w = getWindowChoiceFromUser(disp);
	WindowMonitor *monitor = new WindowMonitor();
	monitor->setWindow(&w);
	monitor->setDisplay(disp);
	monitor->begin();
	delete monitor;
}
