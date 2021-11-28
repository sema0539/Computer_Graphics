/*
 *  Projections
 *
 *  Draw 27 cubes to demonstrate orthogonal & prespective projections
 *
 *  Key bindings:
 *  m          Toggle between perspective and orthogonal
 *  +/-        Changes field of view for perspective
 *  a          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif
//  Default resolution
//  For Retina displays compile with -DRES=2
#ifndef RES
#define RES 1
#endif

int axes=0;       //  Display axes
int th=0;         //  Azimuth of view angle
int mode=0;
int move=1;       //  Move light
int ph=15;        //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=6.0;   //  Size of world
// Light values from ex13
int light     =   1;  // Lighting
int one       =   1;  // Unit value
int distance  =   5;  // Light distance
int inc       =  10;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  10;  // Ambient intensity (%)
int diffuse   =  50;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light

// tracks the postion of the camera
double fpx = 0;
double fpy = 0;
double fpz = 0;

//tracks the direction the camera is looking
double Cx = 0;
double Cz = 0;

//double norm[3];

const char* text[] = {"Orthogonal","Perspective","FirstPerson","Debugging"};

//  Macro for sin & cos in degrees
#define Cos(th) cos(3.14159265/180*(th))
#define Sin(th) sin(3.14159265/180*(th))

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

/*
 *  Check for OpenGL errors
 */
void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

/*
 *  Print message to stderr and exit
 */
void Fatal(const char* format , ...)
{
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}

/*
 *  Set projection
 */
static void Project()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();

   if(mode == 0)
     glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   else
     gluPerspective(fov,asp,dim/4,4*dim);

   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 */
 // from ex 8 however I changed the colors
static void ground(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   glBegin(GL_QUADS);
   //  Top
   glColor3f(0.25,0.35,0.15);
   glNormal3f( 0,+1, 0);
   glVertex3f(-1,+1,+1);
   glVertex3f(+1,+1,+1);
   glVertex3f(+1,+1,-1);
   glVertex3f(-1,+1,-1);
   //  End
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th,double ph, double r, double g, double b)
{
   if(r==1 && g==1 && b==1){
     glColor3f(1,1,1);
   }
   else{
     glColor3f(r+((Cos(th)*Cos(th))/30),g-((Sin(ph)*Sin(ph))/30),b+((Sin(th)*Sin(th))/30));
   }
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);

   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

// added to ex 8 code so that the color of the color of the sphere can change for each instance
static void sphere(double x,double y,double z,double r, double r_rgb, double g, double b)
{
   const int d=15;

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(r,r,r);
   //  White ball with yellow specular
   float yellow[]   = {1.0,1.0,0.0,1.0};
   float Emission[] = {0.0,0.0,0.01*emission,1.0};

   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);

   //  Latitude bands
   for (int ph=-90;ph<90;ph+=d)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=d)
      {
         Vertex(th,ph,r_rgb,g,b);
         Vertex(th,ph+d,r_rgb,g,b);
      }
      glEnd();
   }

   //  Undo transformations
   glPopMatrix();
}

//stuct for the vertices
typedef struct{float x,y,z;}vtx;

//helper function for getting surface normal of a polygon bascially from ex13
static void get_surface_normal(vtx p1, vtx p2, vtx p3){
   //  Planar vector 0
   float dx0 = p1.x-p2.x;
   float dy0 = p1.y-p2.y;
   float dz0 = p1.z-p2.z;
   //  Planar vector 1
   float dx1 = p3.x-p1.x;
   float dy1 = p3.y-p1.y;
   float dz1 = p3.z-p1.z;
   //  Normal
   float Nx = dy0*dz1 - dy1*dz0;
   float Ny = dz0*dx1 - dz1*dx0;
   float Nz = dx0*dy1 - dx1*dy0;

   glNormal3f(Nx,Ny,Nz);
}

//all the vertices needed for getting surface normal of each face of a tree_trunk
const vtx vert[] = {
  {0,+1,+1.25*0.75},
  {0.25*.6,+2,+1.25*.6},
  {+1.25*.6,+2,+0.75*.6},
  {+1*0.75,+1,+0.75*0.75},
  {+1.25*.6,+2,+0.75*.6},
  {+1.25*.6,+2,-0.25*.6},
  {+1*0.75,+1,-0.25*0.75},
  {+1.25*.6,+2,-0.25*.6},
  {0.25*.6,+2,-0.75*.6},
  {0,+1,-0.75*0.75},
  {0.25*.6,+2,-0.75*.6},
  {-0.75*.6,+2,-0.25*.6},
  {-1*0.75,+1,-0.25*0.75},
  {-0.75*.6,+2,-0.25*.6},
  {-0.75*.6,+2,+0.75*.6},
  {-1*0.75,+1,+0.75*0.75},
  {-0.75*.6,+2,+0.75*.6},
  {0.25*.6,+2,+1.25*.6},
  {0,-1,+1},
  {0,+1,+1.25*0.75},
  {+1*0.75,+1,+0.75*0.75},
  {+1,-1,+0.5},
  {+1*0.75,+1,+0.75*0.75},
  {+1*0.75,+1,-0.25*0.75},
  {+1,-1,-0.5},
  {+1*0.75,+1,-0.25*0.75},
  {0,+1,-0.75*0.75},
  {0,-1,-1},
  {0,+1,-0.75*0.75},
  {-1*0.75,+1,-0.25*0.75},
  {-1,-1,-0.5},
  {-1*0.75,+1,-0.25*0.75},
  {-1*0.75,+1,+0.75*0.75},
  {-1,-1,+0.5},
  {-1*0.75,+1,+0.75*0.75},
  {0,+1,+1.25*0.75},
};
/*
This is suppose to slightly resemnbele a tree trunk.
*/
static void tree_trunk(double x,double y,double z, double dx,double dy,double dz, double th) {

  //  Set specular color to white
  float white[] = {1,1,1,1};
  float black[] = {0,0,0,1};
  glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
  glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
  //  Save transformation
  glPushMatrix();
  //  Offset
  glTranslated(x,y,z);
  glRotated(th,0,1,0);
  glScaled(dx,dy,dz);
  //  Top
  glBegin(GL_POLYGON);

  glColor3f(0.5,0.35,0.2);
  glNormal3f( 0,+1, 0);
  glVertex3f(0.25*.6,+2,+1.25*.6);
  glVertex3f(+1.25*.6,+2,+0.75*.6);
  glVertex3f(+1.25*.6,+2,-0.25*.6);
  glVertex3f(0.25*.6,+2,-0.75*.6);
  glVertex3f(-0.75*.6,+2,-0.25*.6);
  glVertex3f(-0.75*.6,+2,+0.75*.6);
  glEnd();

  // Sides starts here
  glBegin(GL_QUADS);

  glColor3f(0.5,0.35,0.3);
  get_surface_normal(vert[0],vert[1],vert[2]);
  glVertex3f(0,+1,+1.25*0.75);
  glVertex3f(0.25*.6,+2,+1.25*.6);
  glVertex3f(+1.25*.6,+2,+0.75*.6);
  glVertex3f(+1*0.75,+1,+0.75*0.75);

  glColor3f(0.6,0.35,0.25);
  get_surface_normal(vert[3],vert[4],vert[5]);
  glVertex3f(+1*0.75,+1,+0.75*0.75);
  glVertex3f(+1.25*.6,+2,+0.75*.6);
  glVertex3f(+1.25*.6,+2,-0.25*.6);
  glVertex3f(+1*0.75,+1,-0.25*0.75);

  glColor3f(0.55,0.35,0.15);
  get_surface_normal(vert[6],vert[7],vert[8]);
  glVertex3f(+1*0.75,+1,-0.25*0.75);
  glVertex3f(+1.25*.6,+2,-0.25*.6);
  glVertex3f(0.25*.6,+2,-0.75*.6);
  glVertex3f(0,+1,-0.75*0.75);

  glColor3f(0.45,0.35,0.2);
  get_surface_normal(vert[9],vert[10],vert[11]);
  glVertex3f(0,+1,-0.75*0.75);
  glVertex3f(0.25*.6,+2,-0.75*.6);
  glVertex3f(-0.75*.6,+2,-0.25*.6);
  glVertex3f(-1*0.75,+1,-0.25*0.75);

  glColor3f(0.45,0.35,0.25);
  get_surface_normal(vert[12],vert[13],vert[14]);
  glVertex3f(-1*0.75,+1,-0.25*0.75);
  glVertex3f(-0.75*.6,+2,-0.25*.6);
  glVertex3f(-0.75*.6,+2,+0.75*.6);
  glVertex3f(-1*0.75,+1,+0.75*0.75);

  glColor3f(0.49,0.35,0.28);
  get_surface_normal(vert[15],vert[16],vert[17]);
  glVertex3f(-1*0.75,+1,+0.75*0.75);
  glVertex3f(-0.75*.6,+2,+0.75*.6);
  glVertex3f(0.25*.6,+2,+1.25*.6);
  glVertex3f(0,+1,+1.25*0.75);
  glEnd();

  /*  Reference ponts for where intially made so that i could get a slight bend */
  //break point

  glBegin(GL_POLYGON);

  glVertex3f(0,+1,+1.25*0.75);
  glVertex3f(+1*0.75,+1,+0.75*0.75);
  glVertex3f(+1*0.75,+1,-0.25*0.75);
  glVertex3f(0,+1,-0.75*0.75);
  glVertex3f(-1*0.75,+1,-0.25*0.75);
  glVertex3f(-1*0.75,+1,+0.75*0.75);

  glEnd();

  // Sides of the lower part of tree
  glBegin(GL_QUADS);

  glColor3f(0.5,0.35,0.3);
  get_surface_normal(vert[18],vert[19],vert[20]);
  glVertex3f(0,-1,+1);
  glVertex3f(0,+1,+1.25*0.75);
  glVertex3f(+1*0.75,+1,+0.75*0.75);
  glVertex3f(+1,-1,+0.5);

  glColor3f(0.6,0.35,0.25);
  get_surface_normal(vert[21],vert[22],vert[23]);
  glVertex3f(+1,-1,+0.5);
  glVertex3f(+1*0.75,+1,+0.75*0.75);
  glVertex3f(+1*0.75,+1,-0.25*0.75);
  glVertex3f(+1,-1,-0.5);

  glColor3f(0.55,0.35,0.15);
  get_surface_normal(vert[24],vert[25],vert[26]);
  glVertex3f(+1,-1,-0.5);
  glVertex3f(+1*0.75,+1,-0.25*0.75);
  glVertex3f(0,+1,-0.75*0.75);
  glVertex3f(0,-1,-1);

  glColor3f(0.45,0.35,0.2);
  get_surface_normal(vert[27],vert[28],vert[29]);
  glVertex3f(0,-1,-1);
  glVertex3f(0,+1,-0.75*0.75);
  glVertex3f(-1*0.75,+1,-0.25*0.75);
  glVertex3f(-1,-1,-0.5);

  glColor3f(0.45,0.35,0.25);
  get_surface_normal(vert[30],vert[31],vert[32]);
  glVertex3f(-1,-1,-0.5);
  glVertex3f(-1*0.75,+1,-0.25*0.75);
  glVertex3f(-1*0.75,+1,+0.75*0.75);
  glVertex3f(-1,-1,+0.5);

  glColor3f(0.49,0.35,0.28);
  get_surface_normal(vert[33],vert[34],vert[35]);
  glVertex3f(-1,-1,+0.5);
  glVertex3f(-1*0.75,+1,+0.75*0.75);
  glVertex3f(0,+1,+1.25*0.75);
  glVertex3f(0,-1,+1);
  glEnd();
  //sides end here

  //  Bottom
  glBegin(GL_POLYGON);
  glNormal3f( 0,-one, 0);
  glVertex3f(0,-1,+1);
  glVertex3f(+1,-1,+0.5);
  glVertex3f(+1,-1,-0.5);
  glVertex3f(0,-1,-1);
  glVertex3f(-1,-1,-0.5);
  glVertex3f(-1,-1,+0.5);
  //  End
  glEnd();

  //  Undo transofrmations
  glPopMatrix();
}

//looks best for dy in range of [0.6, 1.4]
static void make_tree(double x,double y,double z, double dx,double dy,double dz, double th, double r_rgb, double g, double b)
{
  if(dy>1){
     tree_trunk(x,dy-1,z, dx,dy,dz, th);
     sphere(x,2.25+(dy-1),0+z, 1, r_rgb,g,b);
     sphere(x,2+(dy-1),0.75+z,0.75, r_rgb,g,b);
     sphere(x+0.2,1.8+(dy-1),-0.75+z, 0.6, r_rgb,g,b);
     sphere(x+0.5,1.8+(dy-1),-0.5+z, 0.65, r_rgb,g,b);
     sphere(x+0.5,1.8+(dy-1),-0.35+z, 0.5, r_rgb,g,b);
  }
  else if(dy == 1){
    tree_trunk(x,0,z, dx,dy,dz, th);
    sphere(x,2.25,0+z, 1, r_rgb,g,b);
    sphere(x,2,0.75+z,0.75, r_rgb,g,b);
    sphere(x+0.2,1.8,-0.75+z, 0.6, r_rgb,g,b);
    sphere(x+0.5,1.8,-0.5+z, 0.65, r_rgb,g,b);
    sphere(x+0.5,1.8,-0.35+z, 0.5, r_rgb,g,b);
  }
  else {
     tree_trunk(x,0+(dy-1),z, dx,dy,dz, th);
     sphere(x,2.25-dy,0+z,1, r_rgb,g,b);
     sphere(x,2-dy,0.75+z,0.75, r_rgb,g,b);
     sphere(x+0.2,1.8-dy,-0.75+z,0.6, r_rgb,g,b);
     sphere(x+0.5,1.8-dy,-0.5+z,0.65, r_rgb,g,b);
     sphere(x+0.5,1.8-dy,-0.35+z,0.5, r_rgb,g,b);
  }

}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   // check for obstructed objects
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   if (mode == 1)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      fpx=Ex; // sets postion of FirstPerson camera
      fpz=Ez;
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //  Orthogonal - set world orientation
   else if(mode == 0)
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }
   // First Person
   else if(mode == 2)
   {
      Cx = +2*dim*Sin(th); //Ajust the camera vector based on th
      Cz = -2*dim*Cos(th);
      fpy = +2*dim*Sin(ph); // allows for looking up and down

      gluLookAt(fpx,0,fpz, Cx+fpx,fpy,Cz+fpz, 0,1,0); //  Use gluLookAt, y is the up-axis
   }
   else
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);

      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }

   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

   //  Light switch from ex13
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light position
      float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1,1,1);
      sphere(Position[0],Position[1],Position[2] , 0.1, 1,1,1);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
      glDisable(GL_LIGHTING);
   //draws Scene
   if(mode != 3){
     //  Draw Scene
     ground(0,-2,0 , 5,1,5 , 0);
     // Rocks?
     sphere(-0.75,-1,0.75,0.25, 0.42,0.42,0.42);
     sphere(-0.68,-1,0.6,0.15, 0.42,0.42,0.42);
     sphere(-0.9,-1,0.78,0.12, 0.42,0.42,0.42);
     sphere(-0.9,-1,0.6,0.19, 0.42,0.42,0.42);
     //tree 1
     make_tree(-1.5,0,0, 0.5,1,0.5, 0, 0.25,0.35,0.15);
     // Rocks?
     sphere(2.25,-1,0.75,0.25, 0.42,0.42,0.42);
     sphere(2.32,-1,0.6,0.15, 0.42,0.42,0.42);
     sphere(2.1,-1,0.78,0.12, 0.42,0.42,0.42);
     sphere(2.1,-1,0.6,0.19, 0.42,0.42,0.42);
     //tree 2
     make_tree(+1.5,-0.4,0, 0.35,0.6,0.25, 19, 0.25,0.35-0.6,0.15);
     //tree 3
     make_tree(+3,+0.1,+1.25, 0.2,1.1,0.35, 30, 0.1,0.6,0.42);
     //tree 4
     make_tree(0,-0.4,-2.25, 0.35,0.6,0.35, 5, 0.52,0.25,0.42);
   }
   else //for Debugging
   {
     tree_trunk(0,0,0, 0.5,0.5,0.5, 0);
   }

   glDisable(GL_LIGHTING);
   //  Draw axes
   glColor3f(1,1,1);
   if (axes)
   {
      const double len=1.5;  //  Length of axes
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,len);
      glEnd();
      //  Label axes
      glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,len);
      Print("Z");
   }
   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f Fov=%d Mode=%s",th,ph,dim,fov,text[mode]);
   if (light) //from ex13
   {
      glWindowPos2i(5,45);
      Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
      glWindowPos2i(5,25);
      Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shiny);
   }
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

//I reused some keys from ex13 and used new keys to better fit what I already had

/*
 *  GLUT calls this routine when an arrow key is pressed
 */

void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP && ph<90)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN && ph>0)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_UP && fov<179)
      fov++;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_DOWN && fov>1)
      fov--;
   //  Smooth color model
   else if (key == GLUT_KEY_F1)
       smooth = 1-smooth;
   //  Local Viewer
   else if (key == GLUT_KEY_F2)
       local = 1-local;
   else if (key == GLUT_KEY_F3)
       distance = (distance==1) ? 5 : 1;
   //  Toggle ball increment
   else if (key == GLUT_KEY_F8)
       inc = (inc==10)?3:10;
   //  Flip sign
   else if (key == GLUT_KEY_F9)
       one = -one;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle axes
   else if (ch == 'x' || ch == 'X')
      axes = 1-axes;
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //  Move light
   else if (ch == '<')
      zh += 1;
   else if (ch == '>')
      zh -= 1;
   // zooms in
   else if (ch == '+')
      dim -= 0.1;
   //zooms out
   else if (ch == '-')
      dim += 0.1;
   // first person controls
   if (mode == 2) {
      double dt = 0.05;
      //move camera forward
      if (ch == 'w' || ch == 'W'){
         fpx += Cx*dt;
         fpz += Cz*dt;
      }
      //move camera backward
      else if (ch == 's' || ch == 'S'){
         fpx -= Cx*dt;
         fpz -= Cz*dt;
      }
      //  Switch display mode
      else if (ch == 'm' || ch == 'M')
         mode = (mode+1)%4;
      //  Keep angles to +/-360 degrees
   }
   //  Switch display mode
   else if (ch == 'm' || ch == 'M')
      mode = (mode+1)%4;
   //  Light elevation
   else if (ch=='[')
      ylight -= 0.1;
   else if (ch==']')
       ylight += 0.1;
   //  Ambient level
   else if (ch=='a' && ambient>0)
       ambient -= 5;
   else if (ch=='A' && ambient<100)
       ambient += 5;
   //  Diffuse level
   else if (ch=='d' && diffuse>0)
       diffuse -= 5;
   else if (ch=='D' && diffuse<100)
       diffuse += 5;
   //  Specular level
   else if (ch=='s' && specular>0)
     specular -= 5;
   else if (ch=='S' && specular<100)
     specular += 5;
   //  Emission level
   else if (ch=='e' && emission>0)
     emission -= 5;
   else if (ch=='E' && emission<100)
     emission += 5;
   //  Shininess level
   else if (ch=='n' && shininess>-1)
     shininess -= 1;
   else if (ch=='N' && shininess<7)
     shininess += 1;
   //  Toggle light movement
   else if (ch == 'y' || ch == 'Y')
     move = 1-move;
   //  Translate shininess power to value (-1 => 0)
   shiny = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project();
   //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Set projection
   Project();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering
   // this removes objects that are obstructed by other objects
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(750,750);
   glutCreateWindow("HW5: Sean Masucci");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   //glClearColor(0.1,0.4,0.6,0); //changes background color
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
