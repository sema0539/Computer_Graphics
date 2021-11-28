 #include <stdio.h>
 #include <stdlib.h>
 #include <stdarg.h>
 #ifdef USEGLEW
 #include <GL/glew.h>
 #endif
 //  OpenGL with prototypes for glext
 #define GL_GLEXT_PROTOTYPES
 #ifdef __APPLE__
 #include <GLUT/glut.h>
 #else
 #include <GL/glut.h>
 #endif
 //  Default resolution
 //  For Retina displays compile with -DRES=2
 #ifndef RES
 #define RES 1
 #endif

//  Globals
int th=0;       // Azimuth of view angle
int ph=0;       // Elevation of view angle
int mode=1;     // track what variable is being changed
double z=0;     // Z variable
double dim=2;   // Dimension of orthogonal box
const char* text[] = {"","sigma","beta","rho"};
//Lorenz Params
double s  = 10;
double b  = 2.6666;
double r  = 28;
// holds all the points for each of the lorenz attractor parameters
double pts[50000][3];

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  // Maximum length of text string
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
 *  Check for OpenGL errors
 */
void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

/*
 *  lorenz function
 */
void lorenz()
{
   int i;
   //Coordinates
   double x = 1;
   double y = 1;
   double z = 1;
   double w = 0.025; //used to scale with view
   // Time Step
   double dt = 0.001;
   /*
    *  Integrate 50,000 steps (50 time units with dt = 0.001)
    *  Explicit Euler integration
    */
   for (i=0;i<50000;i++)
   {
      double dx = s*(y-x);
      double dy = x*(r-z)-y;
      double dz = x*y - b*z;
      x += dt*dx;
      y += dt*dy;
      z += dt*dz;
      // multiply by w after point are calculated to get within window view
      // also stores points in array
      pts[i][0] = x*w;
      pts[i][1] = y*w;
      pts[i][2] = z*w;
   }
}

void key(unsigned char ch,int x,int y)
{
  if (ch == 27){
    exit(0);
  }
  //sets everthing to defult parameters
  else if (ch == '0'){
    th = ph = 0;
    s = 10;
    b = 2.6666;
    r = 28;
  }
  //  Switch what parameter is being changed
  else if ('1'<=ch && ch<='3')
  {
     mode = ch-'0';
     if (mode==1) s = s; // sigma
     if (mode==2) b = b; // beta
     if (mode==3) r = r; // rho
  }
  //increase parameter based on mode
  else if (ch =='+'){
    if(mode == 1) s += 1;
    else if(mode == 2) b += 0.5;
    else r += 1;
  }
  //decrease parameter based on mode
  else if (ch =='-'){
    if(mode == 1) s -= 1;
    else if(mode == 2) b -= 0.5;
    else r -= 1;
  }
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase azimuth by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease azimuth by 5 degrees
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
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Orthogonal projection box adjusted for the
   //  aspect ratio of the window
   double asp = (height>0) ? (double)width/height : 1;
   glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

void display()
{
  //gets point from lorenz function
  lorenz();
  //  Clear the image
  glClear(GL_COLOR_BUFFER_BIT);
  //  Reset previous transforms
  glLoadIdentity();
  //  Set view angle
  glRotated(ph,1,0,0);
  glRotated(th,0,1,0);
  // use line strip so that the last vertex does not connect with the first like loop does
  glBegin(GL_LINE_STRIP);
  //loop to get every vertex
  for (int i=0;i<50000;i++)
  {
    // gives gradiant because each point has a slightly different color
    glColor3dv(pts[i]+35);
    // takes the array and gets the vertex for each point and parameter
    // rather than doing glVertex3f(pts[i][0],pts[i][1],pts[i][2]) which would also work
    glVertex3dv(pts[i]);
  }
  glEnd();
  //  Draw axes in white
  glColor3f(1,1,1);
  glBegin(GL_LINES);
  glVertex3d(0,0,0);
  glVertex3d(1.5,0,0);
  glVertex3d(0,0,0);
  glVertex3d(0,1.5,0);
  glVertex3d(0,0,0);
  glVertex3d(0,0,1.5);
  glEnd();
  //  Label axes
  glRasterPos3d(1.5,0,0);
  Print("X");
  glRasterPos3d(0,1.5,0);
  Print("Y");
  glRasterPos3d(0,0,1.5);
  Print("Z");
  //  Display parameters
  glWindowPos2i(5,5);
  Print("View Angle=%d,%d  sigma=%.1f beta=%.3f rho=%.1f mode=%s",th,ph,s,b,r,text[mode]);
  //  Flush and swap
  ErrCheck("display");
  glFlush();
  glutSwapBuffers();
}

int main(int argc,char* argv[])
{
  //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   //  Request 500 x 500 pixel window
   glutInitWindowSize(750,750);
   //  Create the window
   glutCreateWindow("Coordinates");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   //  Return code
   return 0;
}
