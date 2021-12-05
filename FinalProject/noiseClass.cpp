#include "CSCIx229.h"
#include "noiseClass.h"

#include <iostream>
#include <string>

//initalizes the seed and output array
noiseClass::noiseClass(int Width, int Height){
  nOutputWidth = Width;
  nOutputHeight = Height;
  nOutputSize = nOutputWidth*nOutputHeight;
  fNoiseSeed2D = new float[nOutputSize];
  fPerlinNoise2D = new float[nOutputSize];
  for (int i = 0; i < nOutputSize; i++) fNoiseSeed2D[i] = (float)rand() / (float)RAND_MAX;

}
/*
This function approximates perlin noise and is taken from https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_PerlinNoise.cpp
but it is similar to the perline noise algorithm found on wikipedia and I implemented it into this class.
*/
void noiseClass::noise2D(int nWidth, int nHeight, float *fSeed, int nOctaves, float fBias, float *fOutput)
{
		for (int x = 0; x < nWidth; x++)
			for (int y = 0; y < nHeight; y++)
			{
				float fNoise = 0.0f;
				float fScaleAcc = 0.0f;
				float fScale = 1.0f;

				for (int o = 0; o < nOctaves; o++)
				{
					int nPitch = nWidth >> o;
					int nSampleX1 = (x / nPitch) * nPitch;
					int nSampleY1 = (y / nPitch) * nPitch;

					int nSampleX2 = (nSampleX1 + nPitch) % nWidth;
					int nSampleY2 = (nSampleY1 + nPitch) % nWidth;

					float fBlendX = (float)(x - nSampleX1) / (float)nPitch;
					float fBlendY = (float)(y - nSampleY1) / (float)nPitch;

					float fSampleT = (1.0f - fBlendX) * fSeed[nSampleY1 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY1 * nWidth + nSampleX2];
					float fSampleB = (1.0f - fBlendX) * fSeed[nSampleY2 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY2 * nWidth + nSampleX2];

					fScaleAcc += fScale;
					fNoise += (fBlendY * (fSampleB - fSampleT) + fSampleT) * fScale;
					fScale = fScale / fBias;
				}

				// Scale to seed range
				fOutput[y * nWidth + x] = fNoise / fScaleAcc;
			}
}
//gerates a new seed
void noiseClass::randSeed(){
  for (int i = 0; i < nOutputSize; i++) fNoiseSeed2D[i] = (float)rand() / (float)RAND_MAX;
}

float* noiseClass::getSeed(){
  return fNoiseSeed2D;
}
//increases the octaves until it reaches a max of 7
void noiseClass::changeOctaves(){
  nOctaveCount += 1;
  if(nOctaveCount == 7){
    nOctaveCount = 1;
  }
}
void noiseClass::setOctaves(int n){nOctaveCount = n;}
int noiseClass::getOctaves(){ return nOctaveCount;}
void noiseClass::resetOctaves(){nOctaveCount = 1;}
//decreases the scalling bias which can make the scene look less natural
void noiseClass::changeScalingBias(){
  fScalingBias = fScalingBias - 0.2f;
  if(fScalingBias < 0.2f){
    fScalingBias = 2.0f;
  }
}
float noiseClass::getScalingBias(){return fScalingBias;}
float* noiseClass::getOutput(){
  return fPerlinNoise2D;
}
int noiseClass::getOutputWSize(){return nOutputWidth;}
int noiseClass::getOutputHSize(){return nOutputHeight;}
noiseClass::~noiseClass(){
  delete[] fNoiseSeed2D;
  delete[] fPerlinNoise2D;
}
