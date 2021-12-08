#include "CSCIx229.h"
#include "FractalTree.h"
#include "noiseClass.h"

#include <iostream>
#include <string>

struct lHmapStruct{float hmap[25][14];} leavesP;
noiseClass leaves(13,25);

FractalTree::FractalTree(){
    angle1 = 15+(rand()%60);
    angle2 = 15+(rand()%45);
    angle3 = 15+(rand()%75);
    angle4 = 15+(rand()%60);
    seed = rand()%3;
    leaves.setOctaves(4);
    leaves.changeScalingBias();
    texture = new unsigned int[2];
    texture[0] = LoadTexBMP("bark.bmp");
    texture[1] = LoadTexBMP("leaves.bmp");
}
FractalTree::~FractalTree(){
   delete[] texture;
}
//reused from final.cpp
void FractalTree::VertexAdj(double th,double ph, double xPos, double yPos, double zPos, float scale, int flip, int tex,
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
//reused from final.cpp
float FractalTree::map(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//reused code from the rock function
void FractalTree::leavesF(double x,double y,double z,double r)
{
   //makes the points to make the sphere look more like a rock and takes in a new seed for each rock
   leaves.noise2D(leaves.getOutputWSize(), leaves.getOutputHSize(), leaves.getSeed(), leaves.getOctaves(), leaves.getScalingBias(), leaves.getOutput());
   float *fLeavesNoise2D = leaves.getOutput();
   int h = 0;
   //sets a height based on the x, y positon.
   for(int i=0; i<13;i++){
     for(int j=0; j<25; j++){
        // saves the values for the hight of the terrian using Perlin noise
        leavesP.hmap[j][i] = map(fLeavesNoise2D[h],0,1,0.5,r);
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

   glMaterialf(GL_FRONT,GL_SHININESS,0);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[1]);

   //  South pole cap
   glBegin(GL_TRIANGLE_FAN);
   VertexAdj(0,-90, 0,0,0, leavesP.hmap[0][0],0,1 ,0.2, 0.45, 0.15);
   int i = 0;
   for (int th=0;th<=360;th+=d)
   {
     // the first and last postion need to be the same so that they line up
     if(th == 360){
       VertexAdj(th,d-90, 0,0,0, leavesP.hmap[0][0],0,1 ,0.2, 0.45, 0.15);
     } else {
       VertexAdj(th,d-90, 0,0,0, leavesP.hmap[i][0],0,1 ,0.2, 0.45, 0.15);
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
          VertexAdj(th,ph, 0,0,0, leavesP.hmap[0][j],0,1 ,0.2, 0.45, 0.15);
          VertexAdj(th,ph+d, 0,0,0, leavesP.hmap[0][j+1],0,1 ,0.2, 0.45, 0.15);
        } else {
          VertexAdj(th,ph, 0,0,0, leavesP.hmap[i][j],0,1 ,0.2, 0.45, 0.15);
          VertexAdj(th,ph+d, 0,0,0, leavesP.hmap[i][j+1],0,1 ,0.2, 0.45, 0.15);
        }
        i++;
      }
      j++;
      glEnd();
   }
   //  North pole cap
   glBegin(GL_TRIANGLE_FAN);
   VertexAdj(0,90, 0,0,0, leavesP.hmap[0][10],0,1 ,0.2, 0.45, 0.15);
   i=0;
   for (int th=0;th<=360;th+=d)
   {
     // first and last postion need to be the same so there are no gaps
     if(th == 360){
       VertexAdj(th,90-d, 0,0,0, leavesP.hmap[0][10],0,1 ,0.2, 0.45, 0.15);
     } else {
       VertexAdj(th,90-d, 0,0,0, leavesP.hmap[i][10],0,1 ,0.2, 0.45, 0.15);
       i++;
     }
   }
   glEnd();

   //  Undo transformations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
void FractalTree::root(double x,double y,double z,double rtop, double rbtm, double s, double h)
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

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[0]);

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

   //  top cap
   for (int ph=0;ph<90;ph+=d)
   {
      glBegin(GL_QUAD_STRIP);
      int j = 0;
      for (int th=0;th<=360;th+=d)
      {
         xN = rtop*Sin(th)*Cos(ph);
         yN = 1+0.1*       Sin(ph);
         zN = rtop*Cos(th)*Cos(ph);

         glColor3f(0.5,0.4,0.2);
         glNormal3d(xN,yN,zN);
         glTexCoord2f(th/360,ph/180); glVertex3d(xN,yN,zN);

         xN = rtop*Sin(th)*Cos(ph+d);
         yN = 1+0.1*       Sin(ph+d);
         zN = rtop*Cos(th)*Cos(ph+d);

         glColor3f(0.5,0.4,0.2);
         glNormal3d(xN,yN,zN);
         glTexCoord2f(th/360,ph/180); glVertex3d(xN,yN,zN);
         if(ph==0){
           treeBrancht[j].x=xN;
           treeBrancht[j].y=yN;
           treeBrancht[j].z=zN;
         }
         j++;
      }
      glEnd();
    }

    // creates the sides of the brach
    double rep = 0;
    double rep2 = 1;
    glBegin(GL_TRIANGLE_STRIP);
    for(int h=0;h<25;h++){
      glColor3f(0.5,0.35,0.2);
      // uses the vertices from the outside of the top and bottom caps to make the triangle strip for the sides
      glNormal3d(treeBrancht[h].x,treeBrancht[h].y,treeBrancht[h].z);
      glTexCoord2f(rep2,rep); glVertex3d(treeBrancht[h].x,treeBrancht[h].y,treeBrancht[h].z);
      glNormal3d(treeBranchb[h].x,treeBranchb[h].y,treeBranchb[h].z);
      glTexCoord2f(rep2,rep2); glVertex3d(treeBranchb[h].x,treeBranchb[h].y,treeBranchb[h].z);
      rep = 1-rep;
      rep2 = 1-rep2;
    }
    glEnd();
    //  Undo transformations
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
 }
 // gets new seed and randomizes the tree
void FractalTree::randVars(){
   angle1 = 15+(rand()%60);
   angle2 = 15+(rand()%45);
   angle3 = 15+(rand()%75);
   angle4 = 15+(rand()%60);
   seed = rand()%3;
 }
 //makes the tree using recurstion
void FractalTree::makeTree(float len, float stop, float sbtm, double x, double y, double z){
  //root brach
  root(x,y,z, stop,sbtm, 1,len);
  if(len>0.16){
    switch(seed){
       case 0:
         // braches from root
         glPushMatrix();
         //sets the new orgin at the top of the root
         glTranslated(x,len*2+y,z);
         // picks what axes to rotate around
         if(angle1 > 30){
           glRotated(angle4,1,0,0);
           glRotated(angle2,0,1,0);
         } else{
           glRotated(angle4,0,0,1);
           glRotated(angle3,0,1,0);
         }
         // starts the next brach
         // decrease the length and radius of the branch
         makeTree(len*0.67, stop*0.75, sbtm*0.67 ,0,0,0);
         glPopMatrix(); // resets orgin to top of previous branch
         //makes the other branch
         glPushMatrix();
         glTranslated(x,len*2+y,z);
         if(angle2 < -22){
           glRotated(-1*angle2,1,1,1);
           glRotated(angle1,0,1,0);
         } else{
           glRotated(-1*angle3,1,0,1);
         }
         // starts new branch in another direction
         makeTree(len*0.67, stop*0.75, sbtm*0.67 ,0,0,0);
         glPopMatrix();
         break;
       // different seed to make a different tree
       case 1:
         glPushMatrix();
         //sets the new orgin at the top of the root
         glTranslated(x,len*2+y,z);
         // picks what axes to rotate around
         glRotated(angle1,0,0,1);
         glRotated(angle1,0,1,0);

         //braches one way
         // decrease the length and radius of the branch
         makeTree(len*0.67, stop*0.75, sbtm*0.67 ,0,0,0);
         glPopMatrix(); // resets orgin to top of previous branch
         //makes the other branch
         glPushMatrix();
         glTranslated(x,len*2+y,z);

         glRotated(-1*angle2,0,0,1);
         glRotated(angle4,0,1,0);
         //braches the other way
         makeTree(len*0.67, stop*0.75, sbtm*0.67 ,0,0,0);
         glPopMatrix();

         //makes the other branch
         glPushMatrix();
         glTranslated(x,len*2+y,z);

         glRotated(-1*angle2,1,0,0);
         glRotated(angle3,0,1,0);
         //braches the other way
         makeTree(len*0.67, stop*0.75, sbtm*0.67 ,0,0,0);
         glPopMatrix();
         break;
       // different seed to make a different tree
       case 2:
         glPushMatrix();
         //sets the new orgin at the top of the root
         glTranslated(x,len*2+y,z);

         glRotated(angle4,1,1,0);
         glRotated(angle1,0,1,0);
         // braches one way
         // decrease the length and radius of the branch
         makeTree(len*0.67, stop*0.75, sbtm*0.67 ,0,0,0);
         glPopMatrix(); // resets orgin to top of previous branch

         //makes the other branch
         glPushMatrix();
         glTranslated(x,len*2+y,z);

         glRotated(-1*angle3,1,1,1);
         glRotated(angle2,0,1,0);

         //braches the other way
         makeTree(len*0.67, stop*0.75, sbtm*0.67 ,0,0,0);
         glPopMatrix();
         //only branches depending on lentgth
         if(len>0.335){
            //makes the other branch
            glPushMatrix();
            glTranslated(x,len+y,z);

            glRotated(-1*angle2,1,1,1);
            glRotated(angle3,0,1,0);
            //braches the other way
            makeTree(len*0.67, stop*0.75, sbtm*0.67 ,0,0,0);
            glPopMatrix();
         } else {
            glPushMatrix();
            glTranslated(x,len*2+y,z);
            leavesF(0,0,0, 0.4);
            glPopMatrix();
         }
         break;
    }
  } else if(len<0.16){
     glPushMatrix();
     glTranslated(x,len*2+y,z);
     leavesF(0,0,0, 0.5);
     glPopMatrix();
  }
}
