#include "CSCIx229.h"

#include <iostream>
#include <string>

class noiseClass{
  private:

    int nOutputWidth;
	  int nOutputHeight;
  	float *fNoiseSeed2D = nullptr; // hold the seed for the terrian generatin
  	float *fPerlinNoise2D = nullptr; //has the output values for the height of points
    //number of vertices
  	int nOutputSize;

  	int nOctaveCount = 1;
  	float fScalingBias = 2.0f; // effects how the object will look

  public:
    noiseClass(int Width, int Height);
    ~noiseClass();
    void noise2D(int nWidth, int nHeight, float *fSeed, int nOctaves, float fBias, float *fOutput);
    void randSeed();
    float *getSeed();
    void changeOctaves();
    void setOctaves(int n);
    int getOctaves();
    void resetOctaves();
    void changeScalingBias();
    float getScalingBias();
    float* getOutput();
    int getOutputWSize();
    int getOutputHSize();
};
