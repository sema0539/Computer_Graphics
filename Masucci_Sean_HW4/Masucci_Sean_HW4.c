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
int ph=15;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=6.0;   //  Size of world

// tracks the postion of the camera
double fpx = 0;
double fpy = 0;
double fpz = 0;

//tracks the direction the camera is looking
double Cx = 0;
double Cz = 0;

const char* text[] = {"Orthogonal","Perspective","FirstPerson"};

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
 *  Draw vertex in polar coordinates
 */
static void Vertex(double th,double ph, double r, double g, double b)
{
   glColor3f(r+((Cos(th)*Cos(th))/30),g-((Sin(ph)*Sin(ph))/30),b+((Sin(th)*Sin(th))/30));
   glVertex3d(Sin(th)*Cos(ph) , Sin(ph) , Cos(th)*Cos(ph));
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

/*
This is suppose to slightly resemnbele a tree trunk.
I tried using some of the methods shown in example 8 however I was unsuccesful.
So, I had to draw every face of the object which there were 14.
*/
static void tree_trunk(double x,double y,double z, double dx,double dy,double dz, double th) {
  //  Save transformation
  glPushMatrix();
  //  Offset
  glTranslated(x,y,z);
  glRotated(th,0,1,0);
  glScaled(dx,dy,dz);
  //  Top
  glBegin(GL_POLYGON);

  glColor3f(0.5,0.35,0.2);

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

  glVertex3f(0,+1,+1.25*0.75);
  glVertex3f(0.25*.6,+2,+1.25*.6);
  glVertex3f(+1.25*.6,+2,+0.75*.6);
  glVertex3f(+1*0.75,+1,+0.75*0.75);

  glColor3f(0.6,0.35,0.25);

  glVertex3f(+1*0.75,+1,+0.75*0.75);
  glVertex3f(+1.25*.6,+2,+0.75*.6);
  glVertex3f(+1.25*.6,+2,-0.25*.6);
  glVertex3f(+1*0.75,+1,-0.25*0.75);

  glColor3f(0.55,0.35,0.15);

  glVertex3f(+1*0.75,+1,-0.25*0.75);
  glVertex3f(+1.25*.6,+2,-0.25*.6);
  glVertex3f(0.25*.6,+2,-0.75*.6);
  glVertex3f(0,+1,-0.75*0.75);

  glColor3f(0.45,0.35,0.2);

  glVertex3f(0,+1,-0.75*0.75);
  glVertex3f(0.25*.6,+2,-0.75*.6);
  glVertex3f(-0.75*.6,+2,-0.25*.6);
  glVertex3f(-1*0.75,+1,-0.25*0.75);

  glColor3f(0.45,0.35,0.25);

  glVertex3f(-1*0.75,+1,-0.25*0.75);
  glVertex3f(-0.75*.6,+2,-0.25*.6);
  glVertex3f(-0.75*.6,+2,+0.75*.6);
  glVertex3f(-1*0.75,+1,+0.75*0.75);

  glColor3f(0.49,0.35,0.28);

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

  glVertex3f(0,-1,+1);
  glVertex3f(0,+1,+1.25*0.75);
  glVertex3f(+1*0.75,+1,+0.75*0.75);
  glVertex3f(+1,-1,+0.5);

  glColor3f(0.6,0.35,0.25);

  glVertex3f(+1,-1,+0.5);
  glVertex3f(+1*0.75,+1,+0.75*0.75);
  glVertex3f(+1*0.75,+1,-0.25*0.75);
  glVertex3f(+1,-1,-0.5);

  glColor3f(0.55,0.35,0.15);

  glVertex3f(+1,-1,-0.5);
  glVertex3f(+1*0.75,+1,-0.25*0.75);
  glVertex3f(0,+1,-0.75*0.75);
  glVertex3f(0,-1,-1);

  glColor3f(0.45,0.35,0.2);

  glVertex3f(0,-1,-1);
  glVertex3f(0,+1,-0.75*0.75);
  glVertex3f(-1*0.75,+1,-0.25*0.75);
  glVertex3f(-1,-1,-0.5);

  glColor3f(0.45,0.35,0.25);

  glVertex3f(-1,-1,-0.5);
  glVertex3f(-1*0.75,+1,-0.25*0.75);
  glVertex3f(-1*0.75,+1,+0.75*0.75);
  glVertex3f(-1,-1,+0.5);

  glColor3f(0.49,0.35,0.28);

  glVertex3f(-1,-1,+0.5);
  glVertex3f(-1*0.75,+1,+0.75*0.75);
  glVertex3f(0,+1,+1.25*0.75);
  glVertex3f(0,-1,+1);
  glEnd();
  //sides end here

  //  Bottom
  glBegin(GL_POLYGON);

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
   else
   {
      Cx = +2*dim*Sin(th); //Ajust the camera vector based on th
      Cz = -2*dim*Cos(th);
      fpy = +2*dim*Sin(ph); // allows for looking up and down

      gluLookAt(fpx,0,fpz, Cx+fpx,fpy,Cz+fpz, 0,1,0); //  Use gluLookAt, y is the up-axis
   }

   //  Draw Scene
   ground(0,-2,0 , 5,1,5 , 0);
   // Rocks?
   sphere(-0.75,-1,0.75,0.25, 0.42,0.42,0.42);
   sphere(-0.68,-1,0.6,0.15, 0.42,0.42,0.42);
   sphere(-0.9,-1,0.78,0.12, 0.42,0.42,0.42);
   sphere(-0.9,-1,0.6,0.19, 0.42,0.42,0.42);
   //tree?
   tree_trunk(-1.5,0,0, 0.5,1,0.5, 0);
   sphere(-1.5,2.25,0,1, 0.25,0.35,0.15);
   sphere(-1.5,2,0.75,0.75, 0.25,0.35,0.15);
   sphere(-1.3,1.8,-0.75,0.6, 0.25,0.35,0.15);
   sphere(-1,1.8,-0.5,0.65, 0.25,0.35,0.15);
   sphere(-1,1.8,-0.35,0.5, 0.25,0.35,0.15);
   //  Draw instance 2
   //the tree trunk is smaller overall and rotated 19 degrees
   // Rocks?
   sphere(2.25,-1,0.75,0.25, 0.42,0.42,0.42);
   sphere(2.32,-1,0.6,0.15, 0.42,0.42,0.42);
   sphere(2.1,-1,0.78,0.12, 0.42,0.42,0.42);
   sphere(2.1,-1,0.6,0.19, 0.42,0.42,0.42);
   //tree?
   tree_trunk(+1.5,-0.4,0, 0.35,0.6,0.25, 19);
   sphere(+1.5,2.25-0.6,0,1, 0.25,0.35-0.6,0.15);
   sphere(+1.5,2-0.6,0.75,0.75, 0.25,0.35-0.6,0.15);
   sphere(0.2+1.5,1.8-0.6,-0.75,0.6, 0.25,0.35-0.6,0.15);
   sphere(0.5+1.5,1.8-0.6,-0.5,0.65, 0.25,0.35-0.6,0.15);
   sphere(0.5+1.5,1.8-0.6,-0.35,0.5, 0.25,0.35-0.6,0.15);
   //tree 3
   tree_trunk(+3,+0.1,+1.25, 0.2,1.1,0.35, 30);
   sphere(+3,2.25+0.1,0+1.25,1, 0.1,0.6,0.42);
   sphere(+3,2+0.1,0.75+1.25,0.75, 0.1,0.6,0.42);
   sphere(0.2+3,1.8+0.1,-0.75+1.25,0.6, 0.1,0.6,0.42);
   sphere(0.5+3,1.8+0.1,-0.5+1.25,0.65, 0.1,0.6,0.42);
   sphere(0.5+3,1.8+0.1,-0.35+1.25,0.5, 0.1,0.6,0.42);
   //tree 4
   tree_trunk(0,-0.5,-2.25, 0.35,0.5,0.35, 5);
   sphere(0,2.25-1,0-2.25,1, 0.6,0.25,0.42);
   sphere(0,2-1,0.75-2.25,0.75, 0.6,0.25,0.42);
   sphere(0.2,1.8-1,-0.75-2.25,0.6, 0.6,0.25,0.42);
   sphere(0.5,1.8-1,-0.5-2.25,0.65, 0.6,0.25,0.42);
   sphere(0.5,1.8-1,-0.35-2.25,0.5, 0.6,0.25,0.42);

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
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

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
   else if (key == GLUT_KEY_PAGE_UP)
      fov++;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_DOWN && dim>1)
      fov--;
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
         mode = (mode+1)%3;
      //  Keep angles to +/-360 degrees
   }
   //  Switch display mode
   else if (ch == 'm' || ch == 'M')
      mode = (mode+1)%3;
   //  Reproject
   Project();
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
   glutCreateWindow("HW4: Sean Masucci");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   glClearColor(0.1,0.4,0.6,0); //changes background color
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
