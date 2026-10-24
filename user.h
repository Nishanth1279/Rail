#ifndef USER_H
#define USER_H

#include "common.h"

typedef struct User
{
    int passengerId;

    char username[100];

    char password[100];

    struct User *prev;
    struct User *next;

} User;

extern User *userHead;

void initializeUserModule(void);

void loadUserData(void);

void saveUserData(void);

int registerUser(void);

int loginUser(void);

#endif