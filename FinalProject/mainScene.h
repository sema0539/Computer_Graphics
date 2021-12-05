#include "CSCIx229.h"

#include <iostream>
#include <string>
#include <random>

typedef struct{
  int x;
  int y;
}Point;

class mainScene{
  private:

    std::default_random_engine generator;
    //intializes the number of things that are in the main scene
    Point minT;
    Point maxT;
    int numTrees;
    int numRocks;
    int numMush;
    int numFlow;
    //holds the postion of the objects
    Point *treePos = nullptr;
    Point *rockPos = nullptr;
    Point *mushPos = nullptr;
    Point *flowPos = nullptr;

  public:
    mainScene(int nTrees, int nRocks, int nMush, int nFlow, int min, int max);
    ~mainScene();
    Point randomPoint(Point min, Point max);
    Point *getTreePos();
    Point *getRockPos();
    Point *getMushPos();
    Point *getFlowPos();
    int getNumTrees();
    int getNumRocks();
    int getNumMush();
    int getNumFlow();


};
