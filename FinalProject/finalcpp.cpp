#include "CSCIx229.h"

#include <iostream>
#include <string>
#include <random>
#include "noiseClass.h"


int axes=1;       //  Display axes
int th=0;         //  Azimuth of view angle
int mode=0;
int wireframe = 0;
int move=1;       //  Move light
int ph=15;        //  Elevation of view angle
int fov=60;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=6.0;   //  Size of world
double Ox=0,Oy=0,Oz=0; //  LookAt Location
// Light values from ex13
int light     =   0;  // Lighting
int one       =   1;  // Unit value
int distance  =  20;  // Light distance
int inc       =   1;  // Ball increment
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

int row = 32;
int col = 32;

const char* text[] = {"???"};

struct hmapStruct{float hmap[32][32];} t;

typedef struct{float x,y,z;}vtx;

noiseClass perlin;

std::default_random_engine generator;

typedef struct{
  int x;
  int y;
}Point;

Point randomPoint(Point min, Point max)
{
    static std::mt19937 gen;
    std::uniform_int_distribution<> distribX(min.x, max.x);
    std::uniform_int_distribution<> distribY(min.y, max.y);

    return Point{distribX(gen), distribY(gen)};
}

float map(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
   gluPerspective(fov,asp,dim/16,16*dim);

 //  Switch to manipulating the model matrix
 glMatrixMode(GL_MODELVIEW);
 //  Undo previous transformations
 glLoadIdentity();
}

vtx vert[3];

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

static void terrain(double x,double y,double z,
                  double dx,double dy,double dz,
                  double th)
{
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
   glRotated(th,1,0,0);
   glScaled(dx,dy,dz);

   for(int i=0; i<row;i++){
     glBegin(GL_TRIANGLE_STRIP);
     glColor3f(0.45,0.35,0.25);
     for(int j=0; j<col; j++){
       // !! work on Lighting !!
       //get_surface_normal(vert[0],vert[1],vert[2]);
       // saves the vertices of the terrain to calulate the norams
       vert[0].x = j;
       vert[0].y = i;
       vert[0].z = t.hmap[j][i];
       vert[1].x = j;
       vert[1].y = i+1;
       vert[1].z = t.hmap[j][i+1];
       vert[2].x = j+1;
       vert[2].y = i;
       vert[2].z = t.hmap[j+1][i];
       //sets the vertices of the terrain
       get_surface_normal(vert[0],vert[1],vert[2]);
       glVertex3d(j,i,t.hmap[j][i]);
       glVertex3d(j,i+1,t.hmap[j][i+1]);

     }
     glEnd();
   }
   //  End

   //  Undo transofrmations
   glPopMatrix();
}

/*
Draw vertex in polar coordinates with normal
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
   glRotated(th,1,0,0);
   glScaled(r,r,r);
   //  White ball with yellow specular
   float yellow[]   = {1.0,1.0,0.0,1.0};
   float Emission[] = {0.0,0.0,0.01*0,1.0};

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

/*
!!Tree branch object!! add textures
*/
static void treeBranch(double x,double y,double z,double rtop, double rbtm, double s, double h)
{
   const int d=15;

   double xN;
   double yN;
   double zN;

   vtx treeBrancht[25];
   vtx treeBranchb[25];

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x,h+y,z);
   glScaled(s,h,s);

   //  South pole cap
   glBegin(GL_TRIANGLE_FAN);
   glColor3f(0.5,0.35,0.15);
   glNormal3d(0,-1,0);
   glVertex3d(0,-1.05,0);
   int i = 0;
   for (int th=0;th<=360;th+=d)
   {
      xN = rbtm*Sin(th)*Cos(d-90);
      yN =         Sin(d-90);
      zN = rbtm*Cos(th)*Cos(d-90);

      glColor3f(0.5,0.35,0.15);
      glNormal3d(xN,yN,zN);
      glVertex3d(xN,yN,zN);

      // stores the vertices of the outside of the bottom cap
      treeBranchb[i].x=xN;
      treeBranchb[i].y=yN;
      treeBranchb[i].z=zN;
      i++;
   }
   glEnd();

   //  North pole cap
   glBegin(GL_TRIANGLE_FAN);
   glColor3f(0.5,0.4,0.2);
   glNormal3d(0,1,0);
   glVertex3d(0,1.1,0);
   int j = 0;
   for (int th=0;th<=360;th+=d)
   {
     xN = rtop*Sin(th)*Cos(90-d);
     yN =         Sin(90-d);
     zN = rtop*Cos(th)*Cos(90-d);

     glColor3f(0.5,0.4,0.2);
     glNormal3d(xN,yN,zN);
     glVertex3d(xN,yN,zN);

     // stores the vertices of the outside of the top cap
     treeBrancht[j].x=xN;
     treeBrancht[j].y=yN;
     treeBrancht[j].z=zN;
     j++;
   }
   glEnd();

   // creates the sides of the brach
   glBegin(GL_TRIANGLE_STRIP);
   for(int h=0;h<25;h++){
     glColor3f(0.5,0.35,0.2);
     // uses the vertices from the outside of the top and bottom to make the triangle strip
     glNormal3d(treeBrancht[h].x,treeBrancht[h].y,treeBrancht[h].z);
     glVertex3d(treeBrancht[h].x,treeBrancht[h].y,treeBrancht[h].z);
     glNormal3d(treeBranchb[h].x,treeBranchb[h].y,treeBranchb[h].z);
     glVertex3d(treeBranchb[h].x,treeBranchb[h].y,treeBranchb[h].z);
   }
   glEnd();
   //  Undo transformations
   glPopMatrix();
}

/*
!!Mushroom object!! add textures
*/
static void mushroom(double x,double y,double z,double r)
{
  const int d=15;

  double xN;
  double yN;
  double zN;

  vtx domeRim[25][2];
  vtx stem[25][2];

  //  Save transformation
  glPushMatrix();
  //  Offset and scale
  glTranslated(x,y,z);
  glScaled(r,r,r);

  //  outer North pole cap
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(0.5, 0.15, 0.23);
  glNormal3d(0,1,0);
  glVertex3d(0,1.5,0);
  for (int th=0;th<=360;th+=d)
  {
    xN = Sin(th)*Cos(90-d);
    yN = 0.5+       Sin(90-d);
    zN = Cos(th)*Cos(90-d);

    glColor3f(0.5, 0.15, 0.23);
    glNormal3d(xN,yN,zN);
    glVertex3d(xN,yN,zN);
  }
  glEnd();

  //  outer Latitude bands basiically makes a dome shape
  for (int ph=90-2*d;ph>=(d-90)/6;ph-=d)
  {
     glBegin(GL_TRIANGLE_STRIP);
     int i = 0;
     glColor3f(0.5, 0.15, 0.23);
     for (int th=0;th<=360;th+=d)
     {
        xN = Sin(th)*Cos(ph);
        yN = 0.5+Sin(ph);
        zN = Cos(th)*Cos(ph);
        glNormal3d(xN,yN,zN);
        glVertex3d(xN,yN,zN);

        xN = Sin(th)*Cos(ph+d);
        yN = 0.5+Sin(ph+d);
        zN = Cos(th)*Cos(ph+d);
        glNormal3d(xN,yN,zN);
        glVertex3d(xN,yN,zN);
        // saves the last vertices of the outer dome
        if(ph == 0){
          domeRim[i][0].x = Sin(th)*Cos(ph);
          domeRim[i][0].y = 0.5+Sin(ph);
          domeRim[i][0].z = Cos(th)*Cos(ph);
          i++;
        }

     }
     glEnd();
  }

  //  botom of the stem
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(0.95,0.92,0.8);
  glNormal3d(0,-1,0);
  glVertex3d(0,0,0);
  int c = 0;
  for (int th=0;th<=360;th+=d)
  {
    xN = 0.85*Sin(th)*Cos(90-d);
    yN = -0.85+0.85*        Sin(90-d);
    zN = 0.85*Cos(th)*Cos(90-d);

    glColor3f(0.95,0.92,0.8);
    glNormal3d(0,-1,0);
    glVertex3d(xN,yN,zN);

    stem[c][1].x = xN;
    stem[c][1].y = yN;
    stem[c][1].z = zN;
    stem[c][0].x = xN;
    stem[c][0].y = 0.5+0.85*Sin(90-d);
    stem[c][0].z = zN;
    c++;
  }
  glEnd();

  //  inner Latitude bands
  for (int ph=90-2*d;ph>=(d-90)/6;ph-=d)
  {
     glBegin(GL_TRIANGLE_STRIP);
     int j = 0;
     glColor3f(0.95,0.92,0.8);
     for (int th=0;th<=360;th+=d)
     {
        xN = 0.85*Sin(th)*Cos(ph);
        yN = 0.5+0.85*Sin(ph);
        zN = 0.85*Cos(th)*Cos(ph);
        glNormal3d(xN,yN,zN);
        glVertex3d(xN,yN,zN);

        xN = 0.85*Sin(th)*Cos(ph+d);
        yN = 0.5+0.85*Sin(ph+d);
        zN = 0.85*Cos(th)*Cos(ph+d);
        glNormal3d(xN,yN,zN);
        glVertex3d(xN,yN,zN);
        // saves the last vertices of the inner dome
        if(ph == 0){
          domeRim[j][1].x = 0.85*Sin(th)*Cos(ph);
          domeRim[j][1].y = 0.5+0.85*Sin(ph);
          domeRim[j][1].z = 0.85*Cos(th)*Cos(ph);
          j++;
        }


     }
     glEnd();
  }

  // bottem rim of dome
  glBegin(GL_TRIANGLE_STRIP);
  int g;
  for(g=0; g<25; g++){
    // uses the inner and outer vertices at the end of the domes to make the rim
    glColor3f(0.95,0.92,0.8);
    glNormal3d(0,-1,0);
    glVertex3d(domeRim[g][0].x,domeRim[g][0].y,domeRim[g][0].z);
    glVertex3d(domeRim[g][1].x,domeRim[g][1].y,domeRim[g][1].z);
  }
  glEnd();

  // sides of stem
  glBegin(GL_TRIANGLE_STRIP);
  for(g=0; g<25; g++){
    glColor3f(0.95,0.92,0.9);
    glNormal3d(stem[g][0].x,stem[g][0].y,stem[g][0].z);
    glVertex3d(stem[g][0].x,stem[g][0].y,stem[g][0].z);
    glNormal3d(stem[g][1].x,stem[g][1].y,stem[g][1].z);
    glVertex3d(stem[g][1].x,stem[g][1].y,stem[g][1].z);

  }
  glEnd();

  //  Undo transformations
  glPopMatrix();
}

/*
!!Generates Trees using Recursion!! still needs work
*/
float angle1 = 315;
float angle2 = 45;

void branch(float len, float stop, float sbtm){
  treeBranch(0,0,0, stop,sbtm, 1,len);
  if(len>0.33){
    glPushMatrix();
    glTranslated(0,len*2,0);
    if(angle1 > 30){
      glRotated(angle1,1,1,0);
      glRotated(angle1,0,1,0);
    } else{
      glRotated(angle1,0,1,1);
      glRotated(angle1,0,1,0);
    }
    branch(len*0.67, stop*0.75, sbtm*0.75);
    if(len*0.67<0.33){
      sphere(0,0,0, 1, 0.05, 0.45, 0.15);
    }
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,len*2,0);
    if(angle2 < -22){
      glRotated(angle2,1,1,1);
      glRotated(angle1,0,1,0);
    } else{
      glRotated(angle2,1,0,1);
      glRotated(angle1,0,1,0);
    }
    branch(len*0.67, stop*0.75, sbtm*0.75);
    if(len*0.67<0.33){
      sphere(0,0,0, 0.5, 0.05, 0.45, 0.15);
    }
    glPopMatrix();
  }

}

float value[2];

float *findMinMax(float *arr){
  float min = 10;
  float max = -10;
  int width = perlin.getOutputWSize();
  int height = perlin.getOutputHSize();
  for (int i = 0; i<width*height; i++)
    {
      if (map(arr[i],0,1,-10,10) < min) {
          min = map(arr[i],0,1,-10,10);
      }

      if (map(arr[i],0,1,-10,10) > max) {
          max = map(arr[i],0,1,-10,10);
      }
    }
    value[0] = min;
    value[1] = max;
    return value;
}

Point treePos[50];

void myInit(){
  Point minT,maxT,outT;
  minT.x = -15;
  minT.y = -15;
  maxT.x = 15;
  maxT.y = 15;
  for(int i = 0;i < 50;i++){
    outT = randomPoint(minT, maxT);
    treePos[i].x = outT.x;
    treePos[i].y = outT.y;
  }
}

void display()
{

  perlin.noise2D(perlin.getOutputWSize(), perlin.getOutputHSize(), perlin.getSeed(), perlin.getOctaves(), perlin.getScalingBias(), perlin.getOutput());
  float *fPerlinNoise2D = perlin.getOutput();
  int h = 0;
  for(int i=0; i<row;i++){
    for(int j=0; j<col; j++){
       // saves the values for the hight of the terrian using Perlin noise
       t.hmap[j][i] = 0;//map(fPerlinNoise2D[h],0,1,-10,10);
       h++;
    }
  }
  //gets the min and max x and z cordents

  // gets the minimum and max height of terrain
  float *minMax = findMinMax(fPerlinNoise2D);
  ///  Erase the window and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  if(wireframe == 0){
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  } else{
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }
  glLoadIdentity();
  //  Set perspective
  if(mode==0){
     glRotatef(ph,1,0,0);
     glRotatef(th,0,1,0);
  }
  else {
     //  Eye position
     double Ex = -2*dim*Sin(th)*Cos(ph);
     double Ey = +2*dim        *Sin(ph);
     double Ez = +2*dim*Cos(th)*Cos(ph);

     gluLookAt(Ex,Ey,Ez , Ox,minMax[1]+Oy,Oz , 0,1,0);
  }
  //  Flat or smooth shading
  glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

  //  Light switch from ex13
  if (light)
  {
     //  Translate intensity to color vectors
     float Ambient[]   = {0.01*10 ,0.01*10 ,0.01*10 ,1.0};
     float Diffuse[]   = {0.01*50 ,0.01*50 ,0.01*50 ,1.0};
     float Specular[]  = {0.01*0,0.01*0,0.01*0,1.0};
     //  Light position
     float Position[]  = {distance*Cos(zh),distance*Sin(zh),1.0,ylight};
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

  terrain((col/2)*-1,0,row/2 , 1,1,1 , 270);
  for(int i = 0;i < 50;i++){
    treeBranch(treePos[i].x,t.hmap[treePos[i].y][treePos[i].x],treePos[i].y, 0.25,0.45, 1,3);
  }
  //std::cout<<t.hmap[0][0]<<std::endl;
  //branch(1.5, 0.25, 0.45);
  //cylinder(0,0,0,  1,1,1,  0.15,3,0,15);
  //treeBranch(0,0,0, 0.25,0.45, 1,3);
  //mushroom(0,0,0,1);

  glDisable(GL_LIGHTING);
  //  Draw axes - no lighting from here on
  glColor3f(1,1,1);
  if (axes)
  {
     const double len=3;
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
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   else if (ch == '+')
       dim -= 0.1;
   //zooms out
   else if (ch == '-')
       dim += 0.1;
   //  Switch display mode
   else if (ch == 'f')
       fov = 0;
   else if (ch == 'F')
       fov = 60;
   else if (ch == 'r'){
     angle1 = 15+(rand()%75);
     angle2 = -(15+(rand()%45));
   }
   else if(ch == 'l' || ch == 'L')
     light = 1-light;
   else if(ch=='m' || ch == 'M')
     mode = (mode+1)%3;
   else if(ch == 'o' ||ch == 'o')
     wireframe = (wireframe+1)%2;
   else if(ch == '1'){
     perlin.changeOctaves();
   }
   else if(ch == '2'){
     perlin.changeScalingBias();
   }
   else if(ch == '3'){
     perlin.resetOctaves();
     perlin.randSeed();
   }

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
   myInit();
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
