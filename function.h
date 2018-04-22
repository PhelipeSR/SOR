#ifndef FUNCTION_H
#define FUNCTION_H

// *******************************************************************************************
// ******INCLUDES*****************************************************************************
// *******************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include <omp.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

// *******************************************************************************************
// ******DEFINES******************************************************************************
// *******************************************************************************************

#define TRUE (0==0)
#define FALSE (0==1)
#define MAX_BYTE 255
#define MAX_DEPTH 3
#define N_SPHERES 2
#define N_LIGHTS 2
#define AREA 500
#define SPHERE_RAY 10
#define AMBIENT 0.35f
#define LIGHT_INT 3.5f
#define TEXTURE_SCALE 0.5f
#define WID 1280
#define HEI 800
#define PI ((float) (355/113))
#define FOV	((float) (PI/4))
#define HALF_FOV (FOV * 0.5)
#define NRAN 1024
#define MASK (NRAN - 1)
#define RAY_MAG 1000

// *******************************************************************************************
// ******STRUCTS******************************************************************************
// *******************************************************************************************

struct c_color {
    float r;
    float g;
    float b;
    float alpha;
}typedef color;

struct p_point {
    float x;
    float y;
    float z;
}typedef point;

struct r_ray {
    point o;
    point d;
}typedef ray;

struct v_viewport {
    int width;
    int height;
}typedef viewport;

struct c_camera {
    point eye;
    point lookat;
    point up,u,v,w;
    float distance;
    viewport view;
}typedef camera;

struct s_sphere {
    point center;
    point maxBB;
    point minBB;
    color c;
    float cr;
    float kd; //[0,1]
    float kf; //[0,1]
    float r;

    int e;
    float ks;
}typedef sphere;

struct o_open_cylinder {
    float bottom;
    float top;
    float r;

    point maxBB;
    point minBB;
    color c;
    float cr;
    float kd; //[0,1]
    float kf; //[0,1]

    int e;
    float ks;
}typedef open_cylinder;

struct r_rectangle {
    point p0;
    point a;
    point b;

    color c;
    float cr;
    float kd; //[0,1]
    float kf; //[0,1]

    int e;
    float ks;
}typedef rectangle;

struct p_plane {
    point normal;
    color c;
    float d;

    float cr;
    float kd; //[0,1]
    float kf; //[0,1]
    int e;
    float ks;

    point m_uAxis;
    point m_vAxis;
}typedef plane;

struct p_light {
    float ls; //light intensity
    point l; //light coordinates
    color c; //light color
}typedef ponctual_light;

enum object {
    NONE,
    SPHERE,
    PLANE,
    OPEN_CYLINDER,
    RECTANGLE,
    CONE,
    BOX
};

typedef unsigned char uchar;

// *******************************************************************************************
// ******FUNCTIONS****************************************************************************
// *******************************************************************************************

ray get_primary_ray(camera *cam, int x, int y, int sample);
point get_sample_pos(int x, int y, int sample);
point jitter(int x, int y, int s);
void generateScene();

void addVec(point *a, point *b);
void subVec(point *a, point *b);
void scalarMulVec(point *a,float sc);

//int inside_grid(grid *gr, point * p);
void initImage(camera *c,uchar * frame);
//void setupGrid(grid *g,point max, point min, int nx, int ny, int nz);
ray * generatePrimaryRays(camera *c);
float dist(point *o , point *d);
void setupCamera(camera *c);
void initCamera(camera *c, point eye, point lookat, int width, int height);
void printPrimaryRays(ray *r, int size);
void normalize(point *a);
void crossProduct(point *a, point *b, point *r);
float dotProduct(point *a, point *b);
float distancia(point a, point b);
float clamp(float x, float min, float max);
float m_max(float a,float b);
float m_min(float a,float b);
//void intersectBoundingBox(grid * gr, ray * raio, point *t_min, point *t_max, float *t0, float *t1);
int save_bmp(char * file_name, camera *c, uchar *frame);

int intersectSphere(sphere * s, ray * r, float *t);
int intersectPlane(plane *p, ray *r, float *t);
int intersectOpenCylinder(open_cylinder *cylinder, ray *r, float *t_res);
int intersectRectangle(rectangle *rec, ray *r, float *t);

enum object intersect3Dscene(int *idx,ray *r, float *t);

void gamut(float *r, float *g, float *b);
float checkerTexture(float u, float v);

//generate random spheres into the global 'data' array and lights into 'lights' array
void generateRandomSpheres();
void generateRandomLightSources();

//conversion from Float to Uchar
uchar floatToIntColor(float c);

//actual ray tracing CORE functions

color trace(camera cam, ray *raio, int iter);
color shade(camera cam, point *incid , enum object obj, int index, point *p, int iter);


#endif