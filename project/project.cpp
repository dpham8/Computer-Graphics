//Dat Pham
//
//program: lab10 copied from lab9
//
//program: lab10.cpp
//author:  Gordon Griesel
//date:    Fall 2021
//purpose: Framework for X11 (Xlib, XWindows)
//
//This program implements a wrapper class to handle X11 operations.
//The main function contain an infinite loop.
//Keyboard input is handled.
//Mouse input is handled.
//All drawing is done in the render() function.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <X11/Xutil.h>
//#include <c/time>

using namespace std;

#define PI 3.141592653589793238462643             
#define rnd() ((Flt)rand() / (Flt)RAND_MAX)

typedef double Flt;

class Image {
public:    
    int width, height;
    unsigned char *data;
    Image(const char *fname) {
        ifstream fin(fname);
        if (fin.fail()) {
            cout << "ERROR - opening image" << fname << endl;
            data = new unsigned char[3];
            width = 1;
            height = 1;
            return;
            //exit(0);
        }
        char p6[8];
        fin >> p6;
        fin >> width >> height;
        int maxcolor;
        fin >> maxcolor;
        data = new unsigned char [width * height * 3];
        fin.read((char *)data, 1);
        fin.read((char *)data, width*height*3);
        fin.close();
        cout << width << " " << height << endl;    
    }
} wood("/home/stu/dpham/3480/6/wood.ppm"), 
earth("/home/stu/dpham/3480/9/earth.ppm"), 
stone("/home/stu/dpham/3480/9/stone.ppm"), 
lava("/home/stu/dpham/3480/b/ray_tracing/lava.ppm");

class Vec {
public:
	Flt x,y,z;
	Vec() { x = y = z = 0.0; }
	Vec(Flt a, Flt b, Flt c) {
		x = a;
		y = b;
		z = c;
	}
	Vec(const Vec &v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}
	void copy(const Vec &v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}
	void copy(const unsigned char v[3]) {
		x = v[0] / 255.0;
		y = v[1] / 255.0;
		z = v[2] / 255.0;
	}
	void make(Flt a, Flt b, Flt c) {
		x = a;
		y = b;
		z = c;
	} 

    void negate() {
		x = -x;
		y = -y;
		z = -z;
	} 
    
    /*
    void make(int a, int b, int c) {
		x = a;
		y = b;
		z = c;
	}
    */
	void diff(Vec v0, Vec v1) {
		x = v0.x - v1.x;
		y = v0.y - v1.y;
		z = v0.z - v1.z;
	}
	void sub(Vec v) {
		x = x - v.x;
		y = y - v.y;
		z = z - v.z;
	}
	void add(Flt a) {
        x = x + a;
        y = y + a;
        z = z + a;
    }
    //
    void add(const Vec &v) {
		x = x + v.x;
		y = y + v.y;
		z = z + v.z;
	}
	void add(const Vec &v0, const Vec &v1) {
		x = v0.x + v1.x;
		y = v0.y + v1.y;
		z = v0.z + v1.z;
	}
	void mult(Vec v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
	}
	void scale(Flt a) {
		x *= a;
		y *= a;
		z *= a;
	}
	void normalize() {
		Flt len = sqrt(x*x + y*y + z*z);
		if (len == 0.0)
			return;
		x /= len;
		y /= len;
		z /= len;
	}
	Flt dotProduct(Vec v) {
		return x*v.x + y*v.y + z*v.z;
	}
	Flt len() { return sqrt(x*x + y*y + z*z); }
	void combine(Flt A, Vec a, Flt B, Vec b) {
		x = A * a.x + B * b.x;
		y = A * a.y + B * b.y;
		z = A * a.z + B * b.z;
	}
	void crossProduct(const Vec &v0, const Vec &v1) {
		x = v0.y*v1.z - v1.y*v0.z;
		y = v0.z*v1.x - v1.z*v0.x;
		z = v0.x*v1.y - v1.x*v0.y;
	}
	void clamp(Flt low, Flt hi) {
		if (x < low) x = low;
		if (y < low) y = low;
		if (y < low) z = low;
		if (x > hi) x = hi;
		if (y > hi) y = hi;
		if (z > hi) z = hi;
	}
};


Flt degreesToRadians(Flt angle) {
	return (angle / 360.0) * (3.141592653589793 * 2.0);
}

enum {
	OBJ_TYPE_DISC,
	OBJ_TYPE_RING,
	OBJ_TYPE_SPHERE,
    OBJ_TYPE_TRI
};

class Object {
public:
	int type;
	Vec pos;
    Vec vel;
	Flt radius, radius2;
	unsigned char color[3];
	Vec normal;
	int checker;
    Vec checker_color[2];
	//triangle qualitites
    Vec tri[3];
    int texture_mapped;
    Flt textCoord[3][2];
    Flt u,v,w;
    Flt mat[3][3];
    Flt angle[3];
    // 
    Flt checker_size;
    int specular;
    int perlin;
    int marble;
    int stexture;
    int hspecular;
    int tspecular;
    int shadow;
    //flags
    int perlin2, perlin3;
    int marble2, marble3;
    
    //clipping
    int nclips;
    Vec cpoint;
    Vec cnormal;
    float cradius;
    int inside;
    
    //triangle multiple clippings
    struct Clip {
        Vec cpoint;
        Vec cnormal;
        float cradius;
        int inside;
    } clip[5];
    
    Object() {
		checker = 0;
        specular = hspecular = tspecular = 0;
        perlin = 0;
        marble = 0;
        stexture = 0;
        nclips = 0;
        shadow = 0;
    }
} obj[ 1000 ];
int nobjects = 0;

class Ray {
public:
	Vec o;
	Vec d;
};

struct Hit {
	//t     = distance to hit point
	//p     = hit point
	//norm  = normal of surface hit
	//color = color of surface hit
	Flt t;
	Vec p, norm, color;
};

class Camera {
public:
	Vec pos, at, up;
	Flt ang;
	Camera() {
		pos.make(0.0, 0.0, 1000.0);
		at.make(0.0, 0.0, -1000.0);
		up.make(0.0, 1.0, 0.0);
		ang = 45.0;
	}
} cam;

class Global {
public:
	//define window resolution
	int xres, yres;
	int type;
	int menu;
	int perspective;
    int hit_idx;
    //
	Vec light_pos;
	Vec ambient;
	Vec diffuse;
    //
    int capture;
	int max_frames;
    int frames; 
    //
    //domino
    Flt dom_angle;
    float dom_y; 
    /*
    //test
    float mat[4][4] = {{1,0,0,0 },
                        {0,cos(angle),-sin(angle),0},
                        {0,sin(angle),cos(angle),0 },
                        {0,0,0,1}};

    float mat[3][3] = {{1,0,0},
                        {0,1,0},
                        (0,0,1}};
    */
    Global() {
		xres = 640;
		yres = 480;
		type = 0;
		menu = 0;
        capture = max_frames = frames = 0;
		perspective = 0;
		light_pos.make(100, 100, 100);
		ambient.make(1,1,1);
		diffuse.make(0,0,0);
        hit_idx = -1;
        dom_angle = 0.0;
        dom_y = 0.0;
	}
} g;

class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GC gc;
public:
	X11_wrapper();
	~X11_wrapper();
	bool getPending();
	void getNextEvent(XEvent *e);
	void check_resize(XEvent *e);
	void getWindowAttributes(int *width, int *height);
    XImage *getImage(int width, int height); 
    void set_backcolor_3i(int r, int g, int b);
	void set_color_3i(int r, int g, int b);
	void clear_screen();
	void drawString(int x, int y, const char *message);
	void drawPoint(int x, int y);
	void drawLine(int x0, int y0, int x1, int y1);
	void drawRectangle(int x, int y, int w, int h);
	void fillRectangle(int x, int y, int w, int h);
	void check_mouse(XEvent *e);
	int check_keys(XEvent *e);
} x11;

//function prototypes
void physics();
void render();
void rectangle();
void getTriangleNormal(Vec tri[3], Vec &norm);
void rotate();
//
void lab_animation();
void domino_gif();
void takeScreenshot(const char*,int);
          
//===============================================
int main()
{
	int done = 0;
	while (!done) {
		//Check the event queue
		while (x11.getPending()) {
			//Handle the events one-by-one
			XEvent e;
			x11.getNextEvent(&e);
			x11.check_resize(&e);
			done = x11.check_keys(&e);
	   		//datt
            //render();
	        //making gif
            
            //domino falling part, have to do it manually 
            //g.dom_angle = -0.2;
            //g.dom_y = -5.0; 
           
            //g.dom_angle = -0.4;
            //g.dom_y = -20.0;
           
            //g.dom_angle = -0.6;
            //g.dom_y = -40.0;

            //g.dom_angle = -0.8;
            //g.dom_y = -55.0;

            //g.dom_angle = -1.0;
            //g.dom_y = -90.0;

            //g.dom_angle = -1.2;
            //g.dom_y = -145.0;

            //g.dom_angle = -1.3;
            //g.dom_y = -300.0; 

            //g.dom_angle = -1.4;
            //g.dom_y = -400.0;
 
            //void takeScreenshot(const char*,int);
            //takeScreenshot("lab623.ppm",0);
            
            if (g.capture == 1) {
                //lab_animation();                
                domino_gif();
 
                /*
               while (g.frames <= g.max_frames) {
        //void takeScreenshot(const char*,int);
                render();
   
                takeScreenshot("",0);
                //g.dom_angle += 0.5;
               }
                //render();
                //takeScreenshot("",0);
   
   //             g.dom_y = -50.0; 
           
                //render();*/

                   /*
                while (g.frames <= g.max_frames) {
                    void takeScreenshot(const char*,int);
                    render();
                    takeScreenshot("",0);
                    g.dom_angle = 0.2;
                    g.frames++;
                } */
                g.capture = 0;
                system("convert -loop 0 -coalesce -layers OptimizeFrame -delay 40 lab6*ppm animation.gif"); 
            }    
        }
    }
		//render();
		//usleep(1000);
	
	return 0;
}
//===============================================

void domino_gif() {
    //starting cam
    //cam.pos.make( 300.0, 400.0, -100.0); //-200
    //cam.at.make( -500.0, -100.0, 0.0);
	//
    cam.pos.make( 300.0, 400.0, 300.0);
    cam.at.make( -400.0, -100.0, 0.0);
	
    //float pos[3] = {300.0, 400.0, -100.0};
    //float at[3] = {-500.0, -100.0, 0.0};
   
    float pos[3] = {300.0, 400.0, 300.0};
    float at[3] = {-400.0, -100.0, 0.0};
    
    while (g.frames <= g.max_frames) {
//        void takeScreenshot(const char*,int);
        render();
        
        //if (g.frames < 30) {
        //if (g.frames <= 22) {
        /*if (pos[2] < 250.0) {
            takeScreenshot("",0);
            pos[2] += 50.0;
            cam.pos.make( pos[0], pos[1], pos[2] );
            cam.at.make( at[0], at[1], at[2] );
        } else*/ if (pos[2] < 600.0) {
            takeScreenshot("",0);
            pos[0] -= 50.0;
            pos[2] += 50.0;
            at[0] += 50.0;  
            if (at[1] < 100.0) {
                at[1] += 25.0; 
            }
            cam.pos.make( pos[0], pos[1], pos[2] );
            cam.at.make( at[0], at[1], at[2] ); 
        } else if (pos[0] > 700.0) {
            takeScreenshot("",0);
            pos[0] -= 50.0;
            at[0] += 50.0; 
            cam.pos.make( pos[0], pos[1], pos[2] );
            cam.at.make( at[0], at[1], at[2] ); 
        } 
         
        /*
        if (g.frames >= 30) {
            if (pos[2] > 400.0) {
                takeScreenshot("",0);
                pos[2] -= 50.0;
                cam.pos.make( pos[0], pos[1], pos[2] );
                cam.at.make( at[0], at[1], at[2] );
            } else {
               g.frames = 40;
            } 
        } */ 
        
        //if (g.frames > 22) {
         //   takeScreenshot("",0);
          //  g.dom_angle = -0.2; 
         //}
    
        g.frames++;
    }
}

void lab_animation () {
    while (g.frames <= g.max_frames) {
        void takeScreenshot(const char*,int);
        render();
        if (g.frames <= 10) {
            takeScreenshot("",0);
            obj[1].pos.z-= 80.0;                       
            obj[1].radius -= 5.0;
            obj[2].pos.z+= 80.0;                       
            obj[2].radius += 5.0; 
            obj[3].pos.z+= 80.0;                       
            obj[3].radius += 5.0; 
        } else {
            takeScreenshot("",0);
            obj[1].pos.z += 80.0;                       
            obj[1].radius += 5.0; 
            obj[2].pos.z-= 80.0;                       
            obj[2].radius -= 5.0; 
            obj[3].pos.z-= 80.0;                       
            obj[3].radius -= 5.0; 
        } 
        g.frames++;
    }
}

void takeScreenshot(const char *filename, int reset)
{
    //This function will capture your current X11 window,
    //and save it to a PPM P6 image file.
    //File names are generated sequentially.
    static int picnum = 0;
    int x,y;
    int width, height;
    x11.getWindowAttributes(&width, &height);
    if (reset)
        picnum = 0;
    XImage *image = x11.getImage(width, height);
    //
    //If filename argument is empty, generate a sequential filename...
    char ts[256] = "";
    strcpy(ts, filename);
    if (ts[0] == '\0') {
        sprintf(ts,"./lab6%02i.ppm", picnum);
        picnum++;
    }
    FILE *fpo = fopen(ts, "w");
    if (fpo) {
        fprintf(fpo, "P6\n%i %i\n255\n", width, height);
        for (y=0; y<height; y++) {
            for (x=0; x<width; x++) {
                unsigned long pixel = XGetPixel(image, x, y);
                fputc(((pixel & 0x00ff0000)>>16), fpo);
                fputc(((pixel & 0x0000ff00)>> 8), fpo);
                fputc(((pixel & 0x000000ff)    ), fpo);
            }
        }
        fclose(fpo);
    }
    XFree(image);
}


void X11_wrapper::getWindowAttributes(int *width, int *height) {
	XWindowAttributes gwa;
	XGetWindowAttributes(dpy, win, &gwa);
	*width = gwa.width;
	*height = gwa.height;
}

XImage *X11_wrapper::getImage(int width, int height) {
	return XGetImage(dpy, win, 0, 0, width, height, AllPlanes, ZPixmap);
}

X11_wrapper::X11_wrapper() {
	//constructor
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}
	int scr = DefaultScreen(dpy);
	win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr),
							1, 1, g.xres, g.yres, 0, 0x00ffffff, 0x00400040);
	XStoreName(dpy, win, "CMPS-3480   Press Esc to exit.");
	gc = XCreateGC(dpy, win, 0, NULL);
	XMapWindow(dpy, win);
	XSelectInput(dpy, win, ExposureMask | StructureNotifyMask |
							PointerMotionMask | ButtonPressMask |
							ButtonReleaseMask | KeyPressMask);
}
/*
void X11_wrapper::getWindowAttributes(int *width, int *height) {
	XWindowAttributes gwa;
	XGetWindowAttributes(dpy, win, &gwa);
	*width = gwa.width;
	*height = gwa.height;
}

XImage *X11_wrapper::getImage(int width, int height) {
	return XGetImage(dpy, win, 0, 0, width, height, AllPlanes, ZPixmap);
}
*/
X11_wrapper::~X11_wrapper() {
	//destructor
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}
bool X11_wrapper::getPending() {
	return XPending(dpy);
}
void X11_wrapper::getNextEvent(XEvent *e) {
	XNextEvent(dpy, e);
}
void X11_wrapper::check_resize(XEvent *e) {
	//ConfigureNotify is sent when window size changes.
	if (e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	g.xres = xce.width;
	g.yres = xce.height;
}
void X11_wrapper::set_backcolor_3i(int r, int g, int b) {
	unsigned long cref = (r<<16) + (g<<8) + b;
	XSetBackground(dpy, gc, cref);
}
void X11_wrapper::set_color_3i(int r, int g, int b) {
	unsigned long cref = (r<<16) + (g<<8) + b;
	XSetForeground(dpy, gc, cref);
}
void X11_wrapper::clear_screen() {
	XClearWindow(dpy, win);
}
void X11_wrapper::drawString(int x, int y, const char *message) {
	XDrawString(dpy, win, gc, x, y, message, strlen(message));
}
void X11_wrapper::drawPoint(int x, int y) {
	XDrawPoint(dpy, win, gc, x, y);
}
void X11_wrapper::drawLine(int x0, int y0, int x1, int y1) {
	XDrawLine(dpy, win, gc, x0, y0, x1, y1);
}
void X11_wrapper::drawRectangle(int x, int y, int w, int h) {
	//x,y is upper-left corner
	XDrawRectangle(dpy, win, gc, x, y, w, h);
}
void X11_wrapper::fillRectangle(int x, int y, int w, int h) {
	//x,y is upper-left corner
	XFillRectangle(dpy, win, gc, x, y, w, h);
}

int X11_wrapper::check_keys(XEvent *e) {
	if (e->type != KeyPress && e->type != KeyRelease) {
		//not a keyboard event. return.
		return 0;
	}
	int key = XLookupKeysym(&e->xkey, 0);
	//Process only key presses.
	if (e->type != KeyPress)
		return 0;
    
    switch (key) {
        case XK_b:
            break;
        case XK_l:
            //datt
            takeScreenshot("lab632.ppm",0); //31 -wrong 
            break; 
        case XK_t:
            //lab_animation
            g.capture = 1;
            g.max_frames = 20;
            /*render();
            
            takeScreenshot("",0);
            //move the sphere
            obj[1].pos.x+=10.0;
            render();
            takeScreenshot("",0);
            //make a gif animation */
            break;
        case XK_y:
            //domino animation
            g.capture = 1;
            g.max_frames = 22;
            break;
        case XK_p:
			g.perspective ^= 1;
			break;
		case XK_r:
			render();
			break;
		case XK_m:
			g.menu = 1;
			render();
			break;
        case XK_1:
			//a disc
			obj[0].type = OBJ_TYPE_DISC;
			obj[0].pos.make(0, 0, -400.0);
			obj[0].normal.make(0.0, 0.0, 1.0);
			obj[0].normal.normalize();
			obj[0].radius = 200.0;
			obj[0].color[0] = 255;
			obj[0].color[1] = 100;
			obj[0].color[2] = 0;
			obj[0].checker = 0;
			nobjects = 1;
			//
			g.light_pos.make(-10, 10, 200);
			g.ambient.make(1, 1, 1);
			g.diffuse.make(0,0,0);
			//
			cam.pos.make(0.0, 0.0, 1000.0);
			cam.at.make(0.0, 0.0, -1000.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 45.0;
			break;
		case XK_2:
			//three discs
			nobjects = 0;
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(-100, 100, 200.0);
			obj[nobjects].normal.make(0.0, 0.0, 1.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 100.0;
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
			obj[nobjects].checker = 0;
			++nobjects;
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(0, 0, 0.0);
			obj[nobjects].normal.make(0.0, 0.0, 1.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 100.0;
			obj[nobjects].color[0] = 230;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 100;
			obj[nobjects].checker = 0;
			++nobjects;
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(100, -100, -200.0);
			obj[nobjects].normal.make(0.0, 0.0, 1.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 100.0;
			obj[nobjects].color[0] = 230;
			obj[nobjects].color[1] = 100;
			obj[nobjects].color[2] = 230;
			obj[nobjects].checker = 0;
			++nobjects;
			g.light_pos.make(-10, 10, 200);
			g.ambient.make(1, 1, 1);
			g.diffuse.make(0,0,0);
			//
			cam.pos.make(0.0, 0.0, 1000.0);
			cam.at.make(0.0, 0.0,  0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 45.0;
			break;
		case XK_4:
			//two discs on floor
			nobjects = 0;
			//floor
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(0, 0, -300.0);
			obj[nobjects].normal.make(0.0, 1.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 1400.0;
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
			//obj[nobjects].checker = 1;
			++nobjects;
			//left disc, green, 2 sides
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(-160, 0.0, 0.0);
			obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 200.0;
			obj[nobjects].color[0] = 20;
			obj[nobjects].color[1] = 250;
			obj[nobjects].color[2] = 20;	
            ++nobjects;
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(-160+.1, 0, 0.0);
			obj[nobjects].normal.make(1.0, 0.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 200.0;
			obj[nobjects].color[0] = 20;
			obj[nobjects].color[1] = 250;
			obj[nobjects].color[2] = 20;
			++nobjects;
			//right disc, red
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(160, 0, 0.0);
			obj[nobjects].normal.make(1.0, 0.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 200.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 20;
			obj[nobjects].color[2] = 20;
			++nobjects;
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(160-.1, 0, 0.0);
			obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 200.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 20;
			obj[nobjects].color[2] = 20;
            ++nobjects;
			//sphere
			obj[nobjects].type = OBJ_TYPE_SPHERE;
			obj[nobjects].pos.make(0.0, 100.0, 0.0);
			//obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
			obj[nobjects].radius = 100.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
			++nobjects;
			g.light_pos.make(-300, 240, 150);
			g.ambient.make(.5, .5, .5);
			g.diffuse.make(.6, .6, .6);
			//
			cam.pos.make(-20.0, 325.0, 550.0);
			cam.at.make(0.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 90.0;
			break; 
        case XK_5:
			//animation
            //1 sphere on floor
            nobjects = 0;
			//floor
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(0, 0, 0.0);
			obj[nobjects].normal.make(0.0, 1.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 2000.0;
			//obj[nobjects].color[0] = 255;
			//obj[nobjects].color[1] = 150;
			//obj[nobjects].color[2] = 20;
			obj[nobjects].checker = 1;
            obj[nobjects].checker_color[0].make(0,0,0);
			obj[nobjects].checker_color[1].make(1,1,1);
            obj[nobjects].checker_size = 90.0;
            ++nobjects;     
            //sphere
			obj[nobjects].type = OBJ_TYPE_SPHERE;
			obj[nobjects].pos.make(0.0, 60.0, 0.0);
			//obj[nobjects].pos.make(0.0, 60.0, 200.0);	
            //obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
			obj[nobjects].radius = 60.0;
			obj[nobjects].color[0] = 50;
			obj[nobjects].color[1] = 50;
			obj[nobjects].color[2] = 200;
		    obj[nobjects].specular = 1;	
            obj[nobjects].stexture = 1;	 
            ++nobjects;
            //sphere
            obj[nobjects].type = OBJ_TYPE_SPHERE;
			obj[nobjects].pos.make(120.0, 60.0, -750.0);
			//obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
			obj[nobjects].radius = 10.0;
			obj[nobjects].color[0] = 50;
			obj[nobjects].color[1] = 50;
			obj[nobjects].color[2] = 200;
		    obj[nobjects].specular = 1;
	        obj[nobjects].stexture = 1;	 
            ++nobjects;           
            //sphere
            obj[nobjects].type = OBJ_TYPE_SPHERE;
			obj[nobjects].pos.make(-120.0, 60.0, -750.0);
			//obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
			obj[nobjects].radius = 10.0;
			obj[nobjects].color[0] = 50;
			obj[nobjects].color[1] = 50;
			obj[nobjects].color[2] = 200;
		    obj[nobjects].specular = 1;
	        obj[nobjects].stexture = 1;	 
            ++nobjects;
            //triangles
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(-90.0,   0.0, 180.0);
            obj[nobjects].tri[1].make(  0.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-45.0, 120.0, 180.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 50;
            obj[nobjects].color[1] = 250;
            obj[nobjects].color[2] = 100;
            ++nobjects;
             
			g.light_pos.make(0, 1000, 0);
			g.ambient.make(.5, .5, .5);
			g.diffuse.make(.6, .6, .6);
			//
			cam.pos.make(-30.0, 150.0, 600.0);
			cam.at.make(0.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 40.0;
			break;
        case XK_6: 
            //1 disc for Perlin Noise testing 
            nobjects = 0;
			//wall
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(0, 0, 0.0);
			obj[nobjects].normal.make(0.0, 0.0, 1.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 200.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 0;
			obj[nobjects].color[2] = 0;
            obj[nobjects].perlin = 1;
		    ++nobjects;     
            
            g.light_pos.make(0, 1000, 0);
			g.ambient.make(1,1,1);
			g.diffuse.make(0,0,0);
			//
			cam.pos.make(0.0, 0.0, 900.0);
			cam.at.make(0.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 40.0;
	        break;
        case XK_7:
            //1 disc for Perlin Noise testing 
            nobjects = 0;
			//sphere
			obj[nobjects].type = OBJ_TYPE_SPHERE;
			obj[nobjects].pos.make(0, 0, 0.0);
			obj[nobjects].normal.make(0.0, 0.0, 1.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 200.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 0;
			obj[nobjects].color[2] = 0;
            //
            obj[nobjects].perlin2 = 0;
            obj[nobjects].marble2 = 0;
            obj[nobjects].perlin3 = 1;
            obj[nobjects].perlin3 = 0;
            //
            obj[nobjects].perlin = 0;
            obj[nobjects].marble = 1;
		    obj[nobjects].stexture = 0;
            obj[nobjects].checker = 0;
            ++nobjects;      
            g.light_pos.make(500, 500, 0);
			g.ambient.make(.5,.5,.5);
			g.diffuse.make(.5,.5,.5);
			//
			cam.pos.make(200.0, 200.0, 900.0);
		    //cam.pos.make(20.0, 80.0, 1300.0);
            cam.at.make(0.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 40.0;
	        break; 	    
        case XK_8:
	        //vase
            //floor
	        nobjects = 0;
	        obj[nobjects].type = OBJ_TYPE_DISC;
	        obj[nobjects].pos.make(0.0, 0.0, 0.0);
	        obj[nobjects].normal.make(0.0, 1.0, 0.0);
	        obj[nobjects].normal.normalize();
	        obj[nobjects].radius = 2000.0;
	        obj[nobjects].checker = 0;
	        obj[nobjects].color[0] = 250;
	        obj[nobjects].color[1] = 250;
	        obj[nobjects].color[2] = 250;
	        obj[nobjects].stexture = 0;
	        obj[nobjects].specular = 1;
	        obj[nobjects].checker = 1;
	        obj[nobjects].checker_size = 127.0;
	        //obj[nobjects].checker_tex[0] = -1;
	        //obj[nobjects].checker_tex[1] = -1;
	        obj[nobjects].checker_color[0].make(.2, .2, .7);
	        obj[nobjects].checker_color[1].make(.2, .2, .4);
	        ++nobjects;
	        //
	        //stars
	        for (int i=0; i<100; i++) {
		        obj[nobjects].type = OBJ_TYPE_DISC;
		        obj[nobjects].pos.make(
				                rnd()*2000.0-1000.0, rnd()*1500.0, -2000);
		        obj[nobjects].normal.make(0.0, 0.0, 1.0);
		        obj[nobjects].normal.normalize();
		        obj[nobjects].radius = rnd() * 3.0 + 0.5;
		        obj[nobjects].checker = 0;
		        obj[nobjects].color[0] = rnd() * 50.0 + 200.0;
		        obj[nobjects].color[1] = rnd() * 50.0 + 200.0;
		        obj[nobjects].color[2] = rnd() * 50.0 + 200.0;
		        obj[nobjects].stexture = 0;
		        obj[nobjects].specular = 0;
		        obj[nobjects].checker = 0;
		        ++nobjects;
	        }
	        //
	        //build perlin vase
	        {
	        //points on a circle
	        int n = 27;
	        Flt angle = 0.0;
	        Flt inc = (PI * 2.0) / (Flt)n;
	        Flt radii[13]  = { 3,3,1,1,3,4.5,5,5, 2,  1.5, 2, 3, 3 };
	        Flt height[13] = { 0,1,2,3,5,7,  9,11,12,13.5,15,16,17 };
	        Vec pts[40];
	        for (int i=0; i<n; i++) {
		        pts[i].x = cos(angle);
		        pts[i].y = 0.0;
		        pts[i].z = sin(angle);
		        angle += inc;
	        }
	        //13 levels
    	    //13 levels
	        Flt vsize = 20.0;
	        for (int i=0; i<12; i++) {
		        for (int j=0; j<n; j++) {
			        int k = (j+1) % n;
			        obj[nobjects].type = OBJ_TYPE_TRI;
			        //across 2 then up
			        obj[nobjects].tri[0].copy(pts[j]);
			        obj[nobjects].tri[2].copy(pts[k]);
			        obj[nobjects].tri[1].copy(pts[k]);
			        //
			        obj[nobjects].tri[0].scale(radii[i]*vsize);
			        obj[nobjects].tri[2].scale(radii[i]*vsize);
			        obj[nobjects].tri[1].scale(radii[i+1]*vsize);
			        obj[nobjects].tri[0].y = height[i]*vsize;
			        obj[nobjects].tri[2].y = height[i]*vsize;
			        obj[nobjects].tri[1].y = height[i+1]*vsize;
			        //
			        getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
			        obj[nobjects].color[0] = 30;
			        obj[nobjects].color[1] = 110;
			        obj[nobjects].color[2] = 40;
			        obj[nobjects].specular = 1;
			        obj[nobjects].hspecular = 0;
			        obj[nobjects].stexture = 0;
			        ++nobjects;
			        //up diagonal then back
			        obj[nobjects].type = OBJ_TYPE_TRI;
			        obj[nobjects].tri[0].copy(pts[j]);
			        obj[nobjects].tri[2].copy(pts[k]);
			        obj[nobjects].tri[1].copy(pts[j]);
			        //
			        obj[nobjects].tri[0].scale(radii[i]*vsize);
			        obj[nobjects].tri[2].scale(radii[i+1]*vsize);
			        obj[nobjects].tri[1].scale(radii[i+1]*vsize);
			        obj[nobjects].tri[0].y = height[i]*vsize;
			        obj[nobjects].tri[2].y = height[i+1]*vsize;
			        obj[nobjects].tri[1].y = height[i+1]*vsize;
			        //
			        getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
			        obj[nobjects].color[0] = 30;
			        obj[nobjects].color[1] = 110;
			        obj[nobjects].color[2] = 40;
			        obj[nobjects].specular = 1;
			        obj[nobjects].hspecular = 0;
			        obj[nobjects].stexture = 0;
			        ++nobjects;
		        }
	        }
	        }
	        //
	        g.light_pos.make(-200, 500, 600);
	        g.ambient.make(.4, .4, .4);
            g.diffuse.make(.7, .7, .7);
	        //g.background_black = 1;
	        //
	        cam.pos.make(-30.0, 100.0, 900.0);
	        cam.at.make(0.0, 180.0, 0.0);
	        cam.up.make(0.0, 1.0, 0.0);
	        cam.ang = 40.0; 
	        break; 
        case XK_9:
            nobjects = 0;
			//floor
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(0, 0, 0.0);
			obj[nobjects].normal.make(0.0, 1.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 2000.0;
			//obj[nobjects].color[0] = 255;
			//obj[nobjects].color[1] = 150;
			//obj[nobjects].color[2] = 20;
			obj[nobjects].checker = 1;
            obj[nobjects].checker_color[0].make(0,0,0);
			obj[nobjects].checker_color[1].make(1,1,1);
            obj[nobjects].checker_size = 90.0;
            ++nobjects; 
            //GREEN LEFT FACE
            /*
            //bottom right triangle
            //vertices order: left -> right -> top
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(-50.0,   0.0, 150.0);
            obj[nobjects].tri[1].make(  0.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 150.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 50;
            obj[nobjects].color[1] = 250;
            obj[nobjects].color[2] = 100;
            ++nobjects;
            //top right triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(0.0,   100.0, 150.0);
            obj[nobjects].tri[1].make(  0.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 150.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 50;
            obj[nobjects].color[1] = 250;
            obj[nobjects].color[2] = 100;
            ++nobjects;
            */
            
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make(-50.0,   0.0, 150.0);
            obj[nobjects].tri[0].make(  0.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 150.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            //obj[nobjects].ttexture = 1; 
            ++nobjects;
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(  0.0, 100.0, 200.0);
            obj[nobjects].tri[1].make(  0.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 150.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            //obj[nobjects].ttexture = 1; 
            ++nobjects;
             
            /*
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make(-50.0,   0.0, 150.0);
            obj[nobjects].tri[0].make(  50.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 150.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            obj[nobjects].ttexture = 1; 
            ++nobjects;
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(  50.0, 100.0, 200.0);
            obj[nobjects].tri[1].make(  50.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 150.0);
          
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            obj[nobjects].ttexture = 1; 
            ++nobjects;
            */
            //RED RIGHT FACE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make(-40.0,   0.0, 130.0);
            obj[nobjects].tri[0].make(  10.0,   0.0, 180.0);
            obj[nobjects].tri[2].make(-40.0, 100.0, 130.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal); 
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 0;
            obj[nobjects].color[2] = 0; 
            ++nobjects;
            ///top triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make( 10.0,   100.0, 180.0);
            obj[nobjects].tri[1].make( 10.0,   0.0, 180.0);
            obj[nobjects].tri[2].make(-40.0, 100.0, 130.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 0;
            obj[nobjects].color[2] = 0; 
            ++nobjects;
            //YELLOW FACE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make(  0.0, 0.0, 200.0);
            obj[nobjects].tri[0].make( 10.0, 0.0, 180.0);
            obj[nobjects].tri[2].make(  0.0, 100.0, 200.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            ++nobjects;
    
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make( 10.0, 100.0, 180.0);
            obj[nobjects].tri[1].make( 10.0,   0.0, 180.0);
            obj[nobjects].tri[2].make(  0.0, 100.0, 200.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            ++nobjects;
            //BLUE TOP FACE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make(  -50.0, 100.0, 150.0);
            obj[nobjects].tri[0].make(  -40.0, 100.0, 130.0);
            obj[nobjects].tri[2].make(  0.0, 100.0, 200.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 50;
            obj[nobjects].color[2] = 255;
            ++nobjects;
             
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make( 0.0, 100.0, 200.0); 
            obj[nobjects].tri[1].make( 10.0, 100.0, 180.0);
            obj[nobjects].tri[2].make( -40.0, 100.0, 130.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 50;
            obj[nobjects].color[2] = 255;
            ++nobjects;
            
            g.light_pos.make(0, 1000, 0);
			g.ambient.make(.5, .5, .5);
			g.diffuse.make(.6, .6, .6);
			//
			//cam.pos.make(-30.0, 150.0, 600.0);
	        cam.pos.make(0.0, 150.0, 600.0);
            cam.at.make(0.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 40.0;
            break;
        case XK_0:
            nobjects = 0;
			//floor
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(0.0, -20.0, 0.0);
			//slant floor
            obj[nobjects].normal.make(0.0, 1.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 2000.0;
			//obj[nobjects].color[0] = 255;
			//obj[nobjects].color[1] = 150;
			//obj[nobjects].color[2] = 20;
			obj[nobjects].checker = 1;
            obj[nobjects].checker_color[0].make(0,0,0);
			obj[nobjects].checker_color[1].make(1,1,1);
            obj[nobjects].checker_size = 90.0;
            ++nobjects; 
            //MIDDLE RECTANGLE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            /*
            obj[nobjects].tri[1].make(-50.0,   0.0, 150.0);
            obj[nobjects].tri[0].make(  0.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 150.0);*/
            obj[nobjects].tri[1].make(-60.0,   0.0, 100.0);
            obj[nobjects].tri[0].make( 10.0,   0.0, 100.0);
            obj[nobjects].tri[2].make(-60.0, 100.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            //obj[nobjects].ttexture = 0; 
            ++nobjects;
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI; 
            /*obj[nobjects].tri[0].make(  0.0, 100.0, 200.0);
            obj[nobjects].tri[1].make(  0.0,   0.0, 200.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 150.0);*/
            obj[nobjects].tri[0].make( 10.0, 100.0, 100.0);
            obj[nobjects].tri[1].make( 10.0,   0.0, 100.0);
            obj[nobjects].tri[2].make(-60.0, 100.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            //obj[nobjects].ttexture = 0; 
            ++nobjects;
            //TOP RECTANGLE
            //bot tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[0].make(-50.0, 110.0, 100.0);
            obj[nobjects].tri[1].make(  0.0, 100.0, 100.0);
            obj[nobjects].tri[2].make(-50.0, 100.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            //obj[nobjects].ttexture = 0; 
            ++nobjects;
            //top tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make(  0.0, 100.0, 100.0);
            obj[nobjects].tri[0].make(  0.0, 110.0, 100.0);
            obj[nobjects].tri[2].make(-50.0, 110.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            //obj[nobjects].ttexture = 0; 
            ++nobjects; 
            
            //BOTTOM RECTANGLE
            //top tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[0].make(-50.0, 0.0, 100.0);
            obj[nobjects].tri[1].make(  0.0, 0.0, 100.0);
            obj[nobjects].tri[2].make(-50.0, -10.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            //obj[nobjects].ttexture = 0; 
            ++nobjects;
            //bot tri 
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(  0.0, 0.0, 100.0);
            obj[nobjects].tri[1].make(  0.0, -10.0, 100.0);
            obj[nobjects].tri[2].make(-50.0, -10.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;
            //obj[nobjects].ttexture = 0;   
            ++nobjects;   
            
            //URL: https://stackoverflow.com/questions/5369507/opengl-es-1-0-2d-rounded-rectangle 
            //ROUND TOP-LEFT CORNER
            //bot tri
            //float tr_angle = 180.0;
            //ang_diff = 90.0/10; 
            //float x = -50 + 10*cos(100*(PI/180))
            //float y =  100 + 10*sin(100*(pi/180));
            
            for (int i=0; i<10; i++) { 
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  -50.0 + 10*cos((90.0+i*9)*(PI/180.0)),
                                            100.0 + 10*sin((90.0+i*9)*(PI/180.0)),
                                            100.0);
                

                obj[nobjects].tri[1].make( -50.0, 100.0, 100.0);
                //(-50,110) -> (-60,100)
                //mult 10 for ang_diff
                obj[nobjects].tri[2].make(  -50.0 + 10*cos((90.0+(i+1)*9)*(PI/180.0)),
                                            100.0 + 10*sin((90.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 0;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 0;
                ++nobjects;  
            }
            
            /*
            //test 
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(  10.0, 100.0, 100.0);
            obj[nobjects].tri[1].make(  0.0, 100.0, 100.0);
            obj[nobjects].tri[2].make( 0.0, 110.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 0;
            obj[nobjects].color[2] = 0;
            obj[nobjects].ttexture = 0;  
            ++nobjects;   
            */
 
            //ROUND TOP-RIGHT CORNER
            //
            for (int i=0; i<10; i++) { 
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  0.0 + 10*cos((0.0+(i*9))*(PI/180.0)),
                                            100.0 + 10*sin((0.0+(i*9))*(PI/180.0)),
                                            100.0);
               
                obj[nobjects].tri[1].make(  0.0, 100.0, 100.0);
                //(10,100) -> (0,110)
                //mult 10 for ang_diff
                obj[nobjects].tri[2].make(  0.0 + 10*cos((0.0+(i+1)*9)*(PI/180.0)),
                                            100.0 + 10*sin((0.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 0;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 0;  
                ++nobjects;  
            }

            //ROUND BOTTOM-LEFT CORNER
            //
            for (int i=0; i<10; i++) { 
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  -50.0 + 10*cos((180.0+i*9)*(PI/180.0)),
                                            0.0 + 10*sin((180.0+i*9)*(PI/180.0)),
                                            100.0);
                

                obj[nobjects].tri[1].make( -50.0, 0.0, 100.0);
                //(-60,0) -> (-50,-10)
                //mult 10 for ang_diff
                obj[nobjects].tri[2].make(  -50.0 + 10*cos((180.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 10*sin((180.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 0;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 0;  
                ++nobjects;  
            }
            
            //ROUND BOTTOM-RIGHT CORNER
            for (int i=0; i<10; i++) { 
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  0.0 + 10*cos((270.0+i*9)*(PI/180.0)),
                                            0.0 + 10*sin((270.0+i*9)*(PI/180.0)),
                                            100.0);
                

                obj[nobjects].tri[1].make( 0.0, 0.0, 100.0);
                //(0,-10) -> (10,0)
                //mult 10 for ang_diff
                obj[nobjects].tri[2].make(  0.0 + 10*cos((270.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 10*sin((270.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 0;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 0;  
                ++nobjects;  
            }

            g.light_pos.make(0, 1000, 0);
			g.ambient.make(.5, .5, .5);
			g.diffuse.make(.6, .6, .6);
			//
			//cam.pos.make(-30.0, 150.0, 600.0);
	        //change x-angle here
            cam.pos.make(0.0, 150.0, 600.0);
            //move camera up on y-axis?
            cam.at.make(0.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 40.0;
            break; 
        case XK_g:
            nobjects = 0;
			//floor
			obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(0.0, -40.0, 0.0);
			//slant floor
            obj[nobjects].normal.make(0.0, 1.0, 0.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 2000.0;
			//obj[nobjects].color[0] = 250;
			//obj[nobjects].color[1] = 250;
			//obj[nobjects].color[2] = 250;
	        //obj[nobjects].stexture = 0;
            //obj[nobjects].specular = 1;
            obj[nobjects].checker = 1;
            //obj[nobjects].shadow = 1;     
            obj[nobjects].checker_color[0].make(0,0,0);
			obj[nobjects].checker_color[1].make(1,1,1);
            obj[nobjects].checker_size = 500.0;
            ++nobjects; 
            
	        //stars
	        for (int i=0; i<400; i++) {
		        obj[nobjects].type = OBJ_TYPE_DISC;
                //subtract x-pos to go left, y-pos to go right
		        obj[nobjects].pos.make(
				                (rnd()*10000.0)-5000.0, (rnd()*1500.0)-300.0, -2000);
		        obj[nobjects].normal.make(0.0, 0.0, 1.0);
		        obj[nobjects].normal.normalize();
		        obj[nobjects].radius = rnd() * 10.0 + 5.0;
		        obj[nobjects].checker = 0;
		        /*
                obj[nobjects].color[0] = rnd() * 50.0 + 200.0;
		        obj[nobjects].color[1] = rnd() * 50.0 + 200.0;
		        obj[nobjects].color[2] = rnd() * 50.0 + 200.0;*/
                obj[nobjects].color[0] = rnd() * 255.0;
		        obj[nobjects].color[1] = rnd() * 255.0;
		        obj[nobjects].color[2] = rnd() * 255.0;
		        obj[nobjects].stexture = 0;
		        obj[nobjects].specular = 0;
		        obj[nobjects].checker = 0;
                obj[nobjects].shadow = 0;     
		        ++nobjects;
	        } 
            
            /* 
            srand(time(0));
            for (int i=0; i<= 100; i++) {
			    obj[nobjects].type = OBJ_TYPE_DISC;
			    obj[nobjects].pos.make( rand()%(g.xres+1 - -g.xres) + -g.xres,
                                        rand()%(g.yres/2+1 - -g.yres) + -g.yres/2,
                                        -2000.0 );
			    obj[nobjects].normal.make(0.0, 0.0, 1.0);
			    obj[nobjects].normal.normalize();
			    obj[nobjects].radius = rand() % 10+1;
			    obj[nobjects].color[0] = 255;
			    obj[nobjects].color[1] = 255;
			    obj[nobjects].color[2] = 255;
          	    ++nobjects;
            } */

            //--Front Face--
            //MIDDLE RECTANGLE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(-210.0, 0.0, 100.0);
            obj[nobjects].tri[1].make(   0.0, 0.0, 100.0);
            obj[nobjects].tri[2].make(-210.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //clipping!!!!
            obj[nobjects].clip[0].cpoint.make(-170.0,10.0,100.0);
            obj[nobjects].clip[0].cradius = 22.0;
            
            obj[nobjects].clip[1].cpoint.make(-170.0,110.0,100.0);
            obj[nobjects].clip[1].cradius = 22.0;
           
            obj[nobjects].clip[2].cpoint.make(-40.0,10.0,100.0);
            obj[nobjects].clip[2].cradius = 22.0;
  
            //bot-mid dot
            obj[nobjects].clip[3].cpoint.make(-105.0,60.0,100.0);
            obj[nobjects].clip[3].cradius = 22.0;
            
            obj[nobjects].clip[4].cpoint.make(-170.0,190.0,100.0);
            obj[nobjects].clip[4].cradius = 22.0; 
            rotate();
            ++nobjects; 	
            //SPHERES 
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-170.0, 10.0, 120.0); 
            //obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
	        obj[nobjects].radius = 35; //29.8
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 0;
            obj[nobjects].specular = 1;
            obj[nobjects].tspecular = 0;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            rotate();
            ++nobjects;
            
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-170.0, 110.0, 120.0); 
            obj[nobjects].radius = 35; 
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 0;
            obj[nobjects].specular = 1;
            obj[nobjects].tspecular = 0;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            rotate();
            ++nobjects;
            
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-40.0, 10.0, 120.0); 
            obj[nobjects].radius = 35; 
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 0;
            obj[nobjects].specular = 1;
            obj[nobjects].tspecular = 0;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            rotate();
            ++nobjects;
            
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-105.0, 60.0, 120.0); 
            obj[nobjects].radius = 35; 
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 0;
            obj[nobjects].specular = 1;
            obj[nobjects].tspecular = 0;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            rotate();
            ++nobjects;
           
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-170.0, 190.0, 120.0); 
            obj[nobjects].radius = 35; 
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 0;
            obj[nobjects].specular = 1;
            obj[nobjects].tspecular = 0;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            rotate();
            ++nobjects;
           
            //
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make(   0.0, 300.0, 100.0);
            obj[nobjects].tri[0].make(   0.0,   0.0, 100.0);
            obj[nobjects].tri[2].make(-210.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //clipping
            obj[nobjects].clip[0].cpoint.make(-40.0,110.0,100.0);
            obj[nobjects].clip[0].cradius = 22.0;
             
            obj[nobjects].clip[1].cpoint.make(-40.0,290.0,100.0);
            obj[nobjects].clip[1].cradius = 22.0;
                
            //top-mid dot
            obj[nobjects].clip[2].cpoint.make(-105.0,240.0,100.0);
            obj[nobjects].clip[2].cradius = 22.0;
            rotate();
            ++nobjects;
            
            //SPHERES
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-40.0, 110.0, 120.0); 
            obj[nobjects].radius = 35; 
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 0;
            obj[nobjects].specular = 1; 
            obj[nobjects].tspecular = 0;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            rotate();
            ++nobjects;
            
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-40.0, 290.0, 120.0); 
            obj[nobjects].radius = 35; 
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 0;
            obj[nobjects].specular = 1;
            obj[nobjects].tspecular = 0;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            rotate();
            ++nobjects;

            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-105.0, 240.0, 120.0); 
            obj[nobjects].radius = 35; 
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 255;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 1;
            obj[nobjects].tspecular = 0;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            rotate();
            ++nobjects;
           
            //TOP RECTANGLE
            //bot tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[0].make(-180.0, 300.0, 100.0);
            obj[nobjects].tri[1].make( -30.0, 300.0, 100.0);
            obj[nobjects].tri[2].make(-180.0, 330.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255; 
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;    
            //clipping
            obj[nobjects].clip[0].cpoint.make(-40.0,290.0,100.0);
            obj[nobjects].clip[0].cradius = 20.0;
            rotate();
            ++nobjects;
            //top tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[0].make(  -30.0, 330.0, 100.0);
            obj[nobjects].tri[1].make( -180.0, 330.0, 100.0);
            obj[nobjects].tri[2].make(  -30.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255; 
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1; 
            obj[nobjects].shadow = 1;     
            //clipping
            obj[nobjects].clip[0].cpoint.make(-40.0,290.0,100.0);
            obj[nobjects].clip[0].cradius = 20.0;
            rotate();
            ++nobjects; 
            
            //BOTTOM RECTANGLE
            //top tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make( -180.0, 0.0, 100.0);
            obj[nobjects].tri[0].make( -30.0, 0.0, 100.0);
            obj[nobjects].tri[2].make( -180.0, -30.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].clip[0].cpoint.make(-170.0,10.0,100.0);
            obj[nobjects].clip[0].cradius = 22.0;
            obj[nobjects].clip[1].cpoint.make(-40.0,10.0,100.0);
            obj[nobjects].clip[1].cradius = 22.0; 
            rotate();
            ++nobjects;
            //bot tri 
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make( -30.0, -30.0, 100.0);
            obj[nobjects].tri[0].make( -180.0, -30.0, 100.0);
            obj[nobjects].tri[2].make( -30.0, 0.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //clipping!!
            obj[nobjects].clip[0].cpoint.make(-40.0,10.0,100.0);
            obj[nobjects].clip[0].cradius = 22.0;
            rotate();
            ++nobjects; 
            
            //--Back Face--
            //MIDDLE RECTANGLE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make(-210.0, 0.0, 30.0);
            obj[nobjects].tri[0].make(   0.0, 0.0, 30.0);
            obj[nobjects].tri[2].make(-210.0, 300.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            rotate();
            ++nobjects;
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[0].make(   0.0, 300.0, 30.0);
            obj[nobjects].tri[1].make(   0.0,   0.0, 30.0);
            obj[nobjects].tri[2].make(-210.0, 300.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255; 
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            rotate();
            ++nobjects;
            
            //TOP RECTANGLE
            //bot tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make(-180.0, 300.0, 30.0);
            obj[nobjects].tri[0].make( -30.0, 300.0, 30.0);
            obj[nobjects].tri[2].make(-180.0, 330.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            rotate();
            ++nobjects;
            //top tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make(  -30.0, 330.0, 30.0);
            obj[nobjects].tri[0].make( -180.0, 330.0, 30.0);
            obj[nobjects].tri[2].make(  -30.0, 300.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            rotate();
            ++nobjects; 
            
            //BOTTOM RECTANGLE
            //top tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[0].make( -180.0, 0.0, 30.0);
            obj[nobjects].tri[1].make( -30.0, 0.0, 30.0);
            obj[nobjects].tri[2].make( -180.0, -30.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0; 
            rotate();
            ++nobjects;
            //bot tri 
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make( -30.0, -30.0, 30.0);
            obj[nobjects].tri[1].make( -180.0, -30.0, 30.0);
            obj[nobjects].tri[2].make( -30.0, 0.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0;  
            rotate();
            ++nobjects; 
            
            //--Top Face--
            //RECTANGLE
            //bottom tri
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(-180.0, 330.0, 100.0);
            obj[nobjects].tri[1].make( -30.0, 330.0, 100.0);
            obj[nobjects].tri[2].make(-180.0, 330.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0; 
            rotate();
            ++nobjects;
            //top tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[0].make( -30.0, 330.0, 30.0);
            obj[nobjects].tri[1].make( -180.0, 330.0, 30.0);
            obj[nobjects].tri[2].make( -30.0, 330.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0; 
            rotate();
            ++nobjects;
            
            //--Bottom Face--
            //RECTANGLE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make(-180.0, -30.0, 100.0);
            obj[nobjects].tri[0].make( -30.0, -30.0, 100.0);
            obj[nobjects].tri[2].make(-180.0, -30.0,  30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0; 
            rotate();
            ++nobjects;
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make( -30.0, -30.0, 30.0);
            obj[nobjects].tri[0].make( -180.0, -30.0, 30.0);
            obj[nobjects].tri[2].make( -30.0, -30.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0; 
            rotate();
            ++nobjects;
            
            //--Left Face--
            //RECTANGLE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make( -210.0, 0.0, 100.0);
            obj[nobjects].tri[0].make( -210.0, 0.0, 30.0);
            obj[nobjects].tri[2].make( -210.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0;
            rotate(); 
            ++nobjects;
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make( -210.0, 300.0, 30.0);
            obj[nobjects].tri[0].make( -210.0, 300.0, 100.0);
            obj[nobjects].tri[2].make( -210.0, 0.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0; 
            rotate();
            ++nobjects;

            //--Right Face--
            //RECTANGLE
            //bottom triangle
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make( 0.0, 0.0, 100.0);
            obj[nobjects].tri[1].make( 0.0, 0.0, 30.0);
            obj[nobjects].tri[2].make( 0.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0; 
            rotate();
            ++nobjects;
            //top triangle
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[0].make( 0.0, 300.0, 30.0);
            obj[nobjects].tri[1].make( 0.0, 300.0, 100.0);
            obj[nobjects].tri[2].make( 0.0, 0.0, 30.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 255;
            obj[nobjects].specular = 0;
            obj[nobjects].tspecular = 1;
            obj[nobjects].shadow = 1;     
            //obj[nobjects].ttexture = 0; 
            rotate();
            ++nobjects;
                     
            //ROUND TOP-LEFT CORNER 
            for (int i=0; i<10; i++) { 
                //FRONT
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[1].make(  -180.0 + 30*cos((90.0+i*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+i*9)*(PI/180.0)),
                                            100.0);

                obj[nobjects].tri[0].make( -180.0, 300.0, 100.0);
                //(-180,330) -> (-210,300) 
                //mult by 30 for to scale
                obj[nobjects].tri[2].make(  -180.0 + 30*cos((90.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;  
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     
                rotate();
                ++nobjects; 
                
                //BACK
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  -180.0 + 30*cos((90.0+i*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+i*9)*(PI/180.0)),
                                            30.0);

                obj[nobjects].tri[1].make( -50.0, 100.0, 30.0);
                //(-180,330) -> (-210,300) 
                obj[nobjects].tri[2].make(  -180.0 + 30*cos((90.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+(i+1)*9)*(PI/180.0)),
                                            30.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     
                rotate();
                ++nobjects;

                //SIDE
                //connecting side rectangles...
                //top tri
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[1].make(  -180.0 + 30*cos((90.0+i*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+i*9)*(PI/180.0)),
                                            100.0);
                obj[nobjects].tri[0].make(  -180.0 + 30*cos((90.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                obj[nobjects].tri[2].make(  -180.0 + 30*cos((90.0+i*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+i*9)*(PI/180.0)),
                                            30.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     
                rotate();
                ++nobjects; 
                //bot tri
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  -180.0 + 30*cos((90.0+i*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+i*9)*(PI/180.0)),
                                            30.0);
                obj[nobjects].tri[1].make(  -180.0 + 30*cos((90.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+(i+1)*9)*(PI/180.0)),
                                            30.0);
                obj[nobjects].tri[2].make(  -180.0 + 30*cos((90.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((90.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     
                rotate();
                ++nobjects;               
            }
 
            //ROUND TOP-RIGHT CORNER
            //
            for (int i=0; i<10; i++) { 
                //FRONT
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[1].make(  -30.0 + 30*cos((0.0+(i*9))*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i*9))*(PI/180.0)),
                                            100.0); 
 
                obj[nobjects].tri[0].make( -30.0, 300.0, 100.0);
                //(0,300) -> (-30,330)
                obj[nobjects].tri[2].make(  -30.0 + 30*cos((0.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     

                //clipping
                obj[nobjects].clip[0].cpoint.make(-40.0,290.0,100.0);
                obj[nobjects].clip[0].cradius = 20.0;
                rotate();
                ++nobjects; 
                
                //BACK
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  -30.0 + 30*cos((0.0+(i*9))*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i*9))*(PI/180.0)),
                                            30.0);
               
                 
                obj[nobjects].tri[1].make( -30.0, 300.0, 30.0);
                //(0,300) -> (-30,330)
                obj[nobjects].tri[2].make(  -30.0 + 30*cos((0.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i+1)*9)*(PI/180.0)),
                                            30.0);
                

                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;  
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     
                rotate();
                ++nobjects; 
                
                //SIDE
                //connecting side rectangles...
                //top tri
                obj[nobjects].type = OBJ_TYPE_TRI; 
                obj[nobjects].tri[1].make(  -30.0 + 30*cos((0.0+(i*9))*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i*9))*(PI/180.0)),
                                            100.0);
                obj[nobjects].tri[0].make(  -30.0 + 30*cos((0.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                obj[nobjects].tri[2].make(  -30.0 + 30*cos((0.0+(i*9))*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i*9))*(PI/180.0)),
                                            30.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     
                rotate();
                ++nobjects; 
                //bot tri  
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  -30.0 + 30*cos((0.0+(i*9))*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i*9))*(PI/180.0)),
                                            30.0);
                obj[nobjects].tri[1].make(  -30.0 + 30*cos((0.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i+1)*9)*(PI/180.0)),
                                            30.0);
                obj[nobjects].tri[2].make(  -30.0 + 30*cos((0.0+(i+1)*9)*(PI/180.0)),
                                            300.0 + 30*sin((0.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     

                rotate();
                ++nobjects;  
            }

            //ROUND BOTTOM-LEFT CORNER
            //
            for (int i=0; i<10; i++) { 
                //FRONT
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[1].make(  -180.0 + 30*cos((180.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+i*9)*(PI/180.0)),
                                            100.0);
                

                obj[nobjects].tri[0].make( -180.0, 0.0, 100.0);
                //(-210,0) -> (-180,-30)
                obj[nobjects].tri[2].make(  -180.0 + 30*cos((180.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;    
           
                //clipping!!
                obj[nobjects].clip[0].cpoint.make(-170.0,10.0,100.0);
                obj[nobjects].clip[0].cradius = 22.0; 
                rotate();
                ++nobjects; 

                //BACK 
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  -180.0 + 30*cos((180.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+i*9)*(PI/180.0)),
                                            30.0);
                

                obj[nobjects].tri[1].make( -180.0, 0.0, 30.0);
                //(-210,0) -> (-180,-30)
                obj[nobjects].tri[2].make(  -180.0 + 30*cos((180.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+(i+1)*9)*(PI/180.0)),
                                            30.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;    
                rotate();
                ++nobjects; 
               
                //SIDE
                //connecting side rectangles... 
                //top tri
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[1].make(  -180.0 + 30*cos((180.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+i*9)*(PI/180.0)),
                                            100.0);
                obj[nobjects].tri[0].make(  -180.0 + 30*cos((180.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                obj[nobjects].tri[2].make(  -180.0 + 30*cos((180.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+i*9)*(PI/180.0)),
                                            30.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;     
                rotate();
                ++nobjects;
                //bot tri 
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make(  -180.0 + 30*cos((180.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+i*9)*(PI/180.0)),
                                            30.0);
                obj[nobjects].tri[1].make(  -180.0 + 30*cos((180.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+(i+1)*9)*(PI/180.0)),
                                            30.0);
                obj[nobjects].tri[2].make(  -180.0 + 30*cos((180.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((180.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;    
                rotate();
                ++nobjects; 
            }
            
            //ROUND BOTTOM-RIGHT CORNER
            for (int i=0; i<10; i++) { 
                //FRONT
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[1].make( -30.0 + 30*cos((270.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+i*9)*(PI/180.0)),
                                            100.0);
                

                obj[nobjects].tri[0].make( -30.0, 0.0, 100.0);
                //(-30,-30)->(0,0)
                obj[nobjects].tri[2].make( -30.0 + 30*cos((270.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;  
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;    
                //clipping!!
                obj[nobjects].clip[0].cpoint.make(-40.0,10.0,100.0);
                obj[nobjects].clip[0].cradius = 22.0;
                rotate();
                ++nobjects; 
                
                //BACK
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make( -30.0 + 30*cos((270.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+i*9)*(PI/180.0)),
                                            30.0);
                
                obj[nobjects].tri[1].make( -30.0, 0.0, 30.0);
                //(-30,-30)->(0,0)
                obj[nobjects].tri[2].make( -30.0 + 30*cos((270.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+(i+1)*9)*(PI/180.0)),
                                            30.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;    
                rotate();
                ++nobjects;  
                
                //SIDE
                //connecting side rectangles... 
                //top tri
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[1].make( -30.0 + 30*cos((270.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+i*9)*(PI/180.0)),
                                            100.0); 
                obj[nobjects].tri[0].make( -30.0 + 30*cos((270.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                obj[nobjects].tri[2].make( -30.0 + 30*cos((270.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+i*9)*(PI/180.0)),
                                            30.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;    
                rotate();
                ++nobjects; 
                //bot tri
                obj[nobjects].type = OBJ_TYPE_TRI;
                obj[nobjects].tri[0].make( -30.0 + 30*cos((270.0+i*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+i*9)*(PI/180.0)),
                                            30.0);
           
                obj[nobjects].tri[1].make( -30.0 + 30*cos((270.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+(i+1)*9)*(PI/180.0)),
                                            30.0);
                obj[nobjects].tri[2].make( -30.0 + 30*cos((270.0+(i+1)*9)*(PI/180.0)),
                                            0.0 + 30*sin((270.0+(i+1)*9)*(PI/180.0)),
                                            100.0);
                getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
                obj[nobjects].color[0] = 255;
                obj[nobjects].color[1] = 255;
                obj[nobjects].color[2] = 255;
                obj[nobjects].specular = 0;
                obj[nobjects].tspecular = 1;
                obj[nobjects].shadow = 1;    
                rotate();
                ++nobjects; 
            }
            
            //MIDDLE BAR 
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(-200.0, 155.0, 101.0);
            obj[nobjects].tri[1].make( -10.0, 155.0, 101.0);
            obj[nobjects].tri[2].make(-200.0, 145.0, 101.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 0;
            obj[nobjects].color[2] = 0;
            obj[nobjects].shadow = 0;    
            rotate();
            ++nobjects;
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[1].make(-200.0, 145.0, 101.0);
            obj[nobjects].tri[0].make( -10.0, 145.0, 101.0);
            obj[nobjects].tri[2].make( -10.0, 155.0, 101.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 0;
            obj[nobjects].color[2] = 0;
            obj[nobjects].shadow = 0;    
            rotate();
            ++nobjects;
                     
            //dat
            g.light_pos.make( 300, 300, 400);
	        g.ambient.make(.5, .5, .5);
			g.diffuse.make(.6, .6, .6); 
            //7 
            //g.ambient.make(.6, .6, .6);
			//g.diffuse.make(.5, .5, .5); 
            
            /* 
            g.light_pos.make( -400, 300, 400);
			g.ambient.make(.3, .3, .3);
			g.diffuse.make(.7, .7, .7);
            */
            
            /*
            g.light_pos.make(0, 1000, 0);
			g.ambient.make(.5, .5, .5);
			g.diffuse.make(.6, .6, .6);
			//
			*/
	         
            //change x-axis can roate around angle
            cam.pos.make(-90.0, 400.0, 600.0);
            //move camera up on y-axis?
            //x-axis move cam to left/right
            cam.at.make(-90.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0;
            
            //front
            cam.pos.make(-90.0, 120.0, 600.0);
            //move camera up on y-axis?
            //x-axis move cam to left/right
            cam.at.make(-90.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0;
            
 
            /*
            cam.pos.make( 300.0, 400.0, 400.0);
            cam.at.make(-200.0, 100.0, 0.0);
	        */	
            /*
            //behind 
            cam.pos.make( 300.0, 400.0, -200.0);
            cam.at.make( -500.0, -100.0, 0.0);
		    //turning	
            cam.pos.make( 300.0, 400.0, 300.0);
            cam.at.make( -400.0, -100.0, 0.0);
		    	
            cam.pos.make( 200.0, 400.0, 400.0);
            cam.at.make( -300.0, -100.0, 0.0);
		    
            cam.pos.make( 150.0, 400.0, 500.0);
            cam.at.make( -200.0, -100.0, 0.0);
		   
            //mid
			cam.pos.make( 90.0, 400.0, 600.0);
            cam.at.make( -90.0, 100.0, 0.0);
		    // 
		    //end?
            cam.pos.make( -800.0, 400.0, 600.0);
            cam.at.make( 500.0, 100.0, 0.0);
	        //
		    cam.pos.make( -800.0, 400.0, 600.0);
            cam.at.make( 700.0, 100.0, 0.0);
	        */	
            /*
            //top-right 
            //change x-axis can roate around angle
            cam.pos.make(400.0, 500.0, 400.0);
            //move camera up on y-axis?
            //x-axis move cam to left/right
            cam.at.make(-200.0, -100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0;
            */      
            
             
            //change x-axis can roate around angle
            cam.pos.make(-400.0, 120.0, 600.0);
            //move camera up on y-axis?
            //x-axis move cam to left/right
            cam.at.make(-70.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0;
             
            
            /* 
            //camera starting pos
	        //change x-axis can roate around angle
            cam.pos.make( 200.0, 400.0, 400.0);
            //rotation around camera
            //move camera up on y-axis?
            //x-axis move cam to left/right
            cam.at.make(-200.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */    
            /*  
            cam.pos.make( 100.0, 400.0, 400.0);
            cam.at.make( -100.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */
            /* 
            //clip angle test
            cam.pos.make( 0.0, 400.0, 500.0);
            cam.at.make(-200.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0;
            */

            /*
            //top-left corner
            cam.pos.make( -400.0, 500.0, 300.0);
            cam.at.make(0.0, 50.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0;
            */  
            /*
            //pic 
            cam.pos.make( -400.0, 500.0, 500.0);
            cam.at.make(-100.0, 200.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */
            /*
            //pic2 
            cam.pos.make( -500.0, 500.0, 600.0);
            cam.at.make(-100.0, 200.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
           */
            /*
            //fall 
            cam.pos.make( -500.0, 500.0, 400.0);
            cam.at.make(-100.0, -100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */ 
            /*
            //further away
            cam.pos.make( -700.0, 500.0, 800.0);
            cam.at.make( 0.0, 200.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */

            /* 
            //back
            cam.pos.make( -400.0, 500.0, -300.0);
            cam.at.make(-200.0, 300.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0;
            */
            /* 
            //bottom-left corner
            cam.pos.make( -500.0, 50.0, 250.0);
            cam.at.make(-100.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */
            /*
            //bottom-right corner
            cam.pos.make( 200.0, 50.0, 200.0);
            cam.at.make(-400.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0;
            */ 
            break;
        case XK_w: 
            //top tri
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(-210.0, 0.0, 100.0);
            obj[nobjects].tri[1].make(   0.0, 0.0, 100.0);
            obj[nobjects].tri[2].make(-210.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 100;
            obj[nobjects].color[2] = 0;
            obj[nobjects].shadow = 1;
      
            //++nobjects;
            
			//clipping!!!!
            //puts hole in shape
            obj[nobjects].nclips = 0;
            obj[nobjects].cpoint.make(-160.0,40.0,100.0);
            obj[nobjects].cradius = 22.0;
            obj[nobjects].nclips++;
            rotate();
            //nobjects=1;
            ++nobjects; 	
            
            //sphere
			/*
            obj[nobjects].type = OBJ_TYPE_SPHERE;
		    obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
	        obj[nobjects].radius = 90.0;
            //obj[nobjects].radius = 75.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 0;
            */
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            //obj[nobjects].pos.make(-160.0, 40.0, 120.0); //30 29 
            //obj[nobjects].pos.make(-160.0, 40.0, 130.0); //36.1 
            //obj[nobjects].pos.make(-160.0, 40.0, 140.0); //45 
            //
            //obj[nobjects].pos.make(-160.0, 40.0, 120.0); 
            obj[nobjects].pos.make(-160.0, 40.0, 125.0); 
           
            //obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
	        obj[nobjects].radius = 34.5; //29.8 
            //obj[nobjects].radius = 29.8;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 1;
            obj[nobjects].shadow = 1;
      

		    //obj[nobjects].spherical_texture = 0;
            //++nobjects;
            //clipping!!!!
            obj[nobjects].nclips = 0;
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            obj[nobjects].nclips++; 
            
            rotate();
            ++nobjects;
             
            //bot tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make(   0.0, 300.0, 100.0);
            obj[nobjects].tri[0].make(   0.0,   0.0, 100.0);
            obj[nobjects].tri[2].make(-210.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 100;
            obj[nobjects].color[2] = 0; 
            obj[nobjects].shadow = 0;
      
            /*
            {
            //Flt ang = obj[nobjects].angle[2];
            Flt ang = (3.14 * 0.25);
            //ang = 0.1;
            Flt s = sin(ang);
            Flt c = cos(ang);
            
            obj[nobjects].mat[0][0] = c;
            obj[nobjects].mat[0][1] = -s;
            obj[nobjects].mat[0][2] = 0.0;

            obj[nobjects].mat[1][0] = s;
            obj[nobjects].mat[1][1] = c;
            obj[nobjects].mat[1][2] = 0.0;

            obj[nobjects].mat[2][0] = 0.0;
            obj[nobjects].mat[2][1] = 0.0;
            obj[nobjects].mat[2][2] = 1.0;
            //++nobjects;
            }
            */
            rotate();
            ++nobjects;
            
            
            g.light_pos.make( 200, 50, 300);
			g.ambient.make(.3, .3, .3);
			g.diffuse.make(.7, .7, .7);
            //
	        
             
            cam.pos.make(-90.0, 100.0, 500.0);
			cam.at.make(-90.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            /* 
            cam.pos.make(100.0, 100.0, 500.0);
			cam.at.make(-200.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */
            /* 
            cam.pos.make(-450.0, 100.0, 90.0);
			cam.at.make( 300.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
             */
            /* 
            cam.pos.make(-300.0, 100.0, 450.0);
			cam.at.make( 0.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */
	        /*
            g.light_pos.make(0, 1000, 0);
			g.ambient.make(.5, .5, .5);
			g.diffuse.make(.6, .6, .6);
			//
	        //change x-axis can roate around angle
            cam.pos.make(-90.0, 100.0, 600.0);
            //move camera up on y-axis?
            //x-axis move cam to left/right
            cam.at.make(-90.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; */
            break;
        case XK_e:  
            nobjects=0;

            //top tri 
            obj[nobjects].type = OBJ_TYPE_TRI;
            obj[nobjects].tri[0].make(-210.0, 0.0, 100.0);
            obj[nobjects].tri[1].make(   0.0, 0.0, 100.0);
            obj[nobjects].tri[2].make(-210.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 255;
            obj[nobjects].color[1] = 100;
            obj[nobjects].color[2] = 0;
            //++nobjects;
            
			//clipping!!!!
            //puts hole in shape
            obj[nobjects].clip[0].cpoint.make(-160.0,40.0,100.0);
            obj[nobjects].clip[0].cradius = 20.0;
            //
            obj[nobjects].clip[1].cpoint.make(-100.0,40.0,100.0);
            obj[nobjects].clip[1].cradius = 20.0;
           
            obj[nobjects].clip[2].cpoint.make(-100.0,140.0,100.0);
            obj[nobjects].clip[2].cradius = 20.0;
            ++nobjects; 	
             
            //sphere
            
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-160.0, 40.0, 120.0); //30 29 
            //obj[nobjects].pos.make(-160.0, 40.0, 130.0); //36.1 
            //obj[nobjects].pos.make(-160.0, 40.0, 140.0); //45 
            //
            //obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
	        obj[nobjects].radius = 29; 
            //obj[nobjects].radius = 75.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 0; 
		    //obj[nobjects].spherical_texture = 0;
            //++nobjects;
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            ++nobjects;
            
            //sphere
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-100.0, 40.0, 120.0); 
            obj[nobjects].radius = 29; 
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 0; 
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            ++nobjects;
            // 
            //bot tri
            obj[nobjects].type = OBJ_TYPE_TRI; 
            obj[nobjects].tri[1].make(   0.0, 300.0, 100.0);
            obj[nobjects].tri[0].make(   0.0,   0.0, 100.0);
            obj[nobjects].tri[2].make(-210.0, 300.0, 100.0);
            getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
            obj[nobjects].color[0] = 0;
            obj[nobjects].color[1] = 255;
            obj[nobjects].color[2] = 0;

            //clipping!!
            obj[nobjects].clip[0].cpoint.make(-100.0,140.0,100.0);
            obj[nobjects].clip[0].cradius = 20.0;
            ++nobjects; 	 
            //sphere
            
            obj[nobjects].type = OBJ_TYPE_SPHERE;
            obj[nobjects].pos.make(-100.0, 140.0, 120.0);  
            obj[nobjects].radius = 29; 
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 0; 
            //clipping!!
            obj[nobjects].cpoint.make(0.0,0.0,100.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            ++nobjects;
             
            
            g.light_pos.make( 200, 50, 300);
			g.ambient.make(.3, .3, .3);
			g.diffuse.make(.7, .7, .7);
            //
	                   
            cam.pos.make(-90.0, 100.0, 500.0);
			cam.at.make(-90.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            
            /* 
            cam.pos.make(-450.0, 100.0, 90.0);
			cam.at.make( 300.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */ 
            /* 
            cam.pos.make(-300.0, 100.0, 450.0);
			cam.at.make( 0.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; 
            */
	        /*
            g.light_pos.make(0, 1000, 0);
			g.ambient.make(.5, .5, .5);
			g.diffuse.make(.6, .6, .6);
			//
	        //change x-axis can roate around angle
            cam.pos.make(-90.0, 100.0, 600.0);
            //move camera up on y-axis?
            //x-axis move cam to left/right
            cam.at.make(-90.0, 100.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 70.0; */
            break;
        case XK_a:
			//a disc
			obj[0].type = OBJ_TYPE_DISC;
			obj[0].pos.make(0.0, 0.0, 0.0);
			obj[0].normal.make(0.0, 0.0, 1.0);
			obj[0].normal.normalize();
			obj[0].radius = 200.0;
			obj[0].color[0] = 255;
			obj[0].color[1] = 100;
			obj[0].color[2] = 0;
			obj[0].checker = 0;
			//++nobjects;
			//clipping!!!!
            //puts hole in shape
            obj[nobjects].nclips = 0;
            obj[nobjects].cpoint.make(0.0,0.0,70.0);
            obj[nobjects].cradius = 90.0;
            obj[nobjects].nclips++;
            //nobjects=1;
            ++nobjects; 	
             
            //sphere
			obj[nobjects].type = OBJ_TYPE_SPHERE;
		    obj[nobjects].pos.make( 0.0, 0.0, 30.0);
            //obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
	        obj[nobjects].radius = 90.0;
            //obj[nobjects].radius = 75.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 0;
		    //obj[nobjects].spherical_texture = 0;
            //++nobjects;
            //clipping!!!!
            
            obj[nobjects].nclips = 0;
            obj[nobjects].cpoint.make(0.0,0.0,0.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            obj[nobjects].nclips++;
            ++nobjects;
            
            /*
			g.light_pos.make(200, 10, 200);
			g.ambient.make(.3, .3, .3);
			g.diffuse.make(.7, .7, .7);
            */
            //light
	        g.light_pos.make(200, 10, 200);
			g.ambient.make(.3, .3, .3);
			g.diffuse.make(.7, .7, .7);
            
            //
            /*
			cam.pos.make(-500.0, 0.0, 30.0);
			cam.at.make(300.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 45.0;
		    */
             
            cam.pos.make(-200.0, 0.0, 800.0);
			cam.at.make( 0.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 45.0; 
            break; 
        case XK_q:
			//a disc
			obj[0].type = OBJ_TYPE_DISC;
			obj[0].pos.make(200.0, 0.0, 0.0);
			obj[0].normal.make(0.0, 0.0, 1.0);
			obj[0].normal.normalize();
			obj[0].radius = 200.0;
			obj[0].color[0] = 255;
			obj[0].color[1] = 100;
			obj[0].color[2] = 0;
			obj[0].checker = 0;
			//++nobjects;
			//clipping!!!!
            //puts hole in shape
            obj[nobjects].nclips = 0;
            obj[nobjects].cpoint.make(0.0,0.0,70.0);
            obj[nobjects].cradius = 90.0;
           
            //obj[nobjects].cpoint.make(0.0,0.0,70.0);
            //obj[nobjects].cradius = 75.0;
            obj[nobjects].nclips++;
            //nobjects=1;
            ++nobjects; 	
            
            //sphere
			obj[nobjects].type = OBJ_TYPE_SPHERE;
		    obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
	        obj[nobjects].radius = 90.0;
            //obj[nobjects].radius = 75.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 0;
		    //obj[nobjects].spherical_texture = 0;
            //++nobjects;
            //clipping!!!!
            obj[nobjects].nclips = 0;
            obj[nobjects].cpoint.make(0.0,0.0,0.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            obj[nobjects].nclips++;
            //
            //
            //NEW
            ++nobjects;
            obj[nobjects].type = OBJ_TYPE_DISC;
			obj[nobjects].pos.make(-150.0, 0.0, 0.0);
			obj[nobjects].normal.make(0.0, 0.0, 1.0);
			obj[nobjects].normal.normalize();
			obj[nobjects].radius = 200.0;
			obj[nobjects].color[0] = 255;
			obj[nobjects].color[1] = 100;
			obj[nobjects].color[2] = 0;
			obj[nobjects].checker = 0;
			//++nobjects;
			
            //clipping!!!!
            //puts hole in shape
            obj[nobjects].nclips = 0;
            obj[nobjects].cpoint.make(0.0,0.0,70.0);
            obj[nobjects].cradius = 90.0;
           
            //obj[nobjects].cpoint.make(0.0,0.0,70.0);
            //obj[nobjects].cradius = 75.0;
            obj[nobjects].nclips++;
            //nobjects=1;
            ++nobjects; 	
            
            //sphere
			obj[nobjects].type = OBJ_TYPE_SPHERE;
		    obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].pos.make(0.0, 0.0, 30.0);
            //obj[nobjects].normal.make(-1.0, 0.0, 0.0);
			//obj[nobjects].normal.normalize();
	        obj[nobjects].radius = 90.0;
            //obj[nobjects].radius = 75.0;
			obj[nobjects].color[0] = 250;
			obj[nobjects].color[1] = 150;
			obj[nobjects].color[2] = 20;
            obj[nobjects].specular = 0;
		    //obj[nobjects].spherical_texture = 0;
            //++nobjects;
            //clipping!!!!
            obj[nobjects].nclips = 0;
            obj[nobjects].cpoint.make(0.0,0.0,0.0);
            obj[nobjects].cnormal.make(0.0,0.0,1.0);
            obj[nobjects].cradius = 0.0;
            obj[nobjects].nclips++;
            ++nobjects;
            
			g.light_pos.make(200, 10, 200);
			g.ambient.make(.3, .3, .3);
			g.diffuse.make(.7, .7, .7);
            //
			cam.pos.make(-200.0, 0.0, 800.0);
			cam.at.make(0.0, 0.0, 0.0);
			cam.up.make(0.0, 1.0, 0.0);
			cam.ang = 45.0;
			break; 
        case XK_equal:
		case XK_minus:
			break;
		case XK_Escape:
			//quitting the program
			return 1;
	}
	return 0;
}
/*
//-50,100/0,100/0,110/-50,110
//
//input: vertices of rectangle (bot-left, bot-right, top-right, top-left)
void rectangle(float v1, float v2, float v3, float v4 )
{
    obj[nobjects].type = OBJ_TYPE_TRI; 
    //bottom triangle
    obj[nobjects].tri[1].make(-50.0, 110.0, 100.0);
    obj[nobjects].tri[0].make(  0.0, 100.0, 100.0);
    obj[nobjects].tri[2].make(-50.0, 100.0, 100.0);
    getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
    obj[nobjects].color[0] = 0;
    obj[nobjects].color[1] = 0;
    obj[nobjects].color[2] = 255;
    obj[nobjects].ttexture = 0; 
    ++nobjects;
    //top triangle
    obj[nobjects].type = OBJ_TYPE_TRI; 
    obj[nobjects].tri[0].make(  0.0, 100.0, 100.0);
    obj[nobjects].tri[1].make(  0.0, 110.0, 100.0);
    obj[nobjects].tri[2].make(-50.0, 110.0, 100.0);
    getTriangleNormal(obj[nobjects].tri, obj[nobjects].normal);
    obj[nobjects].color[0] = 0;
    obj[nobjects].color[1] = 0;
    obj[nobjects].color[2] = 255;
    obj[nobjects].ttexture = 0; 
    ++nobjects; 
}
*/
void physics(void)
{
	//no physics in this program yet.
}
//here1
void rotate() 
{
    //Flt ang = (3.14 * 0.25);
    //ang = .5;
    //ang = -.6;
    
    //Flt ang = g.dom_angle;
    Flt ang = 0.0;
    if (g.capture ==1) { 
        ang += 0.3;
    }

    Flt s = sin(ang);
    Flt c = cos(ang);
    
    /* 
    //row0        
    obj[nobjects].mat[0][0] = c;
    obj[nobjects].mat[0][1] = -s;
    obj[nobjects].mat[0][2] = 0.0;
    //row1
    obj[nobjects].mat[1][0] = s;
    obj[nobjects].mat[1][1] = c;
    obj[nobjects].mat[1][2] = 0.0;
    //row2
    obj[nobjects].mat[2][0] = 0.0;
    obj[nobjects].mat[2][1] = 0.0;
    obj[nobjects].mat[2][2] = 1.0;
    */
    
    obj[nobjects].mat[0][0] = 1.0;
    obj[nobjects].mat[0][1] = 0.0;
    obj[nobjects].mat[0][2] = 0.0;

    obj[nobjects].mat[1][0] = 0.0;
    obj[nobjects].mat[1][1] = c;
    obj[nobjects].mat[1][2] = -s;
    
    obj[nobjects].mat[2][0] = 0.0;
    obj[nobjects].mat[2][1] = s;
    obj[nobjects].mat[2][2] = c;
}

bool rayIntersectPlane(Vec n, Vec p0, Vec l0, Vec l, Flt &t) 
{ 
	//source online:
	//https://www.scratchapixel.com/lessons/3d-basic-rendering/
	//minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-
	//ray-disk-intersection
	Flt denom = n.dotProduct(l); 
    if (abs(denom) > 0.000001) {
		Vec p0l0;	   	
        p0l0.diff(p0, l0);
        t = p0l0.dotProduct(n) / denom; 
		return (t >= 0.0); 
	}
	return false; 
}

bool rayDiscIntersect(Object *o, Ray *ray, Hit *hit)
{
	Flt t;
	if (rayIntersectPlane(o->normal, o->pos, ray->o, ray->d, t))
	{
		//Calculate the hit point along the ray.
		hit->t = t;
		//(add this to the ray class)
		//hit.t = t;
		hit->p.x = ray->o.x + ray->d.x * t;
		hit->p.y = ray->o.y + ray->d.y * t;
		hit->p.z = ray->o.z + ray->d.z * t;
		//Measure hit from disc center...
		Vec v;
		v.diff(hit->p, o->pos);
		Flt dist = v.len();
		
        /*
		//Is hit point inside radius?
		if (dist <= o->radius) {
			return true;
		}
        */

        //Is hit point inside radius?
		if (dist <= o->radius) {
		    if (o->cradius > 0.0) {
                //check for clipped
                Vec v;
                v.diff(hit->p, o->cpoint);
                Flt len = v.len();
                if (len <= o->cradius)
                     return 0;
            }
            return true;
		}
	}
	return false;
} 

int raySphereIntersect(Object *o, Ray *ray, Hit *hit)
{
	//Log("raySphereIntersect()...\n");
	//Determine if and where a ray intersects a sphere.
	//
	// sphere equation:
	// (p - c) * (p - c) = r * r
	//
	// where:
	// p = point on sphere surface
	// c = center of sphere
	//
	// ray equation:
	// o + d*t
	//
	// where:
	//   o = ray origin
	//   d = ray direction
	//   t = distance along ray, or scalar
	//
	// substitute ray equation into sphere equation
	//
	// (o + t*d - c) * (o + t*d - c) - r * r = 0
	//
	// we want it in this form:
	// a*t*t + b*t + c = 0
	//
	// (o + d*t - c)
	// (o + d*t - c)
	// -------------
	// o*o + o*d*t - o*c + o*d*t + d*t*d*t - d*t*c - o*c + c*d*t + c*c
	// d*t*d*t + o*o + o*d*t - o*c + o*d*t - d*t*c - o*c + c*d*t + c*c
	// d*t*d*t + 2(o*d*t) - 2(c*d*t) + o*o - o*c - o*c + c*c
	// d*t*d*t + 2(o-c)*d*t + o*o - o*c - o*c + c*c
	// d*t*d*t + 2(o-c)*d*t + (o-c)*(o-c)
	//
	// t*t*d*d + t*2*(o-c)*d + (o-c)*(o-c) - r*r
	//
	// a = dx*dx + dy*dy + dz*dz
	// b = 2(ox-cx)*dx + 2(oy-cy)*dy + 2(oz-cz)*dz
	// c = (ox-cx)*(ox-cx) + (oy-cy)*(oy-cy) + (oz-cz)*(oz-cz) - r*r
	//
	// now put it in quadratic form:
	// t = (-b +/- sqrt(b*b - 4ac)) / 2a
	//
	//1. a, b, and c are given to you just above.
	//2. Create variables named a,b,c, and assign the values you see above.
	//3. Look how a,b,c are used in the quadratic equation.
	//4. Make your code solve for t.
	//5. Remember, a quadratic can have 0, 1, or 2 solutions.
	//
	
    Flt a = ray->d.x*ray->d.x + ray->d.y*ray->d.y + ray->d.z*ray->d.z;
	Flt b = 2.0 * (ray->o.x - o->pos.x) * ray->d.x +
			2.0 * (ray->o.y - o->pos.y) * ray->d.y +
			2.0 * (ray->o.z - o->pos.z) * ray->d.z;
	Flt c = (ray->o.x - o->pos.x) * (ray->o.x - o->pos.x) +
			(ray->o.y - o->pos.y) * (ray->o.y - o->pos.y) +
			(ray->o.z - o->pos.z) * (ray->o.z - o->pos.z) -
			o->radius*o->radius;
	Flt t0,t1;
	//discriminant
	Flt disc = b * b - 4.0 * a * c;
	if (disc < 0.0) return 0;
	disc = sqrt(disc);
	t0 = (-b - disc) / (2.0*a);
	t1 = (-b + disc) / (2.0*a);
	//
	if (t0 > 0.0) {
		hit->p.x = ray->o.x + ray->d.x * t0;
		hit->p.y = ray->o.y + ray->d.y * t0;
		hit->p.z = ray->o.z + ray->d.z * t0;
		/*
        //sphereNormal(hit->p, o->center, hit->norm);
		hit->t = t0;
		//Calculate surface normal at hit point of sphere
		o->normal.diff(hit->p, o->pos);
		o->normal.normalize(); */

        //check for clipped
        Vec v;
        v.diff(hit->p, o->cpoint);
        Flt dot =  v.dotProduct(o->cnormal);
        if (dot > 0.0)
            //return 0;
            goto check_other_point;
        //sphereNormal(hit->p, o->center, hit->norm);
		hit->t = t0;
        //Calculate surface normal at hit point of sphere
		o->inside=0;
        o->normal.diff(hit->p, o->pos);
		o->normal.normalize();
		
        return 1;
	}

check_other_point:
	if (t1 > 0.0) {
		hit->p.x = ray->o.x + ray->d.x * t1;
		hit->p.y = ray->o.y + ray->d.y * t1;
		hit->p.z = ray->o.z + ray->d.z * t1;
		//
        //check for clipped
        Vec v;
        v.diff(hit->p, o->cpoint);
        Flt dot = v.dotProduct(o->cnormal);
        if (dot > 0.0)
            return 0;  
        o->inside = 1; 
        //
        hit->t = t1;
		//Calculate surface normal at hit point of sphere
		o->normal.diff(hit->p, o->pos);
		o->normal.normalize();
        //
		o->normal.negate();
        //
        return 1;
	}
	return 0;
}

//here

void reflect(Vec I, Vec N, Vec &R)
{
    //URL: http://paulBourke.net/geometry/reflected/
    //Rr = Ri - 2N (Ri . N)
    //
    //I = incident vector
    //N = surface normal
    //R = reflected ray 
    Flt dot = I.dotProduct(N);
    Flt len = 2.0 * -dot;
    R.x = len * N.x + I.x;
    R.y = len * N.y + I.y;
    R.z = len * N.z + I.z;

}

Flt my_noise2(Vec vec)
{
    extern float noise2(float vec[2]);
    float v[2] = { (float)vec.x, (float)vec.y };
    return (Flt)noise2(v);
}

Flt my_noise3(Vec vec)
{
    extern float noise3(float vec[3]);
    float v[3] = { (float)vec.x, (float)vec.y, (float)vec.z };
    return (Flt)noise3(v);
}

void getTriangleNormal(Vec tri[3], Vec &norm)
{
    Vec v0,v1;
    v0.diff(tri[1], tri[0]);
    v1.diff(tri[2], tri[0]);
    norm.crossProduct(v0, v1);
    norm.normalize();
}


int rayPlaneIntersect(Vec center, Vec normal, Ray *ray, Hit *hit)
{
    Vec v0;
    v0.diff(ray->o, center);
    Flt dot1 = v0.dotProduct(normal);
    if (dot1 == 0.0)
        return 0;
    Flt dot2 = ray->d.dotProduct(normal);
    if (dot2 == 0.0)
        return 0;
    hit->t = -dot1 / dot2;
    if (hit->t < 0.0)
        return 0;
    hit->p.x = ray->o.x + hit->t * ray->d.x;
    hit->p.y = ray->o.y + hit->t * ray->d.y;
    hit->p.z = ray->o.z + hit->t * ray->d.z;
    return 1;
}

//
bool pointInTriangle(Vec tri[3], Vec p, Flt *u, Flt *v)
{
    //source: http://blogs.msdn.com/b/rezanour/archive/2011/08/07/
    //
    //This function determines if point p is inside triangle tri.
    //   step 1: 3D half-space tests
    //   step 2: find barycentric coordinates
    //
    Vec cross0, cross1, cross2;
    Vec ba, ca, pa;
    ba.diff(tri[1], tri[0]);
    ca.diff(tri[2], tri[0]);
    pa.diff(p, tri[0]);
    //This is a half-space test
    cross1.crossProduct(ca, pa);
    cross0.crossProduct(ca, ba);
    if (cross0.dotProduct(cross1) < 0.0)
        return false;
    //This is a half-space test
    cross2.crossProduct(ba,pa);
    cross0.crossProduct(ba,ca);
    if (cross0.dotProduct(cross2) < 0.0)
        return false;
    //Point is within 2 half-spaces
    //Get area proportions
    //Area is actually length/2, but we just care about the relationship.
    Flt areaABC = cross0.len();
    Flt areaV = cross1.len();
    Flt areaU = cross2.len();
    *u = areaU / areaABC;
    *v = areaV / areaABC;
    //return true if valid barycentric coordinates.
    return (*u >= 0.0 && *v >= 0.0 && *u + *v <= 1.0);
}

int rayTriangleIntersect(Object *o, Ray *ray, Hit *hit)
{
    //apply rotation to the ray
    Ray tray;
    tray.o.copy(ray->o);
    tray.d.copy(ray->d);
    //rotate the ray..
    Vec v;
    v.x = tray.o.x * o->mat[0][0] + tray.o.y * o->mat[0][1] + tray.o.z * o->mat[0][2];
    v.y = tray.o.x * o->mat[1][0] + tray.o.y * o->mat[1][1] + tray.o.z * o->mat[1][2];
    v.z = tray.o.x * o->mat[2][0] + tray.o.y * o->mat[2][1] + tray.o.z * o->mat[2][2];
    //v.x += 120.0;
    v.y += g.dom_y;
    tray.o.copy(v);
    v.x = tray.d.x * o->mat[0][0] + tray.d.y * o->mat[0][1] + tray.d.z * o->mat[0][2];
    v.y = tray.d.x * o->mat[1][0] + tray.d.y * o->mat[1][1] + tray.d.z * o->mat[1][2];
    v.z = tray.d.x * o->mat[2][0] + tray.d.y * o->mat[2][1] + tray.d.z * o->mat[2][2];
    tray.d.copy(v);

    //if (rayPlaneIntersect(o->tri[0], o->normal, ray, hit)) {
    if (rayPlaneIntersect(o->tri[0], o->normal, &tray, hit)) {
        Flt u,v;
        if (pointInTriangle(o->tri, hit->p, &u, &v)) {
            //Flt w = 1.0 - u - v;
            //if (o->surface == SURF_BARY) {
            //  o->color[0] = u;
            //  o->color[1] = v;
            //  o->color[2] = w;
            //}
            /*
            //Calculate the hit point along the ray.
		    hit->t = t;
		    //(add this to the ray class)
		    hit->p.x = ray->o.x + ray->d.x * t;
		    hit->p.y = ray->o.y + ray->d.y * t;
		    hit->p.z = ray->o.z + ray->d.z * t;
		   
            //Measure hit from disc center...
		    Vec v[3];
		    v[0].diff(hit->p, o->tri[0]);
            v[1].diff(hit->p, o->tri[1]);
            v[2].diff(hit->p, o->tri[2]);

		    Flt dist[3];
            dist[0] = v[0].len();
            dist[1] = v[1].len();
            dist[2] = v[2].len();

            //Is hit point inside radius?
		    if (dist[0] <= o->tri[0] && dist[1] <= o->tri[1] && dist[2] <= o->tri[2]) {
		        if (o->cradius > 0.0) {
                    //check for clipped
                    Vec v2;
                    v2.diff(hit->p, o->cpoint);
                    Flt len = v2.len();
                    if (len <= o->cradius)
                        return 0;
                }
                return true;
		    }
            */ 
            if (o->cradius > 0.0 || 
                o->clip[0].cradius > 0.0 ||
                o->clip[1].cradius > 0.0 ||
                o->clip[2].cradius > 0.0 ||
                o->clip[3].cradius > 0.0 ||
                o->clip[4].cradius > 0.0 ) 
            {
                //check for clipped
                Vec v2;
                v2.diff(hit->p, o->cpoint);
                Flt len = v2.len();
                //
                //
                Vec c0,c1,c2,c3,c4;
                c0.diff(hit->p, o->clip[0].cpoint);
                Flt len0 = c0.len();
                c1.diff(hit->p, o->clip[1].cpoint);
                Flt len1 = c1.len();
                c2.diff(hit->p, o->clip[2].cpoint);
                Flt len2 = c2.len();
                c3.diff(hit->p, o->clip[3].cpoint);
                Flt len3 = c3.len();
                c4.diff(hit->p, o->clip[4].cpoint);
                Flt len4 = c4.len();
                
                //
                if (len <= o->cradius || 
                    len0 <= o->clip[0].cradius ||
                    len1 <= o->clip[1].cradius ||
                    len2 <= o->clip[2].cradius ||
                    len3 <= o->clip[3].cradius ||
                    len4 <= o->clip[4].cradius  )
                {
                    return 0;
                }
            }
            return true;
           
            //return 1;
        }
    }
    return 0;
}

int getShadow(Vec h, Vec v)
{
	Ray ray;
    Hit hit;
	ray.o.make(h.x,h.y,h.z);
	ray.d.make(v.x,v.y,v.z);
	//nudge ray forward just a little...
	ray.o.x += ray.d.x * 0.00001;
	ray.o.y += ray.d.y * 0.00001;
	ray.o.z += ray.d.z * 0.00001;
	        
    for (int k=0; k<nobjects; k++) {
        switch (obj[k].type) {
		case OBJ_TYPE_DISC:
			//Vec hit;
            if (rayDiscIntersect(&obj[k], &ray, &hit)) {
		        return 1;
            }
			break;
        case OBJ_TYPE_SPHERE:
			//Hit hit;
            if (raySphereIntersect(&obj[k], &ray, &hit)) {
                //if (k! g.hit_idx)
                return 1;
            }
            break;
        case OBJ_TYPE_TRI: 
            if (rayTriangleIntersect(&obj[k], &ray, &hit)) {
                return 1;
            }
            break;
        }
   
    }
	return 0;
}

void trace(Ray *ray, Vec &rgb, Flt weight, int level)
{
    if (level > 5)
        return;
    if (weight < 0.01)
        return;
    //Trace a ray through the scene of objects.
	Hit hit, closehit;
	closehit.t = 1e9;
	int idx = -1;

    for (int k=0; k<nobjects; k++) {
		switch (obj[k].type) {
		case OBJ_TYPE_SPHERE:
			//apply rotation to the ray
            {
            Ray tray;
            tray.o.copy(ray->o);
            tray.d.copy(ray->d);
            //rotate the ray..
            Vec v;
            v.x = tray.o.x * obj[k].mat[0][0] + tray.o.y * obj[k].mat[0][1] + tray.o.z * obj[k].mat[0][2];
            v.y = tray.o.x * obj[k].mat[1][0] + tray.o.y * obj[k].mat[1][1] + tray.o.z * obj[k].mat[1][2];
            v.z = tray.o.x * obj[k].mat[2][0] + tray.o.y * obj[k].mat[2][1] + tray.o.z * obj[k].mat[2][2];
            //v.x += 120.0;
            //v.y += 20.0;
            v.y += g.dom_y;
            tray.o.copy(v);
            v.x = tray.d.x * obj[k].mat[0][0] + tray.d.y * obj[k].mat[0][1] + tray.d.z * obj[k].mat[0][2];
            v.y = tray.d.x * obj[k].mat[1][0] + tray.d.y * obj[k].mat[1][1] + tray.d.z * obj[k].mat[1][2];
            v.z = tray.d.x * obj[k].mat[2][0] + tray.d.y * obj[k].mat[2][1] + tray.d.z * obj[k].mat[2][2];
            tray.d.copy(v);
            //switch back to ray? to see spheres
            if (raySphereIntersect(&obj[k], &tray, &hit)) {
				if (hit.t < closehit.t) {
					closehit.t = hit.t;
					closehit.p.copy(hit.p);
					closehit.color.copy(obj[k].color);
					closehit.norm.copy(obj[k].normal);
					g.hit_idx = k;
                    idx = k;
				}
			}
            }
			break;
		case OBJ_TYPE_DISC:
			if (rayDiscIntersect(&obj[k], ray, &hit)) {
				if (hit.t < closehit.t) {
					closehit.t = hit.t;
					closehit.p.copy(hit.p);
					closehit.color.copy(obj[k].color);
					closehit.norm.copy(obj[k].normal);
					g.hit_idx = k;
                    idx = k;
				}
			}
			break;
        case OBJ_TYPE_TRI:
	        if (rayTriangleIntersect(&obj[k], ray, &hit)) {
		        if (hit.t < closehit.t) {
			        closehit.t = hit.t;
			        closehit.p.copy(hit.p);
			        closehit.color.copy(obj[k].color);
			        closehit.norm.copy(obj[k].normal);
			        g.hit_idx = k;
			        idx = k;
		        }
	        }
	        break;
        }
	}
	if (idx < 0) {
		//ray did not hit an object.
        //color the sky
		ray->d.normalize();
        Flt y = ray->d.y;
        if (y<0.0) y = -y;
        //between 0 and 1
        //Flt blue = ray->d.y * 150.0
        //rgb.make(0.3, 0.3, ray->d.y );
     //   rgb.make((1.0-y), (1.0-y), 1.0);
        
        /*
        extern float noise2(float vec[2]);
        float size = 200.0f;
        Vec col;
        int count = 0;
        while (size >= 2.0f) {
            float vec[2] = { (float)closehit.p.x, (float)closehit.p.y };
            vec[0] += 10000.0f;
            vec[1] += 10000.0f;
            //octave 1  
            vec[0] /= size;
            vec[1] /= size;
            float mag = noise2(vec);
            //we have the magnitude 
            mag = mag + 0.6;
            mag = mag / 1.2;
            mag = mag * size;
            col.x = col.x + mag;
            col.y = col.y + mag;
            col.z = col.z + mag;
            ++count;
            size = size / 2.0f;
        }
        col.scale(1.0f/300.0f);
        closehit.color.copy(col); 
        rgb.copy(col);
        */

        //rgb.make(1.0, 1.0-y,1.0);
         
        rgb.normalize();
        return;
	} 
	//Make the hit object be variable o
	Object *o;
	o = &obj[idx];
       
    //set color of object
    if (o->marble == 1) { 
        Vec col;
        float vec[3] = { 
            (float)closehit.p.x, (float)closehit.p.y, (float)closehit.p.z };
  
        //float vec[2] = { (float)closehit.p.x, (float)closehit.p.y }; //2D
        vec[0] /= 20.0f;
        vec[1] /= 30.0f;
        vec[2] /= 30.0f;
        //
        //turbulize the values
        //double val = sin(closehit.p.x/20.0 + closehit.p.y/30.0); //2D
        //double val = sin(
        //        closehit.p.x/15.0 + closehit.p.y/15.0 +closehit.p.z/15.0);
        //
        float size = 200.0f;
        //Vec col;
        int count = 0;
        extern float noise3(float vec[3]);
        while (size >= 2.0f) {
            float vec[3] = {
                (float)(closehit.p.x/20.0 + closehit.p.y/15.0 +closehit.p.z/15.0)};
      
            vec[0] += 10000.0f;
            vec[1] += 10000.0f;
            vec[2] += 10000.0f;
            //octave 1  
            vec[0] /= size;
            vec[1] /= size;
            vec[2] /= size;
         
            float mag = noise3(vec);
            //we have the magnitude 
            mag = mag + 0.6;
            mag = mag / 1.2;
            mag = mag * size;
            col.x = col.x + mag;
            col.y = col.y + mag;
            col.z = col.z + mag;
            ++count;
            size = size / 2.0f;
        }
        double val = sin(col.x);
        val = val + 1.0;
        val = val / 2.0;
        col.x = val;
        col.y = val;
        col.z = val;
        closehit.color.copy(col);
    }
     
    if (o->perlin == 1) { 
        extern float noise2(float vec[2]);
        float size = 200.0f;
        Vec col;
        int count = 0;
        while (size >= 2.0f) {
            float vec[2] = { (float)closehit.p.x, (float)closehit.p.y };
            vec[0] += 10000.0f;
            vec[1] += 10000.0f;
            //octave 1  
            vec[0] /= size;
            vec[1] /= size;
            float mag = noise2(vec);
            //we have the magnitude 
            mag = mag + 0.6;
            mag = mag / 1.2;
            mag = mag * size;
            col.x = col.x + mag;
            col.y = col.y + mag;
            col.z = col.z + mag;
            ++count;
            size = size / 2.0f;
        }
        col.scale(1.0f/300.0f);
        closehit.color.copy(col);
    }

    if (o->perlin3) {
        float sz = 200.0f;
        float size = sz;
        Vec col;
        while (size >= 1.0f) {
            Vec vec(closehit.p);
            vec.add(4000.0);
            //octave
            vec.scale(1.0/size);
            Flt mag = my_noise3(vec);
            //we have the magnitude
            mag = mag + 0.6;
            mag = mag / 1.2;
            mag = mag * size;
            col.add(mag);
            size = size / 2.0f;
        }
        col.scale(1.0f/sz);
        if (o->marble3) {
            col.x = fabs(sin(col.x * PI));
            col.y = fabs(sin(col.y * PI));
            col.z = fabs(sin(col.z * PI));
        }   
        closehit.color.copy(col);
    }

    if (o->perlin2) {
        float size = 200.0f;
        Vec col;
        while (size >= 1.0f) {
            Vec vec(closehit.p);
            vec.add(10000.0);
            vec.scale(1.0/size);
            Flt mag = my_noise2(vec);
            //we have the magnitude
            mag = mag + 0.6;
            mag = mag / 1.2;
            mag = mag * size;
            col.add(mag);
            size = size / 2.0f;
        }
        col.scale(1.0f/300.0f);
        if (o->marble2) {
            col.x = fabs(sin(col.x * PI));
            col.y = fabs(sin(col.y * PI));
            col.z = fabs(sin(col.z * PI));
        }
        closehit.color.copy(col);
    }
   			
	//set color of object
	if (o->checker == 1) {
		Flt x = (closehit.p.x + 10000.0) / o->checker_size;
		Flt z = (closehit.p.z + 10000.0) / o->checker_size;
		int a = (int)x % 2;
		int b = (int)z % 2;
		// 
       
        if (a == b) {
            //closehit.color.copy(o->checker_color[0]);
		       
            Flt u = x;
            Flt v = z;
            int a = (int)u;
            u -= a;
            int b = (int)v;
            v -= b;
            //assertion
            if (u > 1.0) exit(0);
            if (u < 0.0) exit(0);
            //cout << "uv: " << u << " " << v << endl;
                    
            int tx = u * lava.width;
            int ty = v * lava.height;
            unsigned char *p = lava.data + (ty * lava.width * 3 + tx * 3);
            closehit.color.x = *(p+0) / 255.0;
            closehit.color.y = *(p+1) / 255.0;
            closehit.color.z = *(p+2) / 255.0;
        } else {
     	    //closehit.color.copy(o->checker_color[1]);
	        Flt u = x;
            Flt v = z;
            a = (int)u;
            u -= a;
            b = (int)v;
            v -= b;
            if (u > 1.0) exit(0);
            if (u < 0.0) exit(0); 
        
            int tx = u * lava.width;
            int ty = v * lava.height;
            unsigned char *p = lava.data + (ty * lava.width * 3 + tx * 3);
            closehit.color.x = *(p+0) / 255.0;
            closehit.color.y = *(p+1) / 255.0;
            closehit.color.z = *(p+2) / 255.0;
        }
    }
  /* 
    if (o->type == OBJ_TYPE_TRI && o->ttexture) { 
        Flt x = (closehit.p.x + 10000.0)/40;
		Flt z = (closehit.p.z + 10000.0)/40;
	    //Flt z = (closehit.p.z + 10000.0)/40.0;
		//Flt x = (closehit.p.x + 10000.0)/40.0;

        int a = (int)x;
        x -= a;
        int b = (int)z;
        z -= b;
        //assertion
        if (x > 1.0) exit(0);
        if (x < 0.0) exit(0);
              
        //int tx = x * wood.width;
        //int ty = z * wood.height;
        //unsigned char *p = wood.data + (ty * wood.width * 3 + tx * 3);
        //unsigned char *p = wood.data + (tx * wood.height * 3+ ty *3 );
        
        
        
        int tx = x * dom.width;
        int ty = z * dom.height;
        //unsigned char *p = dom.data + (ty * dom.width * 3 + tx * 3);
        unsigned char *p = dom.data + (tx * dom.height * 3+ ty *3 );
        
         
        //closehit.color.x = *(p+0) / 255.0;
        //closehit.color.y = *(p+1) / 255.0;
        //closehit.color.z = *(p+2) / 255.0; 
        closehit.color.x = *(p+0) / 255.0;
        closehit.color.y = *(p+1) / 255.0;
        closehit.color.z = *(p+2) / 255.0;

    }  */


    //if (o->type == OBJ_TYPE_SPHERE) { 
    if (o->type == OBJ_TYPE_SPHERE && o->stexture) { 
        Flt v = (closehit.norm.y + 1.0) /2.0;
        Flt u = atan2(closehit.norm.z, closehit.norm.x);
        u += PI;
        u /= (PI * 2.0);
        int tx = u * earth.width;
        int ty = v * earth.height;
        tx = tx % earth.width;
        ty = ty % earth.height;
        unsigned char *p = earth.data + (ty * earth.width * 3 + tx * 3);
        closehit.color.x = *(p+0) / 255.0;
        closehit.color.y = *(p+1) / 255.0;
        closehit.color.z = *(p+2) / 255.0;
        
    }
    
    //
    //collect all the light on the object	
    //
    //
    if (o->specular == 1 || o->tspecular == 1) {
        //this object is reflective
        Ray tray;
        //recursive call to trace()
        reflect(ray->d, closehit.norm, tray.d);
        Vec trgb;
        tray.o.copy(closehit.p);
        //nudge the ray forward
        tray.o.x += tray.d.x * 0.00001;
        tray.o.y += tray.d.y * 0.00001;
        tray.o.z += tray.d.z * 0.00001;
      
        trace(&tray, trgb, weight*0.5,level+1);
        trgb.scale(weight);    
        //adjust reflections here    
        if (o->specular == 1) 
            trgb.scale(1.0);
        if (o->tspecular == 1) 
            trgb.scale(0.2);

        rgb.add(trgb);
    }   
    
    //
    //specular highlight
    if (o->specular == 1 || o->tspecular == 1) {
        //URL:
        //Blinn Phong lighting model
        Vec lightDir, halfway;
        lightDir.diff(g.light_pos, closehit.p);
        lightDir.normalize();
        ray->d.normalize();
        halfway.diff(lightDir, ray->d);
        halfway.scale(.5);
        halfway.normalize();
        Flt dot = halfway.dotProduct(closehit.norm);
        if(dot>0.0) {
            dot = pow(dot, 512.0);
            rgb.x += 1.0 * dot * weight;
            rgb.y += 1.0 * dot * weight;
            rgb.z += 1.0 * dot * weight;
        }
    }

	//vector to the light source
	Vec v;
	v.diff(g.light_pos, closehit.p);
	v.normalize();
	Flt sdot = closehit.norm.dotProduct(v);
    int shadow = 0;
    if (sdot > 0.0) {
        //are we in a shadow?
	    shadow = getShadow(closehit.p, v);
    }
    
    if (o->shadow == 1) {
        shadow = 0;
    } 
    
    //surface normal, dot product
	Flt dot = v.dotProduct(o->normal);
	//dot += 1.0;
	//dot /= 2.0;
	if (dot < 0.0)
		dot = 0.0;
	//mult diffuse light by the dot
	Vec diff(g.diffuse);
	diff.scale(dot);
	Vec strength;
	strength.add(g.ambient);
	if (!shadow)
		strength.add(diff);
	rgb.x += closehit.color.x * strength.x * weight;
	rgb.y += closehit.color.y * strength.y * weight;
	rgb.z += closehit.color.z * strength.z * weight;
	rgb.clamp(0.0, 1.0);
}

void render()
{   
	if (g.menu) {
        x11.set_color_3i(255, 255, 0);
		x11.drawString(10,22, "Key presses...");
		x11.drawString(10,34, "1 - Disc");
		x11.drawString(10,46, "2 - 3 Discs");
		x11.drawString(10,58, "4 - scene with light");
		x11.drawString(10,70, "5 - animation with specular lights");	
        x11.drawString(10,82, "6 - Perlin noise test");
		x11.drawString(10,94, "7 - perlin marble");	
        x11.drawString(10,106, "8 - vase");
        x11.drawString(10,118, "T - take screenshot/make gif");	
        x11.drawString(10,130, "R - render");
        x11.drawString(10,142, "9 - rectangular prism");	
        x11.drawString(10,154, "0 - 1 rounded rectangular face");
        x11.drawString(10,166, "G - giant domino");	
        x11.drawString(10,178, "A - clipping");
        x11.drawString(10,190, "Q - clipping test");
        x11.drawString(10,202, "W - triangle clipping test"); 
        x11.drawString(10,214, "E - triangle multiple clipping test"); 
        g.menu = 0;
		return;
	} 
	x11.clear_screen();
	//
	//NEW: setup the camera
	//The camera may be set anywhere looking anywhere.
	Flt fyres = (Flt)g.yres;
	Flt fxres = (Flt)g.xres;
	Flt ty = 1.0 / (fyres - 1.0);
	Flt tx = 1.0 / (fxres - 1.0);
	int px = 1;
	int py = 1;
	Vec up(cam.up);
	up.normalize();
	Flt viewAnglex, aspectRatio;
	Flt frustumheight, frustumwidth;
	Vec rgb, dir, left, out;
	out.diff(cam.at, cam.pos);
	out.normalize();
	aspectRatio = fxres / fyres;
	viewAnglex = degreesToRadians(cam.ang * 0.5);
	frustumwidth = tan(viewAnglex);
	frustumheight = frustumwidth / aspectRatio;
	//frustumwidth is half the distance across screen
	//compute the left and up vectors, for stepping across pixels
	left.crossProduct(out, cam.up);
	left.normalize();
	up.crossProduct(left, out);
	//-------------------------------------------------------------------------
	//
	//shoot a ray through every pixel
	//int hy = g.yres/2;
	//int hx = g.xres/2;
	for (int i=g.yres-1; i>=0; i--) {
		for (int j=0; j<g.xres; j++) {
			//int x = j-hx;
			//int y = i-hy;
			//make the ray that shoots through a pixel.
			px = j;
			py = i;
			Ray ray;
			ray.o.copy(cam.pos);
			dir.combine(-frustumheight * (2.0 * (Flt)py*ty - 1.0), up,
			frustumwidth  * (2.0 * (Flt)px*tx - 1.0), left);
			ray.d.add(dir, out);
			ray.d.normalize();
			//
			Vec rgb;
			//
			trace(&ray, rgb, 1.0, 1);
			x11.set_color_3i(rgb.x*255, rgb.y*255, rgb.z*255);
			x11.drawPoint(j, i);
		}
	}
}















