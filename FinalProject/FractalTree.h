#include "CSCIx229.h"

#include <iostream>
#include <string>

typedef struct{std::string a,b;}lsys;

class FractalTree{
  private:
    int angle1;
    int angle2;
    int angle3;
    int angle4;
    int seed;
    unsigned int *texture;
    void VertexAdj(double th,double ph, double xPos, double yPos, double zPos, float scale, int flip, int tex,double r, double g, double b);
    float map(float x, float in_min, float in_max, float out_min, float out_max);

  public:
    FractalTree();
    ~FractalTree();
    void leavesF(double x,double y,double z,double r);
    void root(double x,double y,double z,double rtop, double rbtm, double s, double h);
    void randVars();
    void makeTree(float len, float stop, float sbtm, double x, double y, double z);

};
