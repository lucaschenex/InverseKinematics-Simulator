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
#include <math.h>
#include "Eigen/Core"
#include "Eigen/Dense"
#include <cmath>


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


// #include <lib3ds/file.h>
// #include <lib3ds/camera.h>
// #include <lib3ds/mesh.h>
// #include <lib3ds/node.h>
// #include <lib3ds/material.h>
// #include <lib3ds/matrix.h>
// #include <lib3ds/vector.h>
// #include <lib3ds/light.h>


// #include <Eigen::vectorbase.h>

// #include <lib3ds_impl.h>

#define PI 3.14159265  // Should be used from mathlib
#define checkImageWidth 64
#define checkImageHeight 64
inline float sqr(float x) { return x*x; }

using namespace std;
using namespace Eigen;


//***********************************************
//  Global Constant
//***********************************************

GLfloat WHITE[] = {1, 1, 1};
GLfloat BLACK[] = {0, 0, 0};
GLfloat RED[] = {1, 0, 0};
GLfloat GREEN[] = {0, 1, 0};
GLfloat MAGENTA[] = {1, 0, 1};
GLfloat LAVENDER[] = {0.9,0.9,0.98};
GLfloat PINK[] = {1,0.753,0.8};
GLfloat HOTPINK[] = {1,0.411,0.71};
GLfloat BLUE[] = {0.53,0.808,0.98};
GLfloat DARKBLUE[] = {0.117,0.57,1};
GLfloat SKYBLUE[] = {0.69,0.886,1};

GLfloat qaBlack[] = {0.0, 0.0, 0.0, 1.0};
GLfloat qaGreen[] = {0.0, 1.0, 0.0, 1.0};
GLfloat qaRed[] = {1.0, 0.0, 0.0, 1.0};
GLfloat qaBlue[] = {0.0, 0.0, 1.0, 1.0};
GLfloat qaWhite[] = {1.0, 1.0, 1.0, 1.0};
GLfloat qaLowAmbient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat qaFullAmbient[] = {1.0, 1.0, 1.0, 1.0};


//***********************************************
//  Classes
//***********************************************

Vector4f xyz_normalize(Vector4f);

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
    private:
        Vector4f direction;
    public:
        Vector4f startpoint;
        float length;
        float limit_angle;


        Link(Vector4f startpoint, Vector4f direction, float length, float limit_angle=PI){
            this -> startpoint = startpoint;
            this -> direction = direction;
            this -> length = length;
            this -> limit_angle = limit_angle;
        };

        void change_link_direction (Vector4f new_direction){
            this-> direction = xyz_normalize(new_direction);

            // float current_angle = acos(new_direction.dot(prev.get_link_direction()))

            // if ((direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2])!=1){
                // cout << "normalization wrong!!" << endl;
            // }
        };

        Vector4f get_link_direction(){
            // cout << "direction is: " << direction << endl;
            return direction;
        }
};

class Checkerboard {
  int displayListId;
  int width;
  int depth;
    public:
  Checkerboard(int width, int depth): width(width), depth(depth) {}
  double centerx() {return width / 2;}
  double centerz() {return depth / 2;}
  void create() {
    displayListId = glGenLists(1);
    glNewList(displayListId, GL_COMPILE);
    GLfloat lightPosition[] = {4, 3, 7, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glBegin(GL_QUADS);
    glNormal3d(0, 0, 1);
    for (int x = -width; x < width - 1; x++) {
      for (int y = -depth; y < depth - 1; y++) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     (x + y) % 2 == 0 ? DARKBLUE : WHITE);
        // glVertex3d(x, 0, z);
        // glVertex3d(x+1, 0, z);
        // glVertex3d(x+1, 0, z+1);
        // glVertex3d(x, 0, z+1);

        glVertex3d(x, y, 0);
        glVertex3d(x+1, y, 0);
        glVertex3d(x+1, y+1, 0);
        glVertex3d(x, y+1, 0);
      }
    }
    glEnd();
    glEndList();
  }
  void draw() {
    glCallList(displayListId);
  }
};

//***********************************************
//  Global Variables
//***********************************************
int number_of_patches;
std::vector<Patch> patch_list;

static float zoomFactor = 1;
static float translateFactor = 0.05;
static float orbitDegrees = 5.0;

float delta_step = 0.1;
float fov = 60.0;

float lookfrmx = 0;
float lookfrmy = 7;
float lookfrmz = 4;

float lookatx = 0;
float lookaty = 0;
float lookatz = 4;

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

//*****************************************************
int window_size = 800;

int number_of_arms;
std::vector<Link> arm, arm1, arm2, arm3,arm4;


// bool mouse_start_flag = false;

float factor_x = 0;
float factor_y = 0;
float factor_z = 0;


float real_x;
float real_y;
float real_z; 

std::vector<char> v;
                                                                                                                                                 
float goal_x;
float goal_y;
float goal_z;

float goal_x1;
float goal_y1;
float goal_z1;

float goal_x2;
float goal_y2;
float goal_z2;

float goal_x3;
float goal_y3;
float goal_z3;

float goal_theta;
float goal_theta1;
float goal_delta = PI/200;
float goal_delta1 = PI/300;

bool on_start = false;

string curve_name = "curve";
string curve_name2 = "Torus";
string curve_name3 = "curve_right";
string curve_name4 = "curve_downl";
string curve_name5 = "curve_downr";

Checkerboard checkerboard(8, 8);


//***********************************************
//  Utility function
//***********************************************
Vector4f xyz_normalize(Vector4f a) {
    Vector3f mem = Vector3f(a[0], a[1], a[2]);
    mem.normalize();
    // Vector4f newM = Vector4f(mem[0], mem[1], mem[2], 0);
 


    float x = a[0];
    float y = a[1];
    float z = a[2];
    if (x!=0.0f || y!=0.0f || z!=0.0f) {
    // if (!(x < 0.01f && y < 0.01f && z < 0.01f)){
        float sc = sqrt(x*x+y*y+z*z);
        if (sc < 0.00001){
            // cout << "you have a problem" << endl;
        }
    //         // return 
    //     // }

        x = (x)/sc;
        y = (y)/sc;
        z = (z)/sc;
    }
    Vector4f newM(x,y,z,0);
    return newM;
};

void rotate_links (Matrix4f rotation, int index, vector<Link> & arm) {
    for (int i = index; i < number_of_arms; i++){
        Vector4f old_direction =  arm[i].get_link_direction();
        Vector4f new_direction = rotation * old_direction;
        arm[i].change_link_direction(new_direction);
        // 1 -->  1, 2, 3
        
        if (i != number_of_arms-1){
            Vector4f new_startpoint = arm[i].startpoint + arm[i].length * arm[i].get_link_direction();
            arm[i+1].startpoint = new_startpoint;
        }

    }
};

Matrix4f rotationM(Vector4f a, Vector4f b, float angle){
    float m0, m1, m2, m3, m4, m5, m6, m7, m8, ux, uy, uz;
    Vector3f a3(a[0],a[1],a[2]);
    Vector3f b3(b[0],b[1],b[2]);
    Vector3f u = b3.cross(a3);
    Vector4f ui = xyz_normalize(Vector4f(u[0],u[1],u[2],0));
    ux = ui[0];
    uy = ui[1];
    uz = ui[2];
    // cout<<ui<<endl;

    m0 = cos(angle)+ux*ux*(1-cos(angle));
    m1 = ux*uy*(1-cos(angle)-uz*sin(angle));
    m2 = ux*uz*(1-cos(angle))+uy*sin(angle);
    m3 = uy*ux*(1-cos(angle))+uz*sin(angle);
    m4 = cos(angle)+uy*uy*(1-cos(angle));
    m5 = uy*uz*(1-cos(angle))-ux*sin(angle);
    m6 = uz*ux*(1-cos(angle))-uy*sin(angle);
    m7 = uz*uy*(1-cos(angle))+ux*sin(angle);
    m8 = cos(angle)+uz*uz*(1-cos(angle));

    Matrix4f returnValue; returnValue<<m0, m1, m2, 0, m3, m4, m5, 0, m6, m7, m8, 0, 0, 0, 0, 1;
    return returnValue;
};

void drawline(Vector4f A, Vector4f B){
    glLineWidth(5.0);
    glBegin(GL_LINES);
        glVertex3f(A[0], A[1] ,A[2]);
        glVertex3f(B[0], B[1], B[2]);
    glEnd();
};

Vector3D Torus_knot(float t, int p = 3, int q = 7){
    float x_value;
    float y_value;
    float z_value;

    float theta;
    float r_nut;

    z_value = 3+sin((float)q/p * t);
    theta = t;
    r_nut = 4+cos(q/p * t);

    x_value = r_nut * cos(theta);
    y_value = r_nut * sin(theta);

    return Vector3D(x_value, y_value, z_value);
};

Vector3D Tornado(float t){


    float x_value = 2*cos(3*t);
    float y_value = 2*sin(5*t);
    float z_value = t;
    return Vector3D(x_value, y_value, z_value);
};

Vector3D curve(float t){
    float x_value = 2+ (sqrt(2)/2)*cos(t)-0*sin(t);
    float y_value = 0+sin(t);
    float z_value = 6+ (sqrt(2)/2)*cos(t)-0*sin(t);
    return Vector3D(x_value, y_value, z_value);
};

Vector3D curve_right(float t){
    float x_value = -2- (sqrt(2)/2)*cos(t)-0*sin(t);
    float y_value = 0+sin(t);
    float z_value = 6+ (sqrt(2)/2)*cos(t)-0*sin(t);
    return Vector3D(x_value, y_value, z_value);
};

Vector3D curve_downl(float t){
    // float x_value = -1.5+cos(t);
    // float y_value = 0+sin(t);
    // return Vector3D(x_value, y_value, 3.5);

    if((t-int(t/(2*PI))*2*PI)<PI){

        float x_value = -3.5+t-int(t/(2*PI))*2*PI;
    float y_value = -1;
    cout<<x_value<<"gogogo"<<endl;
    return Vector3D(x_value, y_value, 3.5);
}
else{
            float x_value = -3.5+PI-t+int(t/(2*PI))*2*PI+PI;
    float y_value = -1;
    cout<<x_value<<"downdowndown"<<endl;
    return Vector3D(x_value, y_value, 3.5);
}

};

Vector3D curve_downr(float t){
    float z_value = 3.5+cos(t);
    float x_value = 1+sin(t);
    return Vector3D(x_value, -1, z_value);
};

void drawpoint(Vector4f A){
    glPointSize(10.0);
    glBegin( GL_POINTS );
        glColor3f( 0.95f, 0.207, 0.031f );
        glVertex3f(A[0], A[1], A[2]);
    glEnd();
};

void drawCone(Link link){
    GLfloat x, y, z, angle;
    int j = 0;

    // glShadeModel(GL_SMOOTH);

    // glFrontFace(GL_CW);

    Vector4f direction =  Vector4f(1.0f, 2.0f, 0.0f, 0.0f);

    glEnable(GL_CULL_FACE);


    // glPushMatrix();
    // glTranslatef(2.0f, 4.0f, 0.0f);
    // gluLookAt(1.0f, 2.0f, 0.0f, )

    // mvMatrix.clone();
    // glLoadMatrix();

    //*************************
        // let's suppose we have: 
    // Vector4f direct = Vector4f(1.0f, 2.0f, 0.0f, 0.0f);
    // Vector4f ori_direct = Vector4f(0.0f, 0.0f, 1.0f, 0.0f);
    //*************************



    //*************************
    // Matrix4f rotate_to_direction = (ori_direct, direct, )
    Vector4f rotate_axis = link.get_link_direction();
    Vector4f init_axis = Vector4f(0.0f, 0.0f, 1.0f, 0.0f);
    float dot_product = rotate_axis.dot(init_axis);
    float rotate_angle = acos(dot_product)/PI*180;
    // cout << "rotate_angle is: " << rotate_angle << endl;
    Vector3f cross_product = (Vector3f(init_axis[0], init_axis[1], init_axis[2]).cross(Vector3f(rotate_axis[0], rotate_axis[1], rotate_axis[2])));
    Vector4f centro_axis = Vector4f(cross_product[0], cross_product[1], cross_product[2], 0.0f);

    // cout << "origin_axis is: " << arm[0].get_link_direction() << endl;
    // cout << "rotate_axis is: " << centro_axis << endl;
    //*************************

    glPushMatrix();
    glTranslatef(link.startpoint[0], link.startpoint[1], link.startpoint[2]);
    glRotatef(rotate_angle, centro_axis[0], centro_axis[1], centro_axis[2]);



    glutSolidSphere(0.18, 20.0f, 20.0f);
    glutSolidCone(0.2, link.length, 10.0f, 10.0f);

    glPopMatrix();
};

void draw_wall(){
    glBegin(GL_QUADS);
    glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     WHITE);
    GLfloat lightPosition[] = {9, 5, 9, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    GLfloat lightPosition2[] = {4, 21, 4, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition2);
    /* Ceiling */
    glMaterialfv(GL_FRONT, GL_AMBIENT,
                     BLUE);
    glVertex3f(7,-8,21);
    glVertex3f(-8,-8,21);
    glVertex3f(-8,8,21);
    glVertex3f(7,8,21);
    /* Walls */
    glMaterialfv(GL_FRONT, GL_AMBIENT,
                     BLUE);
    glVertex3f(-8,-8,0);
    glVertex3f(-8,-8,21);
    glVertex3f(7,-8,21);
    glVertex3f(7,-8,0);
    glMaterialfv(GL_FRONT, GL_AMBIENT,
                     SKYBLUE);
    glVertex3f(7,7,0);
    glVertex3f(7,-8,0);
    glVertex3f(7,-8,21);
    glVertex3f(7,7,21);

    glMaterialfv(GL_FRONT, GL_AMBIENT,
                     SKYBLUE);
    glVertex3f(-8,7,0);
    glVertex3f(-8,7,21);
    glVertex3f(-8,-8,21);
    glVertex3f(-8,-8,0);

    glMaterialfv(GL_FRONT, GL_AMBIENT,
                     BLUE);
    glVertex3f(-8,7,0);
    glVertex3f(8,7,0);
    glVertex3f(8,7,21);

        glVertex3f(-8,7,21);


    glEnd();
};

void curve_init(string name){
    float draw_theta;
    Vector3D draw_xyz;
    glBegin(GL_LINE_LOOP);
        for (draw_theta = 0; draw_theta < 6*PI; draw_theta+=PI/150){
            if (name == "Torus"){
                draw_xyz = Torus_knot(draw_theta);
            } 
            else if (name == "Tornado"){
                draw_xyz = Tornado(draw_theta);
            }
            else if (name == "curve"){
                draw_xyz = curve(draw_theta);
            }
            else if (name == "curve_right"){
                draw_xyz = curve_right(draw_theta);
            }
            else if (name == "curve_downl"){
                draw_xyz = curve_downl(draw_theta);
            }
            else if (name == "curve_downr"){
                draw_xyz = curve_downr(draw_theta);
            }
            glVertex3f(draw_xyz.x, draw_xyz.y, draw_xyz.z);
        }

    glEnd();
};


void Initialize() {
    glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    // glOrtho(-1.0, 1.0, -1.0, 1.0, -3.0, 3.0);

    // Lighting set up
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);


    // Set lighting intensity and color
    GLfloat qaAmbientLight[]    = {0.1, 0.1, 0.1, 1.0};
    GLfloat qaDiffuseLight[]    = {0.8, 0.8, 0.8, 1.0};
    GLfloat qaSpecularLight[]   = {0.6, 0.6, 0.6, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);
    glMaterialf(GL_FRONT, GL_SHININESS, 30);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Set the light position
    GLfloat qaLightPosition[]   = {4.0, 2.0, 3.0, 5.0};
    glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition);

    GLfloat qaLightPosition1[]   = {0.0, -4.0, 4.0, 5.0};
    glLightfv(GL_LIGHT1, GL_POSITION, qaLightPosition1);

    checkerboard.create();

    Vector4f a = Vector4f(0,0,6,1);
    Vector4f a1 = Vector4f(1.0,0,6,1);
    // Vector4f b0 = Vector4f(1.5,0,6,1);
    Vector4f b0 = Vector4f(1,0,5,1);
    Vector4f b = Vector4f(2,0,6,1);
    Vector4f c = Vector4f(3,0,6,1);
    Vector4f e = Vector4f(4,0,6,1);
    Vector4f d = Vector4f(1,0,0,0);
    Vector4f db = Vector4f(0,0,-1,0);
    Vector4f dc = Vector4f(-1,0,0,0);

    number_of_arms = 2;
    Link l0 = Link(a,d,1);
    arm.push_back(l0);
    Link l00 = Link(a1, db, 1);
    arm.push_back(l00);
    // Link l1 = Link(b0,dc,0.5);
    // arm.push_back(l1);
    // Link l2 = Link(b,d,1);
    // arm.push_back(l2);
    // Link l3 = Link(c,d,1);
    // arm.push_back(l3);
    // Link l4 = Link(e,d,1);
    // arm.push_back(l4);


    Vector4f a2 = Vector4f(0,0,6,1);
    Vector4f a3 = Vector4f(-1.0,0,6,1);
    // Vector4f b2 = Vector4f(-1.5,0,6,1);
    Vector4f b2 = Vector4f(-1,0,5,1);
    Vector4f b3 = Vector4f(-2,0,6,1);
    Vector4f c2 = Vector4f(-3,0,6,1);
    Vector4f e2 = Vector4f(-4,0,6,1);
    Vector4f d2 = Vector4f(-1,0,0,0);
    Vector4f d2b = Vector4f(0,0,-1,0);
    Vector4f d2c = Vector4f(1,0,0,0);

    Link z0 = Link(a2,d2,1);
    arm1.push_back(z0);
    Link z00 = Link(a3, d2b, 1);
    arm1.push_back(z00);
    // Link z1 = Link(b2,d2c,0.5);
    // arm1.push_back(z1);
    // Link z2 = Link(b3,d2,1);
    // arm1.push_back(z2);
    // Link z3 = Link(c2,d2,1);
    // arm1.push_back(z3);
    // Link z4 = Link(e2,d2,1);
    // arm1.push_back(z4);
    Vector4f a4 = Vector4f(0,0,6.5,1);
    Vector4f a5 = Vector4f(0.0,0,6,1);
    Vector4f b4 = Vector4f(0,0,5.5,1);
    Vector4f b5 = Vector4f(0,0,5,1);
    Vector4f c3 = Vector4f(0,0,4.5,1);
    Vector4f e3 = Vector4f(0,0,4,1);
    Vector4f d3 = Vector4f(0,0,-1,0);

    Link r0 = Link(a4,d3,0.5);
    arm2.push_back(r0);
    Link r00 = Link(a5, d3, 0.5);
    arm2.push_back(r00);
    // Link r1 = Link(b4,d3,0.5);
    // arm2.push_back(r1);
    // Link r2 = Link(b5,d3,0.5);
    // arm2.push_back(r2);
    // Link r3 = Link(c3,d3,0.5);
    // arm2.push_back(r3);


    Vector4f a6 = Vector4f(0,0,5.5,1);
    Vector4f a7 = Vector4f(1.0,0,4.5,1);
    Vector4f b6 = Vector4f(1,-sqrt(2),4.5,1);
    Vector4f b7 = Vector4f(0,0,5,1);
    Vector4f c4 = Vector4f(0,0,4.5,1);
    Vector4f e4 = Vector4f(0,0,4,1);
    Vector4f d4 = Vector4f(sqrt(2)/2,0,-sqrt(2)/2,0);
    Vector4f d4b = Vector4f(0,-1,0,0);
    Vector4f d4c = Vector4f(-sqrt(2)/2,0,sqrt(2)/2,0);

    Link f0 = Link(a6,d4,sqrt(2));
    arm3.push_back(f0);
    Link f00 = Link(a7, d4b, sqrt(2));
    arm3.push_back(f00);
    // Link f1 = Link(b6,d4c,sqrt(0.5));
    // arm3.push_back(f1);
    // Link l2 = Link(b7,d4,0.5);
    // arm2.push_back(l2);
    // Link l3 = Link(c4,d4,0.5);
    // arm2.push_back(l3);

    Vector4f a8 = Vector4f(0,0,5.5,1);
    Vector4f a9 = Vector4f(-1.0,0,4.5,1);
    Vector4f b8 = Vector4f(-1,-sqrt(2),4.5,1);
    Vector4f b9 = Vector4f(0,0,5,1);
    Vector4f c5 = Vector4f(0,0,4.5,1);
    Vector4f e5 = Vector4f(0,0,4,1);
    Vector4f d5 = Vector4f(-sqrt(2)/2,0,-sqrt(2)/2,0);
    Vector4f d5b = Vector4f(0,-1,0,0);
    Vector4f d5c = Vector4f(sqrt(2)/2,0,sqrt(2)/2,0);

    Link fs0 = Link(a8,d5,sqrt(2));
    arm4.push_back(fs0);
    Link fs00 = Link(a9, d5b, sqrt(2));
    arm4.push_back(fs00);
    // Link fs1 = Link(b8,d5c,sqrt(0.5));
    // arm4.push_back(fs1);


};


float cal_distance(Vector4f a, Vector4f b){
    float distance = sqrt((b - a)[0]*(b - a)[0]+(b - a)[1]*(b - a)[1]+(b - a)[2]*(b - a)[2]);
    return distance;
};

void reachGoal(Vector4f goal, vector<Link> & arm){
    float distance = 100;
    float totalLength = 0;
    int counter = 0;
    float difference = 1000;


    for (int i = 0; i <number_of_arms;i++){
        totalLength+=arm[i].length;
    }

    float reachable_length = cal_distance(goal, arm[0].startpoint);

    // cout << "totalLength is: " << totalLength << endl;
    // cout << "reachable_length is: " << reachable_length << endl;
    // cout << endl;

    // if( reachable_length > totalLength){
    //     // cout << "out of reach !! " << endl;
    //     // Vector4f straight_direction = goal - arm[0].startpoint;
    //     // // straight_direction = xyz_normalize(straight_direction);
    //     // for (int i = 0; i <number_of_arms;i++){
    //     //     arm[i].change_link_direction(straight_direction);

    //     // }
    //     // Matrix4f non_rotation;
    //     // non_rotation<<1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1;
    //     // rotate_links(non_rotation, 0, arm);

    // }


    // else{

        float debug_angle = 0;
        float debug_dotP = 0;
        int count = 0;

        while(distance>0.01){
            // cout << distance << endl;            

            for (int i = number_of_arms-1; i >-1; i--)
            {

                // cout << distance << endl;
                if (distance>0.01){
                    // cout<<arm[i].get_link_direction()<<"first"<<endl;

                    Vector4f RD = xyz_normalize(goal - arm[i].startpoint);
                    

                    Vector4f end = arm[number_of_arms-1].startpoint + arm[number_of_arms-1].get_link_direction() * arm[number_of_arms-1].length;


                    Vector4f RE = xyz_normalize(end - arm[i].startpoint);


                    float angle = acos(RD.dot(RE));
                    debug_angle = angle;
                    debug_dotP = RD.dot(RE);
                    // cout << endl;
                    // cout<<angle<<endl;
                    // cout << RD.dot(RE) <<endl;
                    if (RD.dot(RE) > 1){
                        // cout << "problem is right here!!"<< endl;
                        return;
                    } 
                    // cout << endl;
                    if(angle>-0.0001 and angle<0.0001){
                        counter++;
                        if (counter>number_of_arms-1){
                            // break;
                            // cout << "loop exit because of angle" << endl;
                            return;
                        }
                    }
                    else{
                        counter = 0;
                    }

                    Matrix4f applyM = rotationM(RD, RE, angle);

                    float new_angle = acos((-arm[i-1].get_link_direction()).dot(RD));

                    Vector4f dir = arm[i-1].get_link_direction();

                    float cal_angle = acos((-arm[i].get_link_direction()).dot(dir));
                    // cout<<new_angle<<endl;
                    // cout<<(-arm[i-1].get_link_direction()).dot(RD)<<"\n\n"<<endl;


                    if(new_angle>arm[i].limit_angle){
                        applyM = rotationM(RD, RE,(arm[i].limit_angle-cal_angle));
                        // cout<<(-arm[i].get_link_direction())<<"second"<<endl;
                        count++;
                        if(count==number_of_arms-1){
                            return;
                        }
                    }

                    rotate_links(applyM, i, arm);
                    // cout<<(-arm[i].get_link_direction())<<"third"<<endl;
                    // cout<<applyM<<endl;
                    // myDraw();
                    // cout<<arm[0].direction<<endl;
                    distance = cal_distance(end, goal);
                    // cout<<distance<<endl;
                    // if (difference-distance>0){
                    //     difference = distance;
                    // }else{
                    //     break;
                    // }
                } else {
                    // cout << "distance too small, exit with distance: " << distance << endl;
                }

                // myDraw();
                
            }

            // if (counter>number_of_arms-1){
                // break;
            // }


            // if (difference-distance>0){
            //             difference = distance;
            //         }else{
            //             break;
            //         }
        }
        // cout << "function exiting with distance: " << distance << endl;
        // cout << "function exiting with angle: " << debug_angle << endl; 
        // cout << "function exiting with dot product: " << debug_dotP << endl;


    // }
};

Vector3D GetOGLPos(int x, int y){
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
 
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
 
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
 
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
 
    return Vector3D(posX, posY, posZ);
};

void translate(vector<Link> & arm){
    for (int i = 0; i < number_of_arms; ++i)
    {
        arm[i].startpoint[0]+=factor_x;
        arm[i].startpoint[1]+=factor_y;
        arm[i].startpoint[2]+=factor_z;
    }
};

void myDraw(){

    glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, 1, 0.5, 80);
    // glOrtho(20, 20, 20, 20, 0.5, 20);

    // Set material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, BLUE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, BLUE);
    glMaterialfv(GL_FRONT, GL_SPECULAR, qaWhite);
    glMaterialf(GL_FRONT, GL_SHININESS, 128.0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, qaLowAmbient);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(lookfrmx, lookfrmy, lookfrmz, lookatx, lookaty, lookatz, lookupx, lookupy, lookupz);

    glRotatef(default_orbit, 1, 0.0f, 0.0f);
    glRotatef(default_orbit2, 0.0f, 0.0f, 1.0f);

    //**************** Draw something segment, function call required ***************

    curve_init(curve_name);

    curve_init(curve_name3);

    curve_init(curve_name4);

    curve_init(curve_name5);

    checkerboard.draw();

    draw_wall();


    translate(arm);
    translate(arm1);
    translate(arm2);
    translate(arm3);
    translate(arm4);

    for(int i = 0; i < number_of_arms; i++){ 
        drawCone(arm[i]);
        drawCone(arm1[i]);
        drawCone(arm2[i]);
        drawCone(arm3[i]);
        drawCone(arm4[i]);
    }
    
    Vector3D theta2xyz;
    Vector3D theta3xyz;
    Vector3D theta4xyz;
    Vector3D theta5xyz;

    if (curve_name == "Torus"){
        theta2xyz = Torus_knot(goal_theta);
        // theta3xyz = Torus_knot(goal_theta1);

    }
    else if (curve_name == "Tornado"){
        theta2xyz = Tornado(goal_theta);
        // theta3xyz = Tornado(goal_theta1);

    }

    else if (curve_name == "curve"){
        theta2xyz = curve(goal_theta);
        // theta3xyz = curve(goal_theta1);

    }

    theta3xyz = curve_right(goal_theta1);

    theta4xyz = curve_downl(goal_theta1);

    theta5xyz = curve_downr(goal_theta1);

    goal_x = theta2xyz.x;
    goal_y = theta2xyz.y;
    goal_z = theta2xyz.z;

    goal_x1 = theta3xyz.x;
    goal_y1 = theta3xyz.y;
    goal_z1 = theta3xyz.z;

    goal_x2 = theta4xyz.x;
    goal_y2 = theta4xyz.y;
    goal_z2 = theta4xyz.z;

    goal_x3 = theta5xyz.x;
    goal_y3 = theta5xyz.y;
    goal_z3 = theta5xyz.z;

    // cout << real_x << "  " << real_y << "  " << real_z << endl;
    Vector4f goal = Vector4f(goal_x, goal_y, goal_z, 1);
    Vector4f goal1 = Vector4f(goal_x1, goal_y1, goal_z1, 1);
    Vector4f goal3 = Vector4f(goal_x2, goal_y2, goal_z2, 1);
    Vector4f goal4 = Vector4f(goal_x3, goal_y3, goal_z3, 1);

    // Vector4f goal2 = Vector4f(0,0,3.5,1);
    // cout << "now goal's y is " << goal[1] << endl;
    reachGoal(goal1, arm1);
    reachGoal(goal,arm);
    reachGoal(goal3,arm3);
    reachGoal(goal4,arm4);
    // reachGoal(goal2, arm2);

    glMaterialfv(GL_FRONT, GL_AMBIENT, qaGreen);

    // drawpoint(goal);
    // drawpoint(goal1);

    // cout << "arm's configuration is now: " << arm[3].direction << endl;

    for ( int i = 0; i < number_of_arms; i++)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT, qaBlue);

        glColor3f( 0.95f, 0.207, 0.031f );
        Vector4f point = arm[i].startpoint;
        if (i==number_of_arms-1){
            Vector4f endpoint = arm[number_of_arms-1].startpoint + arm[number_of_arms-1].get_link_direction()*arm[number_of_arms-1].length;
            // drawpoint(point);
            // drawline(point, endpoint);
            // cout << "end arm directs at: \n" << endpoint-point << endl;
            // drawpoint(endpoint);
        }
        else{
            Vector4f point2 = arm[i+1].startpoint;
            // drawpoint(point);
            // drawline(point, point42);
        }
    }

        for ( int i = 0; i < number_of_arms; i++)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT, qaBlue);

        glColor3f( 0.95f, 0.207, 0.031f );
        Vector4f point1 = arm1[i].startpoint;
        if (i==number_of_arms-1){
            Vector4f endpoint1 = arm1[number_of_arms-1].startpoint + arm1[number_of_arms-1].get_link_direction()*arm1[number_of_arms-1].length;
        }
        else{
            Vector4f point3 = arm1[i+1].startpoint;
        }
    }


    for ( int i = 0; i < number_of_arms; i++)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT, qaBlue);

        glColor3f( 0.95f, 0.207, 0.031f );
        Vector4f point1 = arm2[i].startpoint;
        if (i==number_of_arms-1){
            Vector4f endpoint2 = arm2[number_of_arms-1].startpoint + arm2[number_of_arms-1].get_link_direction()*arm2[number_of_arms-1].length;
        }
        else{
            Vector4f point4 = arm2[i+1].startpoint;
        }
    }

    if (on_start){
        goal_theta += goal_delta;
        goal_theta1 += goal_delta1;
        glutPostRedisplay();
    }


    //**************** Draw something segment, function call required ***************


    glFlush();
    glutSwapBuffers();
}



void reshape(GLint w, GLint h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, GLfloat(w) / GLfloat(h), 1.0, 150.0);
  glMatrixMode(GL_MODELVIEW);
};


void keyboard_event_handler(unsigned char key, int x, int y){
    switch(key)
    {
        // toggle between filled framewire mode
        case 'w': 
            filled = !filled;
            firstTime = !firstTime;
            myDraw();
            break;

        // toggle between smooth and flat shading mode
        case 's':
            smooth = !smooth;
            firstTime = !firstTime;
            myDraw();
            break;

        // toggle between filled and hidden line mode
        case 'h':
            break;

        // zoom in 
        case '+':
            fov -= zoomFactor;
            myDraw();
            break;

        // zoom out    
        case '-':
            if (fov > 0){
                fov += zoomFactor;
                myDraw();
                break;
            }

        // change tesselation parameter, for showcase only
        case 'q':
            if (adaptive == true){
                adaptive_threshold = adaptive_threshold/2;
            } else {
                delta_step = delta_step/2;
            }
            myDraw();
            break;

        // toggle between uniform and adaptive tesselation, for showcase only
        case 'a':
            // cout << "hear a" << endl;
            adaptive = !adaptive;
            myDraw();
            break;

        case 'r':
            factor_x+=0.1;
            myDraw();
            factor_x=0;
            break;

        case 'l':
            factor_x-=0.1;
            myDraw();
            factor_x=0;
            break;

        case 'u':
            factor_z+=0.1;
            myDraw();
            factor_z=0;
            break;

        case 'd':
            factor_z-=0.1;
            myDraw();
            factor_z=0;
            break;

        case 'f':
            factor_y+=0.1;
            myDraw();
            factor_y=0;
            break;

        case 'b':
            factor_y-=0.1;
            myDraw();
            factor_y=0;
            break;

    }
};

void keyboard_special_handler(int key, int x, int y){
    bool is_shift_pressed = (glutGetModifiers() == GLUT_ACTIVE_SHIFT);

    switch(key){
        case GLUT_KEY_UP:
            if(is_shift_pressed){
                lookfrmz -= translateFactor;
                lookatz -= translateFactor;
                myDraw();
                break;
            } else { 
                default_orbit += orbitDegrees;
                u_d_rotate = 1.0f;
                myDraw();
                break;
            }
        case GLUT_KEY_DOWN:
            if(is_shift_pressed){
                lookfrmz += translateFactor;
                lookatz += translateFactor;
                myDraw();
                break;
            } else {
                default_orbit -= orbitDegrees;
                u_d_rotate = 1.0f;
                myDraw();
                break;
            }
        case GLUT_KEY_LEFT:
            if(is_shift_pressed){
                lookfrmx -= translateFactor;
                lookatx -= translateFactor;
                myDraw();
                break;
            } else {
                default_orbit2 += orbitDegrees;
                l_r_rotate = 1.0f;
                myDraw();
                break;
            }
        case GLUT_KEY_RIGHT:
            if(is_shift_pressed) {
                lookfrmx += translateFactor;
                lookatx += translateFactor;
                myDraw();
                break;
            } else {    
                default_orbit2 -= orbitDegrees;
                l_r_rotate = 1.0f;
                myDraw();
                break;
            }
    }
};

void mouse_event_handler(int button, int state, int x, int y){
    // if (state == GLUT_DOWN){
    //     Vector3D realme = GetOGLPos(x, y);
    //     real_x = realme.x;
    //     real_y = realme.y;
    //     real_z = realme.z;
    // }
    // myDraw();






    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        if(on_start){
            on_start = false;

        } else { 
            // cout << "start!!" << endl;
            on_start = true;
            // goal_y += 0.1;
        }
    }


    myDraw();
};

void motion_callback_handler(int x, int y){
    Vector3D realme = GetOGLPos(x, y);
    real_x = realme.x;
    real_y = realme.y;
    real_z = realme.z;

    myDraw();
};


float convertor(char *argv){
    std::string strin= string(argv);
    return atof( strin.c_str());
};


// bool load_obj(){

//     Lib3dsFile *myfile = lib3ds_file_load( "teapot.3ds" );

//     cout << myfile << endl;

//     if (!myfile){
//         cout << "no file found" << endl;
//         return false;
//     }

//     cout << "file read" << endl;

//     Lib3dsMesh *m;


//     int Points = 0;
//     int Faces = 0;
//     int Texcoords = 0;
//     int i,j;
//     int Meshes = 0;


//     for (m = myfile->meshes;m;m=m->next){
//         Points += m->points;
//         Texcoords+=m->texels;
//         Faces+=m->faces;
//         Meshes++;   
//     }

//     cout << "meshes: " << Meshes << endl; 
//     cout << "Points: " << Meshes << endl;
//     cout << "Faces: " << Meshes << endl;
//     cout << "Texcoords: " << Meshes << endl;


//     return true;

// }

int main(int iArgc, char** cppArgv) {
    
    // if (string(cppArgv[iArgc-1]) == "-a"){
    //     adaptive = true;
    //     adaptive_threshold = convertor(cppArgv[iArgc-2]);
    // }
    // else{
    //     delta_step = convertor(cppArgv[iArgc-1]);
    // }
    // string inputfile = cppArgv[1];


    // loadPatch(inputfile);





    // bool successful_load = load_obj();
    // if (!successful_load){
    //     cout << "load failed" << endl;
    // }

    glutInit(&iArgc, cppArgv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(window_size, window_size);
    glutInitWindowPosition(200, 200);
    glutCreateWindow("Bezier Surface");
    glutKeyboardFunc(keyboard_event_handler);
    glutSpecialFunc(keyboard_special_handler);
    glutMouseFunc(mouse_event_handler);
    glutMotionFunc(motion_callback_handler);

    Initialize();

    glutDisplayFunc(myDraw);

    glutMainLoop();
    return 0;
}