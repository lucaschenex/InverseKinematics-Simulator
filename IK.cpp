#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>ßßß


#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }

using namespace std;

//***********************************************
//  Classes
//***********************************************

class Vector3D {
    public:

    float x, y, z;

    Vector3D (float X=0.0, float Y=0.0, float Z=0.0) {
        float ary[]         = {X, Y, Z};
        std::vector<float> color (ary, ary+sizeof(ary)/sizeof(float));
        x = X;
        y = Y;
        z = Z;
    }
    
    Vector3D operator* (Vector3D const &v) {
        return Vector3D(x * v.x, y * v.y, z * v.z);
    }
    
    Vector3D operator* (float scaler) {
        return Vector3D(x * scaler, y * scaler, z * scaler);
    }
    
    Vector3D operator+ (Vector3D const &v) {
        return Vector3D(x + v.x, y + v.y,  z + v.z);
    }
    
    Vector3D operator- (Vector3D const &v) {
        return Vector3D(x - v.x, y - v.y, z - v.z);
    }
    
    float dotVector3D (const Vector3D &v) {
        return (float) x * v.x + y * v.y + z * v.z;
    }
    
    void rgb_normalize() {
        if (x!=0.0f || y!=0.0f || z!=0.0f) {
            x = x/(x+y+z);
            y = y/(x+y+z);
            z = z/(x+y+z);
        }
    }
    
    void xyz_normalize() {
        if (x!=0.0f || y!=0.0f || z!=0.0f) {
            float sc = sqrt(x*x+y*y+z*z);
            x = (x)/sc;
            y = (y)/sc;
            z = (z)/sc;
        }
    }
};

class Matrix3f {
    float x1, x2, x3;
    float y1, y2, y3;
    float z1;
};

class Patch{
    public:
    Vector3D storage[4][4];
    Patch(){   
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                storage[i][j] = Vector3D(0, 0, 0);
            }
        }

    }
};

class Link {
    public:
        Vector3D startpoint;
        float length;
        Vector3D direction;

        Link(Vector3D startpoint, Vector3D direction, float length){
            this -> startpoint = startpoint;
            this -> direction = direction;
            this -> length = length;
        }

};

void rotate_link(int index, )

//***********************************************
//  Global Variables
//***********************************************
int number_of_patches;
std::vector<Patch> patch_list;

static float zoomFactor = 1;
static float translateFactor = 0.05;
static float orbitDegrees = 5.0;

float delta_step = 0.1;
float fov = 45.0;

float lookfrmx = 0;
float lookfrmy = 10;
float lookfrmz = 2;

float lookatx = 0;
float lookaty = -1;
float lookatz = -1;

float lookupx = 0;
float lookupy = 0;
float lookupz = 1;

float l_r_rotate = 0.0;
float u_d_rotate = 0.0;
float default_orbit = 0.0;
float default_orbit2 = 0.0;
float adaptive_threshold = 0.01;
int depth = 6;

bool filled = false;
bool smooth = false;
bool adaptive = false;

std::vector<std::vector<Vector3D> > point_set; 
std::vector<std::vector<Vector3D> > normal_set; 

bool firstTime = true;


GLuint woDraw;

GLfloat qaBlack[] = {0.0, 0.0, 0.0, 1.0};
GLfloat qaGreen[] = {0.0, 1.0, 0.0, 1.0};
GLfloat qaRed[] = {1.0, 0.0, 0.0, 1.0};
GLfloat qaBlue[] = {0.0, 0.0, 1.0, 1.0};
GLfloat qaWhite[] = {1.0, 1.0, 1.0, 1.0};
GLfloat qaLowAmbient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat qaFullAmbient[] = {1.0, 1.0, 1.0, 1.0};







