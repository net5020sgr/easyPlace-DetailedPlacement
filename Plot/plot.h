#ifndef PLOT_H
#define PLOT_H
#include "global.h"
#include "CImg.h"
using namespace cimg_library;
const unsigned char Blue[] = {120, 200, 255},
                    Black[] = {0, 0, 0},
                    Green[] = {0, 150, 0},
                    Orange[] = {255, 165, 0},
                    Red[] = {255, 0, 0};
int getX(float, float, float);
int getY(float, float, float, float);
#endif