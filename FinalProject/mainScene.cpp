#include "CSCIx229.h"
#include "mainScene.h"

#include <iostream>
#include <string>
#include <random>

mainScene::mainScene(int nTrees, int nRocks, int nMush, int nFlow, int nPine, int min, int max){
   minT.x = min;
   minT.y = min;
   maxT.x = max;
   maxT.y = max;
   numTrees = nTrees;
   numRocks = nRocks;
   numMush = nMush;
   numFlow = nFlow;
   numPine = nPine;
   treePos = new Point[numTrees];
   rockPos = new Point[numRocks];
   mushPos = new Point[numMush];
   flowPos = new Point[numFlow];
   pinePos = new Point[numPine];
   Point outT;
   //gets the tree postions
   for(int i = 0;i < numTrees;i++){
     outT = randomPoint(minT, maxT);
     treePos[i].x = outT.x;
     treePos[i].y = outT.y;
   }
   //gets the rock positions
   for(int j = 0;j < numRocks;j++){
     outT = randomPoint(minT, maxT);
     rockPos[j].x = outT.x;
     rockPos[j].y = outT.y;
   }
   //gets the mushroom postions
   for(int k = 0;k < numMush;k++){
     outT = randomPoint(minT, maxT);
     mushPos[k].x = outT.x;
     mushPos[k].y = outT.y;
   }
   //gets the flower postions
   for(int l = 0;l < numFlow;l++){
     outT = randomPoint(minT, maxT);
     flowPos[l].x = outT.x;
     flowPos[l].y = outT.y;
   }
   //gets the Pine tree postions
   for(int l = 0;l < numPine;l++){
     outT = randomPoint(minT, maxT);
     pinePos[l].x = outT.x;
     pinePos[l].y = outT.y;
   }
}
mainScene::~mainScene(){
  delete[] treePos;
  delete[] rockPos;
  delete[] mushPos;
  delete[] flowPos;
  delete[] pinePos;
}
// uniformally distributs points on the terrain
Point mainScene::randomPoint(Point min, Point max){
  static std::mt19937 gen;
  std::uniform_int_distribution<> distribX(min.x, max.x);
  std::uniform_int_distribution<> distribY(min.y, max.y);

  return Point{distribX(gen), distribY(gen)};
}
Point *mainScene::getTreePos(){return treePos;}
Point *mainScene::getRockPos(){return rockPos;}
Point *mainScene::getMushPos(){return mushPos;}
Point *mainScene::getFlowPos(){return flowPos;}
Point *mainScene::getPinePos(){return pinePos;}
int mainScene::getNumTrees(){return numTrees;}
int mainScene::getNumRocks(){return numRocks;}
int mainScene::getNumMush(){return numMush;}
int mainScene::getNumFlow(){return numFlow;}
int mainScene::getNumPine(){return numFlow;}
