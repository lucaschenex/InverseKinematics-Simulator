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


#include <lib3ds/file.h>
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>


#include <gl/glfw.h>

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


        Link(Vector4f startpoint, Vector4f direction, float length){
            this -> startpoint = startpoint;
            this -> direction = direction;
            this -> length = length;
        };

        void change_link_direction (Vector4f new_direction){
            this -> direction = xyz_normalize(new_direction);
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
    glNormal3d(0, 1, 0);
    for (int x = 0; x < width - 1; x++) {
      for (int z = 0; z < depth - 1; z++) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     (x + z) % 2 == 0 ? DARKBLUE : WHITE);
        glVertex3d(x, 0, z);
        glVertex3d(x+1, 0, z);
        glVertex3d(x+1, 0, z+1);
        glVertex3d(x, 0, z+1);
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
float lookfrmy = 0;
float lookfrmz = 10;

float lookatx = 0;
float lookaty = 0;
float lookatz = -1;

float lookupx = 0;
float lookupy = 1;
float lookupz = 0;

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
std::vector<Link> arm;

// bool mouse_start_flag = false;

float real_x;
float real_y;
float real_z; 

std::vector<char> v;
                                                                                                                                                 
float goal_x;
float goal_y;
float goal_z;

float goal_theta;
float goal_delta = PI/200;

bool on_start = false;

string curve_name = "Tornado";

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
            cout << "you have a problem" << endl;
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

void rotate_links (Matrix4f rotation, int index) {
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
    cout << "rotate_angle is: " << rotate_angle << endl;
    Vector3f cross_product = (Vector3f(init_axis[0], init_axis[1], init_axis[2]).cross(Vector3f(rotate_axis[0], rotate_axis[1], rotate_axis[2])));
    Vector4f centro_axis = Vector4f(cross_product[0], cross_product[1], cross_product[2], 0.0f);

    cout << "origin_axis is: " << arm[0].get_link_direction() << endl;
    cout << "rotate_axis is: " << centro_axis << endl;
    //*************************

    glPushMatrix();
    glTranslatef(link.startpoint[0], link.startpoint[1], link.startpoint[2]);
    glRotatef(rotate_angle, centro_axis[0], centro_axis[1], centro_axis[2]);



    glutSolidSphere(0.18, 20.0f, 20.0f);
    glutSolidCone(0.2, link.length, 10.0f, 10.0f);

    glPopMatrix();
};


GLuint LOadTexture(const char* TextureName){
    GLuint Texture;
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D,Texture); //Binding the texture

    if(glfwLoadTexture2D(TextureName, GLFW_BUILD_MIPMAPS_BIT)){

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

        return Texture;
    }
    else return -1;
}


void draw_wall(){


    glEnable(GL_TEXTURE_2D);
    // Bitmap image;








    glBegin(GL_QUADS);
    glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     WHITE);
        GLfloat lightPosition[] = {9, 5, 9, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    GLfloat lightPosition2[] = {4, 21, 4, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition2);
    /* Ceiling */
    glVertex3f(0,21,0);
    glVertex3f(0,21,7);
    glVertex3f(7,21,7);
    glVertex3f(7,21,0);
    /* Walls */
    glMaterialfv(GL_FRONT, GL_AMBIENT,
                     BLUE);
    glVertex3f(0,0,0);
    glVertex3f(0,21,0);
    glVertex3f(7,21,0);
    glVertex3f(7,0,0);
    glMaterialfv(GL_FRONT, GL_AMBIENT,
                     SKYBLUE);
    glVertex3f(0,0,0);
    glVertex3f(0,21,0);
    glVertex3f(0,21,7);
    glVertex3f(0,0,7);

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

    checkerboard.create();

    Vector4f a = Vector4f(0,0,0,1);
    Vector4f a1 = Vector4f(1.0,0,0,1);
    Vector4f b0 = Vector4f(1.5,0,0,1);
    Vector4f b = Vector4f(2,0,0,1);
    Vector4f c = Vector4f(3,0,0,1);
    Vector4f e = Vector4f(4,0,0,1);
    Vector4f d = Vector4f(1,0,0,0);

    number_of_arms = 6;
    Link l0 = Link(a,d,1);
    arm.push_back(l0);
    Link l00 = Link(a1, d, 0.5);
    arm.push_back(l00);
    Link l1 = Link(b0,d,0.5);
    arm.push_back(l1);
    Link l2 = Link(b,d,1);
    arm.push_back(l2);
    Link l3 = Link(c,d,1);
    arm.push_back(l3);
    Link l4 = Link(e,d,1);
    arm.push_back(l4);
};


float cal_distance(Vector4f a, Vector4f b){
    float distance = sqrt((b - a)[0]*(b - a)[0]+(b - a)[1]*(b - a)[1]+(b - a)[2]*(b - a)[2]);
    return distance;
};

void reachGoal(Vector4f goal){
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

    if( reachable_length > totalLength){
        cout << "out of reach !! " << endl;
        Vector4f straight_direction = goal - arm[0].startpoint;
        // straight_direction = xyz_normalize(straight_direction);
        for (int i = 0; i <number_of_arms;i++){
            arm[i].change_link_direction(straight_direction);

        }
        Matrix4f non_rotation;
        non_rotation<<1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1;
        rotate_links(non_rotation, 0);
    }


    else{

        float debug_angle = 0;
        float debug_dotP = 0;


        while(distance>0.01){
            cout << distance << endl;            

            for (int i = number_of_arms-1; i >-1; i--)
            {

                // cout << distance << endl;
                if (distance>0.01){

                    Vector4f RD = xyz_normalize(goal - arm[i].startpoint);
                    

                    Vector4f end = arm[number_of_arms-1].startpoint + arm[number_of_arms-1].get_link_direction() * arm[number_of_arms-1].length;


                    Vector4f RE = xyz_normalize(end - arm[i].startpoint);


                    float angle = acos(RD.dot(RE));
                    debug_angle = angle;
                    debug_dotP = RD.dot(RE);
                    cout << endl;
                    cout<<angle<<endl;
                    cout << RD.dot(RE) <<endl;
                    if (RD.dot(RE) > 1){
                        cout << "problem is right here!!"<< endl;
                        return;
                    } 
                    cout << endl;
                    if(angle>-0.0001 and angle<0.0001){
                        counter++;
                        if (counter>number_of_arms-1){
                            // break;
                            cout << "loop exit because of angle" << endl;
                            return;
                        }
                    }
                    else{
                        counter = 0;
                    }

                    Matrix4f applyM = rotationM(RD, RE, angle);

                    rotate_links(applyM, i);
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
                    cout << "distance too small, exit with distance: " << distance << endl;
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
        cout << "function exiting with distance: " << distance << endl;
        cout << "function exiting with angle: " << debug_angle << endl; 
        cout << "function exiting with dot product: " << debug_dotP << endl;


    }
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

void myDraw(){

    glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, 1, 0.5, 80);
    // glOrtho(20, 20, 20, 20, 0.5, 20);

    // Set material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, qaRed);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, qaRed);
    glMaterialfv(GL_FRONT, GL_SPECULAR, qaWhite);
    glMaterialf(GL_FRONT, GL_SHININESS, 128.0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, qaLowAmbient);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(lookfrmx, lookfrmy, lookfrmz, lookatx, lookaty, lookatz, lookupx, lookupy, lookupz);

    glRotatef(default_orbit, 1, 0.0f, 0.0f);
    glRotatef(default_orbit2, 0.0f, 1, 0.0f);

    //**************** Draw something segment, function call required ***************

    curve_init(curve_name);

    checkerboard.draw();

    draw_wall();

    for(int i = 0; i < number_of_arms; i++){ 
        drawCone(arm[i]);
    }
    
    Vector3D theta2xyz;

    if (curve_name == "Torus"){
        theta2xyz = Torus_knot(goal_theta);
    }
    else if (curve_name == "Tornado"){
        theta2xyz = Tornado(goal_theta);

    }
    goal_x = theta2xyz.x;
    goal_y = theta2xyz.y;
    goal_z = theta2xyz.z;


    // cout << real_x << "  " << real_y << "  " << real_z << endl;
    Vector4f goal = Vector4f(goal_x, goal_y, goal_z, 1);

    // cout << "now goal's y is " << goal[1] << endl;
    reachGoal(goal);
    

    glMaterialfv(GL_FRONT, GL_AMBIENT, qaGreen);

    drawpoint(goal);

    // cout << "arm's configuration is now: " << arm[3].direction << endl;

    for ( int i = 0; i < number_of_arms; i++)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT, qaBlue);

        glColor3f( 0.95f, 0.207, 0.031f );
        Vector4f point = arm[i].startpoint;
        if (i==number_of_arms-1){
            Vector4f endpoint = arm[number_of_arms-1].startpoint + arm[number_of_arms-1].get_link_direction()*arm[number_of_arms-1].length;
            drawpoint(point);
            // drawline(point, endpoint);
            cout << "end arm directs at: \n" << endpoint-point << endl;
            drawpoint(endpoint);
        }
        else{
            Vector4f point2 = arm[i+1].startpoint;
            drawpoint(point);
            // drawline(point, point42);
        }
    }


    if (on_start){
        goal_theta += goal_delta;

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
            cout << "hear a" << endl;
            adaptive = !adaptive;
            myDraw();
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
            cout << "start!!" << endl;
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


bool load_obj(){

    Lib3dsFile *myfile = lib3ds_file_load( "teapot.3ds" );

    cout << myfile << endl;

    if (!myfile){
        cout << "no file found" << endl;
        return false;
    }

    cout << "file read" << endl;

    Lib3dsMesh *m;


    int Points = 0;
    int Faces = 0;
    int Texcoords = 0;
    int i,j;
    int Meshes = 0;


    for (m = myfile->meshes;m;m=m->next){
        Points += m->points;
        Texcoords+=m->texels;
        Faces+=m->faces;
        Meshes++;   
    }

    cout << "meshes: " << Meshes << endl; 
    cout << "Points: " << Meshes << endl;
    cout << "Faces: " << Meshes << endl;
    cout << "Texcoords: " << Meshes << endl;


    return true;

}

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