/*
 * ap.h
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */

#ifndef SRC_AP_AP_H_
#define SRC_AP_AP_H_

#include "hw.h"


void apInit(void);
void apMain(int argc, char *argv[]);
void apExit(void);

void ledControl(int argc, char *argv[]);

#endif /* SRC_AP_AP_H_ */
