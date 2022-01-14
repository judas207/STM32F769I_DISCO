/*
 * main.cpp
 *
 *  Created on: Nov 26, 2021
 *      Author: hs
 */




#include "main.h"




int main(void)
{
  bspInit();

  hwInit();
  apInit();

  apMain();

  return 0;
}

