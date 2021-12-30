// Key bindings
//   Esc           Exits the program.
//   z             Toggles axis.
//   c             Increments the octaves by one for the terrain or rocks depending on what object is being viewed.
//   v             Increments the scaling bias which effects how independent each point is.
//   x             Creates a new seed for the terrain or rocks and resets the octaves.
//   r             Changes the trees.
//   f/F           changes the fov.
//   l/L           Toggles the light
//   m/M           Changes between perspective and orthogonal projections
//   i/I           Toggles the wireframe.
//   q/Q           Increases and decreses the fog density
//   0             Shows the main scene.
//   1             Shows the simple tree branch.
//   2             Shows the mushroom.
//   3             Shows just the terrain.
//   4             Shows the rocks.
//   5             Shows the tree.
//   6             Shows the flower
//   7             Shows the Pine Tree
//   +/-           increases/decreases the dim.
//   arrow keys    Control the camera.
//   w/a/s/d       Move camera while in first person
//   page_up       Moves camera height up while in first person
//   page_down     Moves camera height down while in first person

#include "CSCIx229.h"

#include <iostream>
#include <string>
#include <random>
#include "noiseClass.h"
#include "mainScene.h"
#include "FractalTree.h"

int axes=0;       //  Display axes
int th=0;         //  Azimuth of view angle
int mode=1;
int wireframe=0;
int object = 0;   //  display different objects
int move=1;       //  Move light
int ph=15;        //  Elevation of view angle
int fov=60;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=6.0;   //  Size of world
double Ox=0,Oy=0,Oz=0; //  LookAt Location
// Light values from ex13
int light     =   1;  // Lighting
int distance  =  30;  // Light distance
int emission  =   0;  // Emission intensity (%)
int ambient   =  25;  // Ambient intensity (%)
int diffuse   =  75;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
float shiny   =   2;  // Shininess (value)
int zh        =   0;  // Light azimuth
float density = 0.03;
unsigned int texture[8];
// tracks the postion of the camera
double fpx = 0;
double fpy = 0;
double fpz = 0;
//tracks the direction the camera is looking
double Cx = 0;
double Cz = 0;
double Cy = 0;
// size of terrain
int row = 64;
int col = 64;

const char* modeText[] = {"Orthogonal", "Perspective", "FirstPerson"};
const char* objText[] = {"Main Scene", "Tree Branch", "Mushroom", "Terrain", "Rocks", "Tree", "Flower", "Pine Tree"};

//holds the values that are made from the noise function
struct hmapStruct{float hmap[64][64];} t;
struct rHmapStruct{float hmap[25][14];} rockP;

//for generating noise to randomlly make terrian and rocks
noiseClass perlin(row,col);
noiseClass rocks(13,25);
//makes random trees
FractalTree *randTrees;
//get pos of all objects in main scene
mainScene objPos(40,50,75,25,25,0,60);

//holds the seed for each rock
float *rockSeed[50];

//basically from ex8
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

// this is the vertex function just with more customization
static void VertexAdj(double th,double ph, double xPos, double yPos, double zPos, float scale, int flip, int tex,
  double r, double g, double b){
  double x;
  double y;
  double z;
  if(flip==0){
     x = xPos+scale*Sin(th)*Cos(ph);
     y = yPos+scale*Cos(th)*Cos(ph);
     z = zPos+scale*        Sin(ph);

  } else {
     x = xPos+scale*Sin(th)*Cos(ph);
     y = yPos+scale*        Sin(ph);
     z = zPos+scale*Cos(th)*Cos(ph);
  }
  glColor3f(r,g,b);
  glNormal3d(x,y,z);
  // to map texture if needed
  if(tex==1){
    glTexCoord2f(th/360,ph/180);
  } else if(tex==2){ //alternate way to map texture to object
    glTexCoord2f(th/45,ph/15);
  }
  glVertex3d(x,y,z);
}

// helps to map the noise function to numbers that are not between 0 and 1
float map(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// finds the min and max height  of the terrian
float *findMinMax(float *arr){
  float *value;
  value = new float[2];
  float min = 10;
  float max = 0;
  int width = perlin.getOutputWSize();
  int height = perlin.getOutputHSize();
  for (int i = 0; i<width*height; i++)
    {
      if (map(arr[i],0,1,0,10) < min) {
          min = map(arr[i],0,1,0,10);
      }

      if (map(arr[i],0,1,0,10) > max) {
          max = map(arr[i],0,1,0,10);
      }
    }
    value[0] = min;
    value[1] = max;
    return value;
}

// helper function for getting surface normal of a polygon bascially from ex13
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
/*
!!Terrain object!!
*/
static void terrain(double x,double y,double z,
                  double dx,double dy,double dz,
                  double th, double waterLevel)
{
   vtx vert[4];
   //  Set specular color to white
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   //enables textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[2]);

   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,1,0,0);
   glScaled(dx,dy,dz);

   // creates a mesh of polygons and applies different heights
   // to each point based on the noise function
   for(int i=0; i<row;i++){
     glBegin(GL_TRIANGLE_STRIP);
     for(int j=0; j<col-1; j++){
       // saves the vertices of the terrain to calulate the norms
       vert[0].x = j;
       vert[0].y = t.hmap[j][i];
       vert[0].z = i;
       vert[1].x = -j;
       vert[1].y = -t.hmap[j][i+1];
       vert[1].z = -i+1;
       vert[2].x = j+1;
       vert[2].y = t.hmap[j+1][i];
       vert[2].z = i;
       // depending on the water level, the terrain is a different color
       if(t.hmap[j][i] < waterLevel+0.2 && t.hmap[j][i+1]< waterLevel+0.3){
         glColor3f(0.65,0.55,0.5);
       } else {
         glColor3f(0.1, 0.45, 0.15);
       }
       get_surface_normal(vert[0],vert[1],vert[2]);
       glTexCoord2f(i,j); glVertex3d(i,t.hmap[j][i],j);
       glTexCoord2f(i+1,j); glVertex3d(i+1,t.hmap[j][i+1],j);
     }
     glEnd();
   }
   //  End

   //  Undo transofrmations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
/*
!!Water object!!
*/
static void water(double x,double y,double z,
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

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[3]);
    //makes the water transparent
   glEnable(GL_BLEND);
   glColor4f(0.1,0.15,0.45,0.75);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glDepthMask(0);
   // makes a mesh of polygons for the water and is reused from terrain
   for(int i=0; i<row;i++){
     glBegin(GL_TRIANGLE_STRIP);
     for(int j=0; j<col-1; j++){
       //sets the vertices of the terrain
       glNormal3d(0,1,0);
       glTexCoord2f(i,j); glVertex3d(i,0,j);
       glTexCoord2f(i+1,j); glVertex3d(i+1,0,j);

     }
     glEnd();
   }
   //  End
   glDisable(GL_BLEND);
   glDepthMask(1);
   //  Undo transofrmations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
// added to ex 8 code so that the color of the sphere can change for each instance
static void sphere(double x,double y,double z,double r, double r_rgb, double g, double b)
{
   const int d=15;
   float i=1;

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

   if(r!=1 && g!=1 && b!=1){
     glEnable(GL_TEXTURE_2D);
     glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);
     glBindTexture(GL_TEXTURE_2D,texture[1]);
   }

   //  Latitude bands
   for (int ph=-90;ph<90;ph+=d)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=d)
      {
         if(th%2==0 && g!=1){
           //maps texture to first set of vertices
           glTexCoord2f(0,i); Vertex(th,ph,r_rgb,g,b);
           glTexCoord2f(0,0); Vertex(th,ph+d,r_rgb,g,b);
         }
         else if(g!=1){
           //maps texture to second set of vertices
           i-=1/144;
           glTexCoord2f(i,i); Vertex(th,ph,r_rgb,g,b);
           glTexCoord2f(i,0); Vertex(th,ph+d,r_rgb,g,b);
         }
         else {
           Vertex(th,ph,r_rgb,g,b);
           Vertex(th,ph+d,r_rgb,g,b);
         }
      }
      glEnd();
   }

   //  Undo transformations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//This is based on the shere1 func from ex8 but I canged it so that it applies the noise function to each point
//and added the seed variable so that it can gereate differnt seeds for multiple different rocks
/*
!!rock object!!
*/
static void rock(double x,double y,double z,double r, float *seed)
{
   //makes the points to make the sphere look more like a rock and takes in a new seed for each rock
   rocks.noise2D(rocks.getOutputWSize(), rocks.getOutputHSize(), seed, rocks.getOctaves(), rocks.getScalingBias(), rocks.getOutput());
   float *fRockNoise2D = rocks.getOutput();
   int h = 0;
   //sets a height based on the x, y positon.
   for(int i=0; i<13;i++){
     for(int j=0; j<25; j++){
        // saves the values for the hight of the terrian using Perlin noise
        rockP.hmap[j][i] = map(fRockNoise2D[h],0,1,0.5,r);
        h++;
     }
   }
   const int d=15;

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x,y,z);
   glScaled(r,r,r);

   //  White ball with yellow specular
   float yellow[]   = {1.0,1.0,0.0,1.0};
   float Emission[] = {0.0,0.0,0.01*0,1.0};

   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[4]);

   //  South pole cap
   glBegin(GL_TRIANGLE_FAN);
   VertexAdj(0,-90, 0,0,0, rockP.hmap[0][0],0,1 ,0.15,0.15,0.15);
   int i = 0;
   for (int th=0;th<=360;th+=d)
   {
     // the first and last postion need to be the same so that they line up
     if(th == 360){
       VertexAdj(th,d-90, 0,0,0, rockP.hmap[0][0],0,1 ,0.15,0.15,0.15);
     } else {
       VertexAdj(th,d-90, 0,0,0, rockP.hmap[i][0],0,1 ,0.15,0.15,0.15);
       i++;
     }
   }
   glEnd();

   //  Latitude bands
   int j = 0;
   for (int ph=d-90;ph<=90-2*d;ph+=d)
   {
      i=0;
      glBegin(GL_QUAD_STRIP);
      glColor3f(0.15,0.15,0.15);
      for (int th=0;th<=360;th+=d)
      {
        // first and last postion need to be the same so there are no gaps
        if(th == 360){
          VertexAdj(th,ph, 0,0,0, rockP.hmap[0][j],0,1 ,0.15,0.15,0.15);
          VertexAdj(th,ph+d, 0,0,0, rockP.hmap[0][j+1],0,1 ,0.15,0.15,0.15);
        } else {
          VertexAdj(th,ph, 0,0,0, rockP.hmap[i][j],0,1 ,0.15,0.15,0.15);
          VertexAdj(th,ph+d, 0,0,0, rockP.hmap[i][j+1],0,1 ,0.15,0.15,0.15);
        }
        i++;
      }
      j++;
      glEnd();
   }
   //  North pole cap
   glBegin(GL_TRIANGLE_FAN);
   VertexAdj(0,90, 0,0,0, rockP.hmap[0][10],0,1 ,0.15,0.15,0.15);
   i=0;
   for (int th=0;th<=360;th+=d)
   {
     // first and last postion need to be the same so there are no gaps
     if(th == 360){
       VertexAdj(th,90-d, 0,0,0, rockP.hmap[0][10],0,1 ,0.15,0.15,0.15);
     } else {
       VertexAdj(th,90-d, 0,0,0, rockP.hmap[i][10],0,1 ,0.15,0.15,0.15);
       i++;
     }
   }
   glEnd();

   //  Undo transformations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
/*
!!Mushroom object!!
*/
static void mushroom(double x,double y,double z,double r)
{
  const int d=15;

  vtx domeRim[25][2];
  vtx stem[25];

  //  Save transformation
  glPushMatrix();
  //  Offset and scale
  glTranslated(x,y,z);
  glScaled(r,r,r);

  //  Set specular color to white
  float white[] = {1,1,1,1};
  float black[] = {0,0,0,1};
  glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
  glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);

  //  outer top cap
  glBindTexture(GL_TEXTURE_2D,texture[5]);
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(0.5, 0.15, 0.23);
  glNormal3d(0,1,0);
  glVertex3d(0,1.5,0);
  for (int th=0;th<=360;th+=d)
  {
    VertexAdj(th,90-d, 0,0.5,0, 1,1,1, 0.5,0.15,0.23);
  }
  glEnd();

  //  outer Latitude bands basiically makes a dome shape
  for (int ph=90-2*d;ph>=(d-90)/6;ph-=d)
  {
     glBegin(GL_TRIANGLE_STRIP);
     int i = 0;
     for (int th=0;th<=360;th+=d)
     {
        VertexAdj(th,ph, 0,0.5,0, 1,1,1, 0.5,0.15,0.23);
        VertexAdj(th,ph+d, 0,0.5,0, 1,1,1, 0.5,0.15,0.23);
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
    VertexAdj(th,90-d, 0,-0.85,0, 0.85,1,0, 0.95,0.92,0.8);

    stem[c].x = 0.85*Sin(th)*Cos(90-d);
    stem[c].y = -0.85+0.85*  Sin(90-d);
    stem[c].z = 0.85*Cos(th)*Cos(90-d);
    c++;
  }
  glEnd();

  //  inner Latitude bands slighly smaller than the outer dome
  glBindTexture(GL_TEXTURE_2D,texture[6]);
  for (int ph=90-2*d;ph>=(d-90)/6;ph-=d)
  {
     glBegin(GL_TRIANGLE_STRIP);
     int j = 0;
     for (int th=0;th<=360;th+=d)
     {
        VertexAdj(th,ph, 0,0.5,0, 0.85,1,1, 0.95,0.92,0.8);
        VertexAdj(th,ph+d, 0,0.5,0, 0.85,1,1, 0.95,0.92,0.8);
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
  glBindTexture(GL_TEXTURE_2D,texture[6]);
  glBegin(GL_TRIANGLE_STRIP);
  int rimRep = 0;
  for(int g=0; g<25; g++){
    // uses the inner and outer vertices at the end of the domes to make the rim
    glColor3f(0.95,0.92,0.8);
    glNormal3d(0,-1,0);
    glTexCoord2f(rimRep/360,0); glVertex3d(domeRim[g][0].x,domeRim[g][0].y,domeRim[g][0].z);
    glTexCoord2f(rimRep/360,1); glVertex3d(domeRim[g][1].x,domeRim[g][1].y,domeRim[g][1].z);
    rimRep+=15;
  }
  glEnd();

  // sides of stem
  glBegin(GL_TRIANGLE_STRIP);
  for(int o=0; o<25; o++){
    glColor3f(0.95,0.92,0.9);
    glNormal3d(stem[o].x,stem[o].y,stem[o].z);
    glVertex3d(stem[o].x,stem[o].y,stem[o].z);
    glNormal3d(stem[o].x,1.5+stem[o].y,stem[o].z);
    glVertex3d(stem[o].x,1.5+stem[o].y,stem[o].z);

  }
  glEnd();

  //  Undo transformations
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);
}
/*
!!Flower object!!
*/
static void flower(double x,double y,double z,double r)
{
   //for makeing the stem
   vtx stemf[25][2];

   const int d=15;
   double xF;
   double yF;
   double zF;

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x,y,z);
   glScaled(r,r,r);

   //  Set specular color to white
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[5]);

   //  center of the flower reused from ex8
   for (int ph=-90;ph<90;ph+=d)
   {
      glBegin(GL_QUAD_STRIP);
      int i = 0;
      for (int th=0;th<=360;th+=d)
      {
         xF = 0.25*Sin(th)*Cos(ph);
         yF = 1.5+0.15*Sin(ph);
         zF = 0.25*Cos(th)*Cos(ph);
         glNormal3d(xF,yF,zF);
         if(yF <= 1.5){
           glColor3f(0.05, 0.45, 0.15);
         } else {
           glColor3f(0.6,0.55,0.15);
         }
         glVertex3d(xF,yF,zF);
         xF = 0.25*Sin(th)*Cos(ph+d);
         yF = 1.5+0.15*Sin(ph+d);
         zF = 0.25*Cos(th)*Cos(ph+d);
         glNormal3d(xF,yF,zF);
         glVertex3d(xF,yF,zF);
         i++;
      }
      glEnd();
   }

   //petals of the flower
   glBegin(GL_TRIANGLE_FAN);
   glColor3f(0.5, 0.15, 0.23);
   glNormal3d(0,1,0);
   glVertex3d(0,1.5,0);
   for (int th=0;th<=360;th+=d)
   {
     xF = 2*Sin(th)*Cos(90-d);
     yF = 1.59;
     zF = 2*Cos(th)*Cos(90-d);

     glColor3f(x+1/100, y/100,z/100);
     glNormal3d(xF,yF,zF);
     if(th%2 == 0){
       glVertex3d(xF,yF,zF);
     }
   }
   glEnd();

   int j = 0;
   //bottom of the stem reused from mushroom
   glBegin(GL_TRIANGLE_FAN);
   glColor3f(0.05, 0.45, 0.15);
   glNormal3d(0,-1,0);
   glVertex3d(0,0,0);
   for (int th=0;th<=360;th+=d)
   {
     xF = 0.25*Sin(th)*Cos(90-d);
     yF = 0;
     zF = 0.25*Cos(th)*Cos(90-d);
     glColor3f(0.05, 0.45, 0.15);
     glNormal3d(xF,yF,zF);
     glVertex3d(xF,yF,zF);
     stemf[j][0].x = xF;
     stemf[j][0].y = yF;
     stemf[j][0].z = zF;
     j++;
   }
   glEnd();

   j = 0;
   //break point makes bend in the stem
   glBegin(GL_TRIANGLE_FAN);
   glColor3f(0.05, 0.45, 0.15);
   glNormal3d(0,-1,0);
   glVertex3d(0.05,0.95,0.05);
   for (int th=0;th<=360;th+=d)
   {
     xF = 0.05+0.25*Sin(th)*Cos(90-d);
     yF = 0.95;
     zF = 0.05+0.25*Cos(th)*Cos(90-d);
     glColor3f(0.05, 0.45, 0.15);
     glNormal3d(xF,yF,zF);
     glVertex3d(xF,yF,zF);
     //saves the points for the sides of the stem
     stemf[j][1].x = xF;
     stemf[j][1].y = yF;
     stemf[j][1].z = zF;
     j++;

   }
   glEnd();

   glBegin(GL_QUAD_STRIP);
   for(int h=0;h<24;h++){
     glColor3f(0.05, 0.45, 0.15);
     // uses the vertices from the outside of the top and bottom to make the triangle strip
     glNormal3d(stemf[h][0].x,stemf[h][0].y,stemf[h][0].z);
     glVertex3d(stemf[h][0].x,stemf[h][0].y,stemf[h][0].z);
     glNormal3d(stemf[h][1].x,stemf[h][1].y,stemf[h][1].z);
     glVertex3d(stemf[h][1].x,stemf[h][1].y,stemf[h][1].z);
   }
   glEnd();

   glBegin(GL_QUAD_STRIP);
   for(int h=0;h<24;h++){
     glColor3f(0.05, 0.45, 0.15);
     // uses the vertices from the outside of the top and bottom to make the triangle strip
     glNormal3d(stemf[h][1].x,stemf[h][1].y,stemf[h][1].z);
     glVertex3d(stemf[h][1].x,stemf[h][1].y,stemf[h][1].z);
     glNormal3d(stemf[h][0].x,1.5+stemf[h][0].y,stemf[h][0].z);
     glVertex3d(stemf[h][0].x,1.5+stemf[h][0].y,stemf[h][0].z);
   }
   glEnd();
   //  Undo transformations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
/*
!!Pine Tree object!!
*/
static void tree2(double x,double y,double z, double r)
{

  const int d=15;

  vtx trunk[25];

  double xT;
  double yT;
  double zT;
  //  Save transformation
  glPushMatrix();
  //  Offset and scale
  glTranslated(x,y,z);
  glScaled(r,r,r);

  //  Set specular color to white
  float white[] = {1,1,1,1};
  float black[] = {0,0,0,1};
  glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
  glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);
  glBindTexture(GL_TEXTURE_2D,texture[7]);

  //  tree cap
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,2,0, 0.5,1,2, 0.2,0.52,0.3);
  for (int th=0;th<=360;th+=d)
  {
     VertexAdj(th,0, 0,2,0, 0.35,1,2, 0.2,0.52,0.3);
  }
  glEnd();
  //  bottom of tree cap
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,1.5,0, 0.5,1,2, 0.2,0.52,0.3);
  for (int th=0;th<=360;th+=d)
  {
     VertexAdj(th,0, 0,2,0, 0.35,1,2, 0.2,0.52,0.3);
  }
  glEnd();

  //  second level of leaves from the top
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,1.65,0, 0.5,1,2, 0.2,0.52,0.3);
  for (int th=0;th<=360;th+=d)
  {
     VertexAdj(th,0, 0,1.65,0, 0.5,1,2, 0.2,0.52,0.3);
  }
  glEnd();
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,1.15,0, 0.5,1,2, 0.2,0.52,0.3);
  for (int th=0;th<=360;th+=d)
  {
     VertexAdj(th,0, 0,1.65,0, 0.5,1,2, 0.2,0.52,0.3);
  }
  glEnd();

  //  third level from the top
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,1.25,0, 0.85,1,2, 0.2,0.52,0.3);
  for (int th=0;th<=360;th+=d)
  {
     VertexAdj(th,0, 0,1,0, 0.85,1,2, 0.2,0.52,0.3);
  }
  glEnd();
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,0.5,0, 0.85,1,2, 0.2,0.52,0.3);
  for (int th=0;th<=360;th+=d)
  {
     VertexAdj(th,0, 0,1,0, 0.85,1,2, 0.2,0.52,0.3);
  }
  glEnd();

  //  bottom level
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,0.5,0, 1.15,1,2, 0.2,0.52,0.3);
  for (int th=0;th<=360;th+=d)
  {
     VertexAdj(th,0, 0,0.5,0, 1.15,1,2, 0.2,0.52,0.3);
  }
  glEnd();
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,0,0, 1.15,1,2, 0.2,0.52,0.3);
  for (int th=0;th<=360;th+=d)
  {
     VertexAdj(th,0, 0,0.5,0, 1.15,1,2, 0.2,0.52,0.3);
  }
  glEnd();

  //trunk
  glBegin(GL_TRIANGLE_FAN);
  VertexAdj(0,90, 0,-0.25,0, 0.15,1,0, 0.5,0.35,0.15);
  int i=0;
  for (int th=0;th<=360;th+=d)
  {
     xT = 0.15*Sin(th)*Cos(0);
     yT = 0.15*        Sin(0);
     zT = 0.15*Cos(th)*Cos(0);
     glColor3f(0.5,0.35,0.15);
     glNormal3d(xT,yT,zT);
     glVertex3d(xT,yT,zT);
     //saves the edges of the fan in order to make the sides of the trunk
     trunk[i].x = xT;
     trunk[i].y = yT;
     trunk[i].z = zT;
     i++;
  }
  glEnd();
  //trunk sides
  glBindTexture(GL_TEXTURE_2D,texture[0]);
  int rep=0;
  int rep2=1;
  glBegin(GL_TRIANGLE_STRIP);
  glColor3f(0.5,0.35,0.15);
  for(int j=0;j<25;j++){
     glNormal3d(trunk[j].x,trunk[j].y,trunk[j].z);
     glTexCoord2f(rep2,rep); glVertex3d(trunk[j].x,trunk[j].y,trunk[j].z);
     glNormal3d(trunk[j].x,trunk[j].y+1.15,trunk[j].z);
     glTexCoord2f(rep2,rep2); glVertex3d(trunk[j].x,trunk[j].y+1.15,trunk[j].z);
     rep = 1-rep;
     rep2 = 1-rep2;
  }
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}

void myInit() {
  //gets the rock seeds
  for(int j = 0;j < objPos.getNumRocks();j++){
    rockSeed[j] = rocks.getSeed();
    rocks.randSeed();
  }
  randTrees = new FractalTree[objPos.getNumTrees()];
  //sets the octaves so that the intial scene is not flat
  perlin.setOctaves(5);
  perlin.changeScalingBias();
  rocks.setOctaves(4);
  rocks.changeScalingBias();
}

static void draw(float waterLevel){
  if(object == 1)
    // just draws the tree trunk used to make the trees
    randTrees[0].root(0,0,0, 0.05,0.45, 1,1);
  else if(object == 2)
    mushroom(0,0,0,1);
  else if(object == 3)
    terrain((col/2)*-1,0,row/2*-1, 1,1,1 , 0, waterLevel);
  else if(object == 4)
    rock(0,0,0 ,1, rocks.getSeed());
  else if(object == 5)
    // draws a single tree
    randTrees[0].makeTree(1, 0.075, 0.45 ,0,0,0);
  else if(object == 6)
    flower(0,0,0,1);
  else if(object == 7)
    tree2(0,0,0, 1);
  // main scene
  else {
    Point *treePos = objPos.getTreePos();
    Point *rockPos = objPos.getRockPos();
    Point *mushPos = objPos.getMushPos();
    Point *flowPos = objPos.getFlowPos();
    Point *pinePos = objPos.getPinePos();
    terrain((col/2)*-1,0,row/2*-1, 1,1,1 , 0, waterLevel);
    water((col/2)*-1,waterLevel,row/2*-1, 1,1,1 , 0);
    //loop that places all the trees
    for(int i = 0;i < objPos.getNumTrees();i++){
       if(t.hmap[treePos[i].x][treePos[i].y]>waterLevel+0.5){ //only places trees if they are high enough above the waterlevel
          randTrees[i].makeTree(0.5,0.075,0.45, (col/2*-1)+treePos[i].y,t.hmap[treePos[i].x][treePos[i].y],(row/2*-1)+treePos[i].x);
       }
    }
    //loop that places all the rocks
    for(int j = 0;j < objPos.getNumRocks();j++){
       if(t.hmap[rockPos[j].x][rockPos[j].y]>waterLevel+0.5){
          rock((col/2*-1)+rockPos[j].y,t.hmap[rockPos[j].x][rockPos[j].y],(row/2*-1)+rockPos[j].x, 0.75, rockSeed[j]);
       }
    }
    //loop that places all of the mushrooms
    for(int k = 0;k < objPos.getNumMush();k++){
       if(t.hmap[mushPos[k].x][mushPos[k].y]>waterLevel+0.5){
          mushroom((col/2*-1)+mushPos[k].y,t.hmap[mushPos[k].x][mushPos[k].y],(row/2*-1)+mushPos[k].x, 0.15);
       }

    }
    //loop that places all of the Flowers
    for(int l = 0;l < objPos.getNumFlow();l++){
       if(t.hmap[flowPos[l].x][flowPos[l].y]>waterLevel+0.5){
          flower((col/2*-1)+flowPos[l].y,t.hmap[flowPos[l].x][flowPos[l].y],(row/2*-1)+flowPos[l].x, 0.25);
       }
    }
    //loop that places all of the Flowers
    for(int l = 0;l < objPos.getNumPine();l++){
       if(t.hmap[pinePos[l].x][pinePos[l].y]>waterLevel+0.5){
          tree2((col/2*-1)+pinePos[l].y,t.hmap[pinePos[l].x][pinePos[l].y],(row/2*-1)+pinePos[l].x, 0.85);
       }
    }
  }
}

void display()
{
  float fogColor[4] = {0.5, 0.5, 0.5, 1.0};
  // gets the height for each point in the terrain mesh using the noise functions
  perlin.noise2D(perlin.getOutputWSize(), perlin.getOutputHSize(), perlin.getSeed(), perlin.getOctaves(), perlin.getScalingBias(), perlin.getOutput());
  float *fPerlinNoise2D = perlin.getOutput();
  int h = 0;
  for(int i=0; i<row;i++){
    for(int j=0; j<col; j++){
       // saves the values for the height of the terrian using Perlin noise
       t.hmap[j][i] = map(fPerlinNoise2D[h],0,1,0,10);
       h++;
    }
  }
  // gets the minimum and max height of terrain
  float *minMax = findMinMax(fPerlinNoise2D);

  ///  Erase the window and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  // only enables fog in the main scene
  if(object == 0){
     glEnable (GL_FOG);
     glFogi (GL_FOG_MODE, GL_EXP2); // sets the fog mode
     glFogfv (GL_FOG_COLOR, fogColor); // sets the fog color
     glFogf (GL_FOG_DENSITY, density); // sets desity which can be changed
     glHint (GL_FOG_HINT, GL_NICEST);
  } else {
     glDisable(GL_FOG);
  }

  //enables and disables the wire frame
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
  else if(mode==1){
     //  Eye position
     double Ex = -2*dim*Sin(th)*Cos(ph);
     double Ey = +2*dim        *Sin(ph);
     double Ez = +2*dim*Cos(th)*Cos(ph);

     if(object == 0 || object == 3)
       gluLookAt(Ex,minMax[1]+Ey,Ez , Ox,minMax[1]+Oy,Oz , 0,1,0); // makes sure the terrain is always in view when teraain is changed
     else
       gluLookAt(Ex,Ey,Ez , Ox,Oy,Oz , 0,1,0);
  }
  //first person
  else if (mode == 2 && object == 0) {
     Cx = +2*dim*Sin(th); //Ajust the camera vector based on th
     Cz = -2*dim*Cos(th);
     fpy = +2*dim*Sin(ph); // allows for looking up and down

     gluLookAt(fpx,minMax[1]+Cy,fpz, Cx+fpx,Cy+fpy,Cz+fpz, 0,1,0); //  Use gluLookAt, y is the up-axis
  }

  //  Light switch from ex13
  if (light)
  {
     //  Translate intensity to color vectors
     float setAmb = 0.01*ambient;
     float Ambient[]   = {setAmb ,setAmb ,setAmb ,1.0};
     float setDiff = 0.01*diffuse;
     float Diffuse[]   = {setDiff ,setDiff ,setDiff ,1.0};
     float setSpec = 0.01*specular;
     float Specular[]  = {setSpec,setSpec,setSpec,1.0};
     //  Light position
     float Position[]  = {distance*Cos(zh),distance*Sin(zh),1.0,0};
     //  Draw light position as ball (still no lighting here)
     glColor3f(1,1,1);
     sphere(Position[0],Position[1],Position[2] , 0.1, 1,1,1);
     //  OpenGL should normalize normal vectors
     glEnable(GL_NORMALIZE);
     //  Enable lighting
     glEnable(GL_LIGHTING);
     //  Location of viewer for specular calculations
     glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
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

  float waterLevel = minMax[1]/2; // gets the water level based on the highest part of the terrian
  draw(waterLevel);

  glDisable(GL_LIGHTING);
  glDisable(GL_FOG);
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
  Print("Angle=%d,%d  Dim=%.1f Fov=%d Mode=%s Object=%s",th,ph,dim,fov,modeText[mode],objText[object]);

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
  // increases and decreases camera height
  else if (key == GLUT_KEY_PAGE_UP)
     Cy += 0.5;
  else if (key == GLUT_KEY_PAGE_DOWN)
     Cy -= 0.5;
  //  Keep angles to +/-360 degrees
  th %= 360;
  ph %= 360;

  //  Update projection
  Project();
  //  Tell GLUT it is necessary to redisplay the scene
  glutPostRedisplay();
}

void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   // zooms in
   else if (ch == '+')
       dim -= 0.1;
   //zooms out
   else if (ch == '-')
       dim += 0.1;
   else if (ch == 'z')
       axes = 1-axes;
   //  changes fov
   else if (ch == 'f')
       fov -= 1;
   else if (ch == 'F')
       fov += 1;
   else if (ch == 'q')
       density -= 0.01;
   else if (ch == 'Q')
       density += 0.01;
   // changes the tree
   else if (ch == 'r')
       randTrees[0].randVars();
   // toggles light
   else if(ch == 'l' || ch == 'L')
     light = 1-light;
   //changes mode between perspective and orthogonal
   else if(ch=='m' || ch == 'M'){
      fpy=0;
      fpx=0;
      fpz=0;
      mode = (mode+1)%3;
      if(object!=0 && mode ==2){
        mode=0;
      }
   }
   //shows the differnt objects made and the scene
   else if(ch == '0'){
      object = 0;
      dim = 10;
      ph = 175;
   }
   else if(ch == '1'){
     object = 1;
     dim = 3.0;
     mode = 1;
   }
   else if(ch == '2'){
     object = 2;
     dim = 3.0;
     mode = 1;
   }
   else if(ch == '3'){
      object = 3;
      dim = 21.0;
      mode = 1;
   }
   else if(ch == '4'){
     object = 4;
     dim = 3.0;
     mode = 1;
   }
   else if(ch == '5'){
     object = 5;
     dim = 3.0;
     mode = 1;
   }
   else if(ch == '6'){
     object = 6;
     dim = 3.0;
     mode = 1;
   }
   else if(ch == '7'){
     object = 7;
     dim = 3.0;
     mode = 1;
   }
   //toggles the wireframe
   else if(ch == 'i' ||ch == 'I')
     wireframe = (wireframe+1)%2;
   // c, v, and x control different aspects of terrian along with regenerating terrian
   else if(ch == 'c'){
     if(object == 4){
       rocks.changeOctaves();
       if(rocks.getOctaves() == 5){
         rocks.resetOctaves();
       }
     } else {
       perlin.changeOctaves();
     }
   }
   else if(ch == 'v'){
     perlin.changeScalingBias();
     rocks.changeScalingBias();
   }
   else if(ch == 'x'){
     perlin.resetOctaves();
     perlin.randSeed();
     if(object!=0 || object!=3){
        rocks.randSeed();
     }
   }
   // first person
   if(mode == 2 && object == 0){
     double dt = 0.05;
     dim=4;
     if(ch == 'w'){
       fpx += Cx*dt;
       fpz += Cz*dt;
     }
     if(ch == 'a'){
       fpx += (+2*dim*Sin(th-90))*dt;
       fpz += (-2*dim*Cos(th-90))*dt;
     }
     else if(ch == 's'){
       fpx -= Cx*(dt);
       fpz -= Cz*(dt);
     }
     if(ch == 'd'){
       fpx += (+2*dim*Sin(th+90))*dt;
       fpz += (-2*dim*Cos(th+90))*dt;
     }
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
   glutCreateWindow("Final Project: Sean Masucci");
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
   glutIdleFunc(idle);
   //  Load textures
   texture[0] = LoadTexBMP("bark.bmp");
   texture[1] = LoadTexBMP("leaves.bmp");
   texture[2] = LoadTexBMP("grass.bmp");
   texture[3] = LoadTexBMP("water.bmp");
   texture[4] = LoadTexBMP("rock.bmp");
   texture[5] = LoadTexBMP("mushroomCap.bmp");
   texture[6] = LoadTexBMP("bottomMushroom.bmp");
   texture[7] = LoadTexBMP("pineTree.bmp");
   myInit();
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
