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
double asp=1;     //  Aspect ratio
double dim=6.0;   //  Size of world
const char* text[] = {"Scene","My Object"};

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

   glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
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
   //  Orthogonal - set world orientation
   glRotatef(ph,1,0,0);
   glRotatef(th,0,1,0);

   switch (mode)
   {
      //  Draw Scene
      case 0:
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
         break;
      //  show My shape
      case 1:
         tree_trunk(0,0,0, 0.5,1,0.5, 0);
         break;
   }

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
   Print("Angle=%d,%d  Dim=%.1f Projection=%s   %s",th,ph,dim,"Orthogonal",text[mode]);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
 // CHANGE
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
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
 // CHANGE
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle axes
   else if (ch == 'a' || ch == 'A')
      axes = 1-axes;
   // zooms in
   else if (ch == '+')
      dim -= 0.1;
   //zooms out
   else if (ch == '-')
      dim += 0.1;
   //  Switch what can be seen
   else if (ch == 'm'){
      mode = (mode+1)%2;
      dim = 6.0;
   }
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
   glutCreateWindow("Projections");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
