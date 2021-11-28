#include "CSCIx229.h"

int axes=0;       //  Display axes
int th=0;         //  Azimuth of view angle
int mode=0;
int move=1;       //  Move light
int ph=15;        //  Elevation of view angle
int fov=0;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=6.0;   //  Size of world
double Ox=0,Oy=0,Oz=0; //  LookAt Location
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
unsigned int texture[4]; // Texture names

// tracks the postion of the camera
double fpx = 0;
double fpy = 0;
double fpz = 0;

//tracks the direction the camera is looking
double Cx = 0;
double Cz = 0;

const char* text[] = {"???"};

/*
 *  Set projection
 */
 static void Project()
 {
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();

   if(fov == 0)
     glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   else
     gluPerspective(fov,asp,dim/16,16*dim);

   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
 }
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


void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   glEnable(GL_DEPTH_TEST);
   //  Set perspective
   glLoadIdentity();
   if(fov==0){
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }
   else {
      //  Eye position
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);

      gluLookAt(Ex,Ey,Ez , Ox,Oy,Oz , 0,1,0);
   }


   ground(0,-2,0 , 5,1,5 , 0);
   //  Draw axes - no lighting from here on
   glColor3f(1,1,1);
   if (axes)
   {
      const double len=1.5;
      glBegin(GL_LINES);
      glVertex3d(Ox,Oy,Oz);
      glVertex3d(Ox+len,Oy,Oz);
      glVertex3d(Ox,Oy,Oz);
      glVertex3d(Ox,Oy+len,Oz);
      glVertex3d(Ox,Oy,Oz);
      glVertex3d(Ox,Oy,Oz+len);
      glEnd();
      //  Label axes
      glRasterPos3d(Ox+len,Oy,Oz);
      Print("X");
      glRasterPos3d(Ox,Oy+len,Oz);
      Print("Y");
      glRasterPos3d(Ox,Oy,Oz+len);
      Print("Z");
   }
   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f Fov=%d",th,ph,dim,fov);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

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
  else if (key == GLUT_KEY_DOWN)
     ph -= 5;
  //  PageUp key - increase dim
  else if (key == GLUT_KEY_PAGE_UP && fov<179)
     fov++;
  //  PageDown key - decrease dim
  else if (key == GLUT_KEY_PAGE_DOWN && fov>1)
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
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   else if (ch == '+')
      dim -= 0.1;
   //zooms out
   else if (ch == '-')
      dim += 0.1;
   //  Switch display mode
   else if (ch == 'm')
      fov = 0;
   else if (ch == 'M')
      fov = 60;
   //  Reproject
   Project();
  //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Set projection
   Project();
}


int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering
   // this removes objects that are obstructed by other objects
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(750,750);
   glutCreateWindow("Final Project");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   glClearColor(0.0,0.0,0.0,0); //changes background color
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Load textures

   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
