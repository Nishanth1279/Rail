#ifndef TRAIN_H
#define TRAIN_H

#include "common.h"

extern Train *trainHead;

void initializeTrainModule(void);

Train *findTrain(int trainNumber);

void loadTrainData(void);

void saveTrainData(void);

void addTrain(void);

void viewTrainDetails(void);

void searchTrain(void);

void updateTrain(void);

void deleteTrain(void);

void applyTrainDelay(Train *train,int delayMinutes);


#endif