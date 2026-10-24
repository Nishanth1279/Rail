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

int loggedInPassengerId = FAILURE;

int main(void)
{
    int choice;

    initializeSystem();

    initializeTrainModule();

    initializeTrainScheduleModule();
    
    initializeAvailabilityModule();
    
    initializeBookingModule();

    initializePassengerModule();

    initializeUserModule();

    while(1)
    {
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

            default:
            {
                printf(
                "\nInvalid Choice\n");
            }
        }
    }

    return 0;
}