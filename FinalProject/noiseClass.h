#include "CSCIx229.h"

#include <iostream>
#include <string>

//based on stuff rom oneloneCoder
class noiseClass{
  private:

    int nOutputWidth = 32;
	  int nOutputHeight = 32;
  	float *fNoiseSeed2D = nullptr; // hold the seed for the terrian generatin
  	float *fPerlinNoise2D = nullptr; //has the output values for the height of points
    //number of vertices
  	int nOutputSize = 1024;

  	int nOctaveCount = 1;
  	float fScalingBias = 2.0f; // effects how smooth the points will look

  public:
    noiseClass();
    void noise2D(int nWidth, int nHeight, float *fSeed, int nOctaves, float fBias, float *fOutput);
    void randSeed();
    float *getSeed();
    void changeOctaves();
    int getOctaves();
    void resetOctaves();
    void changeScalingBias();
    float getScalingBias();
    float* getOutput();
    int getOutputWSize();
    int getOutputHSize();
};
