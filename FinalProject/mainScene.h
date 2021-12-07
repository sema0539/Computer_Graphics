#include "CSCIx229.h"

#include <iostream>
#include <string>
#include <random>

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
    int numPine;
    //holds the postion of the objects
    Point *treePos = nullptr;
    Point *rockPos = nullptr;
    Point *mushPos = nullptr;
    Point *flowPos = nullptr;
    Point *pinePos = nullptr;

  public:
    mainScene(int nTrees, int nRocks, int nMush, int nFlow, int nPine, int min, int max);
    ~mainScene();
    Point randomPoint(Point min, Point max);
    Point *getTreePos();
    Point *getRockPos();
    Point *getMushPos();
    Point *getFlowPos();
    Point *getPinePos();
    int getNumTrees();
    int getNumRocks();
    int getNumMush();
    int getNumFlow();
    int getNumPine();


};
