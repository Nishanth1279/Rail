#include "utils.h"
#include "train.h"
#include "passenger.h"
#include "user.h"
#include "trainschedule.h"
#include "usermenu.h"
#include "adminmenu.h"
#include "availability.h"
#include "reserved_booking.h"
#include "pnr_status.h"
#include "admin_auth.h"
#include <setjmp.h>

int loggedInPassengerId = FAILURE;

int main(void)
{
    
    volatile int invalidChoices = 0;
    jmp_buf main_env;

    initializeSystem();

    initializeTrainModule();

    initializeTrainScheduleModule();
    
    initializeAvailabilityModule();
    
    initializeBookingModule();

    initializePassengerModule();

    initializeUserModule();

    (void)setjmp(main_env);
    railway_set_navigation_target(&main_env);

    while(1)
    {
        int choice;
        railway_set_navigation_target(&main_env);
        printf(
        "\n========== MAIN MENU ==========");

        printf(
        "\n1. Register User");

        printf(
        "\n2. Login User");

        printf(
        "\n3. Admin Menu");

        printf(
        "\n4. PNR Status Verification");

        printf(
        "\n0. Exit");

        choice =
        getIntegerInput(
        "\nEnter Choice (0-4): ");

        if (choice >= 0 && choice <= 4) invalidChoices = 0;
        if (choice < 0 || choice > 4) {
            invalidChoices++;
            printf("\nInvalid Choice (%d/3).\n", invalidChoices);
            if (invalidChoices >= 3) {
                printf("Three invalid main-menu choices. Staying in Main Menu.\n");
                invalidChoices = 0;
            }
            continue;
        }
        switch(choice)
        {
            case 1:
            {
                loggedInPassengerId =
                    registerUser();

                if(loggedInPassengerId
                   != FAILURE)
                {
                    userMenu(
                    loggedInPassengerId);
                }

                break;
            }

            case 2:
            {
                loggedInPassengerId =
                    loginUser();

                if(loggedInPassengerId
                   != FAILURE)
                {
                    userMenu(
                    loggedInPassengerId);
                }

                break;
            }

            case 3:
            {
                if (adminLogin() == SUCCESS) adminMenu();
                break;
            }

            case 4:
            {
                verifyPNRStatus();
                break;
            }

            case 0:
            {
                cleanupSystem();
                return 0;
            }

            default: break;
        }
    }

    return 0;
}