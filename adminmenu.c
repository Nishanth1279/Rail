#include "adminmenu.h"

#include "utils.h"

#include "train.h"
#include "passenger.h"
#include "trainschedule.h"
#include "availability.h"
#include "reserved_booking.h"

void adminMenu(void)
{
    int choice;

    while(1)
    {
        printf(
        "\n===== ADMIN MENU =====");

        printf(
        "\n1. Add Train");

        printf(
        "\n2. View Trains");

        printf(
        "\n3. Search Train");

        printf(
        "\n4. Update Train");

        printf(
        "\n5. Delete Train");

        printf(
        "\n6. View Passengers");

        printf(
        "\n7. Search Passenger");

        printf(
        "\n8. Delete Passenger");

        printf(
        "\n9. Add Train Schedule");

        printf(
        "\n10. View Train Schedules");

        printf(
        "\n11. Search Train Schedule");

        printf(
        "\n12. Update Train Schedule");

        printf(
        "\n13. Delete Train Schedule");

        printf("\n14. View Availability");

        printf("\n15. Logout");
        choice =getIntegerInput("\nEnter choice (1-15; 15 to logout): ");

        switch(choice)
        {
            case 1:
            {
                addTrain();
                break;
            }

            case 2:
            {
                viewTrainDetails();
                break;
            }

            case 3:
            {
                searchTrain();
                break;
            }

            case 4:
            {
                updateTrain();
                break;
            }

            case 5:
            {
                deleteTrain();
                break;
            }

            case 6:
            {
                viewPassengers();
                break;
            }

            case 7:
            {
                searchPassenger();
                break;
            }

            case 8:
            {
                deletePassenger();
                break;
            }

            case 9:
            {
                addTrainSchedule();
                break;
            }

            case 10:
            {
                viewTrainSchedules();
                break;
            }

            case 11:
            {
                searchTrainSchedule();
                break;
            }

            case 12:
            {
                updateTrainSchedule();
                break;
            }

            case 13:
            {
                deleteTrainSchedule();
                break;
            }

            case 14:
            {
                checkAvailability();
                break;
            }
            
            case 15:
            {
                return;
            }

            default:
            {
                printf(
                "\nInvalid Choice\n");
            }
        }
    }
}