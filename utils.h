#ifndef UTILS_H
#define UTILS_H

#include "common.h"

void initializeSystem(void);

void cleanupSystem(void);

void clearInputBuffer(void);

int getIntegerInput(const char *message);

void getStringInput(const char *message,
                    char *buffer,
                    int size);
float getFloatInput(
    const char *message);
int isValidJourneyDate(const char *date);
    
#endif