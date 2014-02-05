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

//***********************************************
//  Utility function
//***********************************************
std::vector<Vector3D> bezierCurve(Vector3D* curve, GLfloat u){
    Vector3D A,B,C,D,E,p,dp;
    std::vector<Vector3D> value;
    A = curve[0] * (1.0-u) + curve[1] * u;
    B = curve[1] * (1.0-u) + curve[2] * u;
    C = curve[2] * (1.0-u) + curve[3] * u;
    D = A * (1.0-u) + B * u;
    E = B * (1.0-u) + C * u;
    p = D * (1.0-u) + E * u;
    dp = (E - D) * 3;
    
    value.push_back(p);
    value.push_back(dp);
    return value;
};

std::vector<Vector3D> bezierPatch(Vector3D patch[][4], GLfloat u, GLfloat v){
    Vector3D curvev[4];
    Vector3D curveu[4];
    Vector3D n, p, dpu,dpv;
    std::vector<Vector3D> returnValue;
    
    curvev[0] = bezierCurve(patch[0], u)[0];
    curvev[1] = bezierCurve(patch[1], u)[0];
    curvev[2] = bezierCurve(patch[2], u)[0];
    curvev[3] = bezierCurve(patch[3], u)[0];
    
    Vector3D patch_1[4], patch_2[4], patch_3[4], patch_4[4];
    
    patch_1[0] = patch[0][0];
    patch_1[1] = patch[1][0];
    patch_1[2] = patch[2][0];
    patch_1[3] = patch[3][0];
    
    patch_2[0] = patch[0][1];
    patch_2[1] = patch[1][1];
    patch_2[2] = patch[2][1];
    patch_2[3] = patch[3][1];
    
    patch_3[0] = patch[0][2];
    patch_3[1] = patch[1][2];
    patch_3[2] = patch[2][2];
    patch_3[3] = patch[3][2];
    
    patch_4[0] = patch[0][3];
    patch_4[1] = patch[1][3];
    patch_4[2] = patch[2][3];
    patch_4[3] = patch[3][3];
    
    curveu[0] = bezierCurve(patch_1, v)[0];
    curveu[1] = bezierCurve(patch_2, v)[0];
    curveu[2] = bezierCurve(patch_3, v)[0];
    curveu[3] = bezierCurve(patch_4, v)[0];
    
    p = bezierCurve(curvev, v)[0];
    dpu = bezierCurve(curveu, u)[1];
    dpv = bezierCurve(curvev, v)[1];
    
    n = Vector3D(dpu.y*dpv.z-dpu.z*dpv.y, dpu.z*dpv.x-dpu.x*dpv.z, dpu.x*dpv.y-dpu.y*dpv.x);
    n = n*-1;
    n.xyz_normalize();
    
    returnValue.push_back(p);
    returnValue.push_back(n);
    
    return returnValue;
};

std::vector<std::vector<Vector3D> > subdivide(Vector3D patch[][4], GLfloat delta, bool msg){
    GLfloat u,v;
    Vector3D evaluation[2];
    int times = ceil(1/delta);

    std::vector<Vector3D> save_point;
    std::vector<Vector3D> save_normal;
    std::vector<std::vector<Vector3D> > save;
    // Vector3D save[(times+1)*(times+1)];
    
    for(int k=0; k<times+1; k++){
        u = k*delta;
        for(int j=0; j<times+1; j++){
            v = j*delta;
            evaluation[0] = bezierPatch(patch,u,v)[0];
            evaluation[1] = bezierPatch(patch,u,v)[1];
            // glColor3f( 0.95f, 0.207, 0.031f );
            // glVertex3f(evaluation[0].x,evaluation[0].y,evaluation[0].z);
            save_point.push_back(evaluation[0]);
            save_normal.push_back(evaluation[1]);
        }
    }
    save.push_back(save_point);
    save.push_back(save_normal);
    return save;
};

//adaptive
bool distance(Vector3D a, Vector3D b, GLfloat threhold){
    GLfloat distance = (a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z);
    if (sqrt(distance)<threhold){
        return true;
    }
    else{
        return false;
    }
};

void drawline(Vector3D A, Vector3D B){
    glBegin(GL_LINES);
        glVertex3f(A.x, A.y ,A.z );
        glVertex3f(B.x, B.y, B.z);
    glEnd();
}



void test_triangle(Vector3D A, Vector3D B, Vector3D D, GLfloat Au, GLfloat Av, GLfloat Bu, GLfloat Bv, GLfloat Du, GLfloat Dv, Vector3D patch[][4], GLfloat threhold, int now_depth, bool msg){
    Vector3D AB = (A+B)*0.5;
    Vector3D AD = (A+D)*0.5;
    Vector3D BD = (B+D)*0.5;

    Vector3D curveAB = bezierPatch(patch,0.5*(Av+Bv),0.5*(Au+Bu))[0];
    Vector3D curveAD = bezierPatch(patch,0.5*(Av+Dv),0.5*(Au+Du))[0];
    Vector3D curveBD = bezierPatch(patch,0.5*(Bv+Dv),0.5*(Bu+Du))[0];

    Vector3D nor_A = bezierPatch(patch,Av, Au)[1];
    Vector3D nor_B = bezierPatch(patch,Bv, Bu)[1];
    Vector3D nor_D = bezierPatch(patch,Dv, Du)[1];

    bool result_1 = distance(AB,curveAB,threhold);
    bool result_2 = distance(BD,curveBD,threhold);
    bool result_3 = distance(AD,curveAD,threhold);
    
    if (now_depth==0){
        
        if(!msg){
            drawline(A,B);
            drawline(A,D);
            drawline(B,D); 
        } else {
            glBegin(GL_TRIANGLES);
                glNormal3f(nor_A.x, nor_A.y, nor_A.z);
                glVertex3f(A.x, A.y ,A.z);
                glNormal3f(nor_B.x, nor_B.y, nor_B.z);
                glVertex3f(B.x, B.y, B.z);
                glNormal3f(nor_D.x, nor_D.y, nor_D.z);
                glVertex3f(D.x, D.y, D.z);
            glEnd();
        }
    
    }else{
        if (result_1){   
            if(result_2){
                if(result_3){
                    if(!msg){
                        drawline(A,B);
                        drawline(A,D);
                        drawline(B,D); 
                    } else {
                        glBegin(GL_TRIANGLES);
                            glNormal3f(nor_A.x, nor_A.y, nor_A.z);
                            glVertex3f(A.x, A.y ,A.z);
                            glNormal3f(nor_B.x, nor_B.y, nor_B.z);    
                            glVertex3f(B.x, B.y, B.z);
                            glNormal3f(nor_D.x, nor_D.y, nor_D.z);    
                            glVertex3f(D.x, D.y, D.z);
                        glEnd();
                    }
                }
                else{
                    test_triangle(A,B,curveAD,Au,Av,Bu,Bv,0.5*(Au+Du),0.5*(Av+Dv), patch,threhold,now_depth-1, msg);
                    test_triangle(B,D,curveAD,Bu,Bv,Du,Dv,0.5*(Au+Du),0.5*(Av+Dv), patch,threhold,now_depth-1, msg);
                }
            }
            else{
                if (result_3){
                    test_triangle(A,B,curveBD,Au,Av,Bu,Bv,0.5*(Bu+Du),0.5*(Bv+Dv),patch,threhold,now_depth-1, msg);
                    test_triangle(A,D,curveBD,Au,Av,Du,Dv,0.5*(Bu+Du),0.5*(Bv+Dv),patch,threhold,now_depth-1, msg);
                }
                else{
                    test_triangle(A,B,curveAD,Au,Av,Bu,Bv,0.5*(Au+Du),0.5*(Av+Dv),patch,threhold,now_depth-1, msg);
                    test_triangle(B, curveAD, curveBD, Bu,Bv,0.5*(Au+Du),0.5*(Av+Dv) ,0.5*(Bu+Du),0.5*(Bv+Dv),patch, threhold,now_depth-1, msg);
                    test_triangle(D, curveAD, curveBD, Du,Dv, 0.5*(Au+Du),0.5*(Av+Dv) ,0.5*(Bu+Du),0.5*(Bv+Dv), patch, threhold,now_depth-1, msg);
                }
            }
        }
        else{
            
            if(result_2){
                if(result_3){
                    test_triangle(A,D,curveAB,Au,Av,Du,Dv,0.5*(Au+Bu),0.5*(Av+Bv),patch,threhold,depth-1, msg);
                    test_triangle(B,D,curveAB,Bu,Bv,Du,Dv,0.5*(Au+Bu),0.5*(Av+Bv),patch,threhold,depth-1, msg);
                }
                else{
                    test_triangle(D,B,curveAB,Du,Dv,Bu,Bv,0.5*(Au+Bu),0.5*(Av+Bv),patch,threhold,depth-1, msg);
                    test_triangle(A, curveAB, curveAD, Au,Av,0.5*(Au+Bu),0.5*(Av+Bv) ,0.5*(Au+Du),0.5*(Av+Dv),patch, threhold,now_depth-1, msg);
                    test_triangle(D, curveAD, curveAB, Du,Dv,0.5*(Au+Du),0.5*(Av+Dv) ,0.5*(Au+Bu),0.5*(Av+Bv),patch, threhold,now_depth-1, msg);
                }
            }
            else{
                if(result_3){
                    test_triangle(D,A,curveBD,Du,Dv,Au,Av,0.5*(Bu+Du),0.5*(Bv+Dv),patch,threhold,now_depth-1, msg);
                    test_triangle(A, curveBD, curveAB, Au,Av,0.5*(Bu+Du),0.5*(Bv+Dv) ,0.5*(Au+Bu),0.5*(Av+Bv),patch, threhold,now_depth-1, msg);
                    test_triangle(B, curveBD, curveAB, Bu,Bv,0.5*(Bu+Du),0.5*(Bv+Dv) ,0.5*(Au+Bu),0.5*(Av+Bv),patch, threhold,now_depth-1, msg);
                }
                else{
                    test_triangle(curveAB,curveAD,curveBD,0.5*(Au+Bu),0.5*(Av+Bv) ,0.5*(Au+Du),0.5*(Av+Dv),0.5*(Bu+Du),0.5*(Bv+Dv), patch, threhold,now_depth-1, msg);
                    test_triangle(D,curveBD,curveAD,Du,Dv,0.5*(Bu+Du),0.5*(Bv+Dv) ,0.5*(Au+Du),0.5*(Av+Dv),patch,threhold,now_depth-1, msg);
                    test_triangle(A, curveAB, curveAD, Au,Av,0.5*(Au+Bu),0.5*(Av+Bv) ,0.5*(Au+Du),0.5*(Av+Dv),patch, threhold,now_depth-1, msg);
                    test_triangle(B, curveBD, curveAB, Bu,Bv,0.5*(Bu+Du),0.5*(Bv+Dv) ,0.5*(Au+Bu),0.5*(Av+Bv),patch, threhold,now_depth-1, msg);
                }
            }
        }
    }
};

void subdivide_adptive(Vector3D patch[][4], GLfloat threhold, bool msg){
    Vector3D A = patch[0][0];
    Vector3D B = patch[0][3];
    Vector3D C = patch[3][0];
    Vector3D D = patch[3][3];
    //triangle ABD
    test_triangle(A,B,D,0,0,0,1,1,1, patch, threhold, depth, msg);
    //triangle ACD
    test_triangle(A,C,D, 0,0,1,0,1,1,patch, threhold, depth, msg);
};

void update_frame(std::vector<Vector3D> save, std::vector<Vector3D> norm, bool msg, GLfloat delta){
    int times = ceil(1/delta);
    // static bool firstTime = true;

    // drawing routine
    if (!msg){
        for(int i=0;i<(times+1)*(times+1)-1;i++){
            glLineWidth(1); 
            glColor3f(1.0, 0.0, 0.0);

            if (i%(times+1)==times ){
                glBegin(GL_LINES);
                    glVertex3f(save[i].x, save[i].y ,save[i].z );
                    glVertex3f(save[i+times+1].x, save[i+times+1].y, save[i+times+1].z);
                glEnd();
            }

            else if (i>(times+1)*(times)-1){
                glBegin(GL_LINES);
                    glVertex3f(save[i].x, save[i].y ,save[i].z );
                    glVertex3f(save[i+1].x, save[i+1].y, save[i+1].z);
                glEnd();
            }
            else{
                glBegin(GL_LINES);
                    glVertex3f(save[i].x, save[i].y ,save[i].z );
                    glVertex3f(save[i+1].x, save[i+1].y, save[i+1].z);
                glEnd();

                glBegin(GL_LINES);
                    glVertex3f(save[i].x, save[i].y ,save[i].z );
                    glVertex3f(save[i+times+2].x, save[i+times+2].y, save[i+times+2].z);
                glEnd();

                glBegin(GL_LINES);
                    glVertex3f(save[i].x, save[i].y ,save[i].z );
                    glVertex3f(save[i+times+1].x, save[i+times+1].y, save[i+times+1].z);
                glEnd();
            }
        }
    }

    if (msg){
        for(int i=0;i<(times+1)*(times+1)-1;i++){
            glLineWidth(1); 
            glColor3f(1.0, 0.0, 0.0);

            if (i%(times+1)==times ){    
            }

            else if (i>(times+1)*(times)-1){
            }
            else{
                // Vector3D normal1 = SurfaceNormal(norm[i], norm[i+1], norm[i+times+2]);
                glBegin(GL_TRIANGLES);
                    glNormal3f(norm[i].x, norm[i].y, norm[i].z);
                    glVertex3f(save[i].x, save[i].y ,save[i].z );
                    glNormal3f(norm[i+1].x, norm[i+1].y, norm[i+1].z);
                    glVertex3f(save[i+1].x, save[i+1].y, save[i+1].z);
                    glNormal3f(norm[i+times+2].x, norm[i+times+2].y, norm[i+times+2].z);
                    glVertex3f(save[i+times+2].x, save[i+times+2].y, save[i+times+2].z);
                glEnd();

                // Vector3D normal2 = SurfaceNormal(norm[i], norm[i+times+1], norm[i+times+2]);
                glBegin(GL_TRIANGLES);
                    glNormal3f(norm[i].x, norm[i].y, norm[i].z);
                    glVertex3f(save[i].x, save[i].y ,save[i].z );
                    glNormal3f(norm[i+times+1].x, norm[i+times+1].y, norm[i+times+1].z);
                    glVertex3f(save[i+times+1].x, save[i+times+1].y, save[i+times+1].z);
                    glNormal3f(norm[i+times+2].x, norm[i+times+2].y, norm[i+times+2].z);
                    glVertex3f(save[i+times+2].x, save[i+times+2].y, save[i+times+2].z);
                glEnd();
            }
        }
    }

    glFlush();
};

//***********************************************
//  Input Parser
//***********************************************
void loadPatch(std::string file) {
    //store variables and set stuff at the end
    std::ifstream inpfile(file.c_str());
    if(!inpfile.is_open()) {
        std::cout << "Unable to open file" << std::endl;
    } else {
        std::string line;
        Patch patch_under_construction = Patch();
        int patch_row_under_construction = 0;
        while(inpfile.good()) {
            std::vector<std::string> splitline;
            std::string buf;
            std::getline(inpfile,line);
            std::stringstream ss(line);
            while (ss >> buf) {
                splitline.push_back(buf);
            }

            //Read patch number at first line
            if(splitline.size() == 1) {
                // printf("nope");
                number_of_patches = atof(splitline[0].c_str());
            } 
            //End of definition of a patch
            else if(splitline.size() == 0) {
                Patch patch_done_construction = patch_under_construction;
                patch_list.push_back(patch_done_construction);
                patch_under_construction = Patch();
                patch_row_under_construction = 0;
            }
            else {
                // printf("  nope  \n");

                Vector3D v0 = Vector3D( atof(splitline[0].c_str()), atof(splitline[1].c_str()), atof(splitline[2].c_str()) );
                Vector3D v1 = Vector3D( atof(splitline[3].c_str()), atof(splitline[4].c_str()), atof(splitline[5].c_str()) );
                Vector3D v2 = Vector3D( atof(splitline[6].c_str()), atof(splitline[7].c_str()), atof(splitline[8].c_str()) );
                Vector3D v3 = Vector3D( atof(splitline[9].c_str()), atof(splitline[10].c_str()), atof(splitline[11].c_str()) );

                patch_under_construction.storage[0][patch_row_under_construction] = v0;
                patch_under_construction.storage[1][patch_row_under_construction] = v1;
                patch_under_construction.storage[2][patch_row_under_construction] = v2;
                patch_under_construction.storage[3][patch_row_under_construction] = v3;

                patch_row_under_construction++;
            }
        }
        inpfile.close();
    }
};

void Initialize() {
    glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    // glOrtho(-1.0, 1.0, -1.0, 1.0, -3.0, 3.0);
    glPointSize (6.0);

    // Lighting set up
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Set lighting intensity and color
    GLfloat qaAmbientLight[]    = {0.5, 0.5, 0.5, 1.0};
    GLfloat qaDiffuseLight[]    = {0.5, 0.5, 0.5, 1.0};
    GLfloat qaSpecularLight[]   = {0.5, 0.5, 0.5, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);

    // Set the light position
    GLfloat qaLightPosition[]   = {10.0, 8.0, 7.0, 5.0};
    glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition);

    for(int i = 0; i < number_of_patches; i++){
        point_set.push_back(subdivide(patch_list[i].storage, delta_step, true)[0]);
        normal_set.push_back(subdivide(patch_list[i].storage, delta_step, true)[1]);
    }
    // cout << "Initialize complete" << endl;
}

void myDraw(){

    glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);
    // glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, 1, 0.5, 20);

        if (smooth){
            glShadeModel(GL_SMOOTH);
        } else {
            glShadeModel(GL_FLAT);
        }

        // Set material properties
        glMaterialfv(GL_FRONT, GL_AMBIENT, qaGreen);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, qaGreen);
        glMaterialfv(GL_FRONT, GL_SPECULAR, qaWhite);
        glMaterialf(GL_FRONT, GL_SHININESS, 128.0);
        glLightfv(GL_LIGHT0, GL_AMBIENT, qaLowAmbient);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();


        // glPushMatrix();
        gluLookAt(lookfrmx, lookfrmy, lookfrmz, lookatx, lookaty, lookatz, lookupx, lookupy, lookupz);
        // glPushMatrix();
        // glLoadIdentity();
        
        // glLoadIdentity();
        // glPushMatrix();
        // glRotatef(default_orbit, 0.0f, 1.0f, 0.0f);
        // glPushMatrix();
        // glPopMatrix();
        // glPushMatrix();
        // glLoadIdentity();
        glRotatef(default_orbit, u_d_rotate, 0.0f, 0.0f);
        // glPopMatrix();
        glRotatef(default_orbit, 0.0f, l_r_rotate, 0.0f);
        // glRotatef(default_orbit, u_d_rotate, l_r_rotate, 0.0f);
        // glPopMatrix();


    if(firstTime){
        woDraw = glGenLists(1);
        firstTime = false;
        glNewList(woDraw, GL_COMPILE);
        if(!adaptive){
            if(filled){
                for (int tt = 0; tt < number_of_patches; tt++){
                   update_frame(point_set[tt], normal_set[tt], true, delta_step);
                }
            } else {
                for (int tt = 0; tt < number_of_patches; tt++){
                    update_frame(point_set[tt], normal_set[tt], false, delta_step);
                }
            }
        } else {
            if(filled){
                for (int tt = 0; tt < number_of_patches; tt++){
                    subdivide_adptive(patch_list[tt].storage, adaptive_threshold, true);
                }
            } else {
                for (int tt = 0; tt < number_of_patches; tt++){
                    subdivide_adptive(patch_list[tt].storage, adaptive_threshold, false);
                }
            }
        }
        glEndList();
        // cout << "compilation complete" << endl;
    } 
        glCallList(woDraw);

    glFlush();
    glutSwapBuffers();
}

void keyboard_event_handler(unsigned char key, int x, int y){
    switch(key)
    {
        case 'w': 
            filled = !filled;
            firstTime = !firstTime;
            myDraw();
            break;

        case 's':
            smooth = !smooth;
            firstTime = !firstTime;
            myDraw();
            break;

        case '+':
            fov -= zoomFactor;
            myDraw();
            break;
        case '-':
            if (fov > 0){
                fov += zoomFactor;
                myDraw();
                break;
            }
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
                l_r_rotate = 0.0f;
                u_d_rotate = 1.0f;
                myDraw();
                // default_orbit -=orbitDegrees;
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
                l_r_rotate = 0.0f;
                u_d_rotate = 1.0f;
                myDraw();
                // default_orbit +=orbitDegrees;
                break;
            }
        case GLUT_KEY_LEFT:
            if(is_shift_pressed){
                lookfrmx -= translateFactor;
                lookatx -= translateFactor;
                myDraw();
                break;
            } else {
                default_orbit += orbitDegrees;
                l_r_rotate = 1.0f;
                u_d_rotate = 0.0f;
                myDraw();
                // default_orbit -=orbitDegrees;
                break;
            }
        case GLUT_KEY_RIGHT:
            if(is_shift_pressed) {
                lookfrmx += translateFactor;
                lookatx += translateFactor;
                myDraw();
                break;
            } else {    
                default_orbit -= orbitDegrees;
                l_r_rotate = 1.0f;
                u_d_rotate = 0.0f;
                myDraw();
                // default_orbit +=orbitDegrees;
                break;
            }
    }
}

int main(int iArgc, char** cppArgv) {
    loadPatch("teapot.bez.txt");

    glutInit(&iArgc, cppArgv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(200, 200);
    glutCreateWindow("Bezier Surface");
    glutKeyboardFunc(keyboard_event_handler);
    glutSpecialFunc(keyboard_special_handler);

    Initialize();

    glutDisplayFunc(myDraw);


    glutMainLoop();
    return 0;
}









