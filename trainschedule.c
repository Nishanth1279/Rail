#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trainschedule.h"
#include "train.h"
#include "utils.h"
#include "availability.h"

#define SCHEDULE_FILE "trainschedule.txt"
#define SCHEDULE_TEMP_FILE "trainschedule.txt.tmp"

TrainSchedule *scheduleHead = NULL;

/*=========================================================
    PRIVATE FUNCTION PROTOTYPES
=========================================================*/

static int trainExists(
    int trainNumber);

static int validateJourneyDate(
    const char *date);

static int scheduleExists(
    int trainNumber,
    const char *journeyDate);

static void updateJourneyDate(
    char *journeyDate,
    int delayMinutes,
    const char *sourceDepartureTime);

/*=========================================================
    INITIALIZATION
=========================================================*/

void initializeTrainScheduleModule(void)
{
    FILE *fp;

    fp = fopen(
        SCHEDULE_FILE,
        "a");

    if(fp == NULL)
    {
        printf(
        "\nUnable To Create Schedule File\n");

        return;
    }

    fclose(fp);

    loadTrainScheduleData();
}

/*=========================================================
    VALIDATION FUNCTIONS
=========================================================*/

static int trainExists(
    int trainNumber)
{
    Train *current =
        trainHead;

    while(current != NULL)
    {
        if(current->trainNumber ==
           trainNumber)
        {
            return SUCCESS;
        }

        current =
            current->next;
    }

    return FAILURE;
}

static int validateJourneyDate(const char *date)
{
    int dd, mm, yyyy;
    char extra;
    int daysInMonth;
    if (date == NULL || strlen(date) != 10 || date[2] != ':' || date[5] != ':' ||
        sscanf(date, "%2d:%2d:%4d%c", &dd, &mm, &yyyy, &extra) != 3)
        return FAILURE;
    for (int i = 0; i < 10; ++i)
        if (i != 2 && i != 5 && (date[i] < '0' || date[i] > '9')) return FAILURE;
    if (yyyy < 1900 || yyyy > 9999 || mm < 1 || mm > 12) return FAILURE;
    static const int monthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    daysInMonth = monthDays[mm - 1];
    if (mm == 2 && ((yyyy % 400 == 0) || (yyyy % 4 == 0 && yyyy % 100 != 0))) daysInMonth = 29;
    if (dd < 1 || dd > daysInMonth) return FAILURE;
    return SUCCESS;
}

static int scheduleExists(
    int trainNumber,
    const char *journeyDate)
{
    TrainSchedule *current =
        scheduleHead;

    while(current != NULL)
    {
        if(current->trainNumber ==
           trainNumber
           &&
           strcmp(
           current->journeyDate,
           journeyDate) == 0)
        {
            return SUCCESS;
        }

        current =
            current->next;
    }

    return FAILURE;
}

static void updateJourneyDate(
    char *journeyDate,
    int delayMinutes,
    const char *sourceDepartureTime)
{
    int dd;
    int mm;
    int yyyy;

    int hh;
    int min;

    sscanf(
        journeyDate,
        "%d:%d:%d",
        &dd,
        &mm,
        &yyyy);

    sscanf(
        sourceDepartureTime,
        "%d:%d",
        &hh,
        &min);

    int totalMinutes =
        (hh * 60) +
        min +
        delayMinutes;

    int extraDays =
        totalMinutes / 1440;

    dd += extraDays;

    sprintf(
        journeyDate,
        "%02d:%02d:%04d",
        dd,
        mm,
        yyyy);
}   

/*=========================================================
    LOAD DATA
=========================================================*/

void loadTrainScheduleData(void)
{
    FILE *fp = fopen(SCHEDULE_FILE, "r");
    char line[128];

    if (fp == NULL)
        return;

    /* Avoid duplicate nodes if this function is called more than once. */
    while (scheduleHead != NULL)
    {
        TrainSchedule *next = scheduleHead->next;
        free(scheduleHead);
        scheduleHead = next;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "SCHEDULE_START") != 0)
            continue;

        TrainSchedule record;
        memset(&record, 0, sizeof(record));

        int valid = 1;

        if (fgets(line, sizeof(line), fp) == NULL) break;
        line[strcspn(line, "\r\n")] = '\0';
        char *endptr;
        long trainNo = strtol(line, &endptr, 10);
        if (*line == '\0' || *endptr != '\0' || trainNo <= 0)
            valid = 0;
        record.trainNumber = (int)trainNo;

        if (fgets(line, sizeof(line), fp) == NULL) break;
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) >= sizeof(record.journeyDate) || line[0] == '\0')
            valid = 0;
        else
            strcpy(record.journeyDate, line);

        if (fgets(line, sizeof(line), fp) == NULL) break;
        line[strcspn(line, "\r\n")] = '\0';
        long status = strtol(line, &endptr, 10);
        if (*line == '\0' || *endptr != '\0' ||
            status < TRAIN_RUNNING || status > TRAIN_SPECIAL)
            valid = 0;
        record.status = (int)status;

        if (fgets(line, sizeof(line), fp) == NULL) break;
        line[strcspn(line, "\r\n")] = '\0';
        long delay = strtol(line, &endptr, 10);
        if (*line == '\0' || *endptr != '\0' || delay < 0)
            valid = 0;
        record.delayMinutes = (int)delay;

        long endMarkerPos = ftell(fp);
        if (fgets(line, sizeof(line), fp) == NULL) break;
        long linePos = ftell(fp) - (long)strlen(line);
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "SCHEDULE_END") != 0)
        {
            printf("\nWARNING: Invalid schedule record skipped.\n");

            /* If the next record's start marker was read as the end marker,
               rewind to it so the next loop can load that record. */
            if (strcmp(line, "SCHEDULE_START") == 0)
            {
                fseek(fp, linePos, SEEK_SET);
            }
            else
            {
                while (fgets(line, sizeof(line), fp) != NULL)
                {
                    long pos = ftell(fp) - (long)strlen(line);
                    line[strcspn(line, "\r\n")] = '\0';
                    if (strcmp(line, "SCHEDULE_START") == 0)
                    {
                        fseek(fp, pos, SEEK_SET);
                        break;
                    }
                }
            }
            (void)endMarkerPos;
            continue;
        }

        if (!validateJourneyDate(record.journeyDate) || !trainExists(record.trainNumber) || record.delayMinutes > 43200)
            valid = 0;
        if (!valid)
        {
            printf("\nWARNING: Invalid schedule record skipped.\n");
            continue;
        }

        TrainSchedule *existing = scheduleHead;
        int duplicate = 0;
        while (existing != NULL)
        {
            if (existing->trainNumber == record.trainNumber &&
                strcmp(existing->journeyDate, record.journeyDate) == 0)
            {
                duplicate = 1;
                break;
            }
            existing = existing->next;
        }
        if (duplicate)
            continue;

        TrainSchedule *node = malloc(sizeof(*node));
        if (node == NULL)
        {
            printf("\nMemory allocation failed while loading schedules.\n");
            fclose(fp);
            return;
        }

        *node = record;
        node->prev = NULL;
        node->next = scheduleHead;
        if (scheduleHead != NULL)
            scheduleHead->prev = node;
        scheduleHead = node;
    }

    fclose(fp);
}

/*=========================================================
    SAVE DATA
=========================================================*/

void saveTrainScheduleData(void)
{
    FILE *fp = fopen(SCHEDULE_TEMP_FILE, "w");

    if (fp == NULL)
    {
        printf("\nUnable to open temporary schedule file for writing.\n");
        return;
    }

    TrainSchedule *current = scheduleHead;

    while (current != NULL)
    {
        if (fprintf(fp, "SCHEDULE_START\n%d\n%s\n%d\n%d\nSCHEDULE_END\n",
                    current->trainNumber,
                    current->journeyDate,
                    current->status,
                    current->delayMinutes) < 0)
        {
            fclose(fp);
            remove(SCHEDULE_TEMP_FILE);
            printf("\nError writing schedule data; original file preserved.\n");
            return;
        }

        current = current->next;
    }

    if (fflush(fp) != 0 || ferror(fp))
    {
        fclose(fp);
        remove(SCHEDULE_TEMP_FILE);
        printf("\nError flushing schedule data; original file preserved.\n");
        return;
    }

    if (fclose(fp) != 0)
    {
        remove(SCHEDULE_TEMP_FILE);
        printf("\nError closing schedule file; original file preserved.\n");
        return;
    }

    if (rename(SCHEDULE_TEMP_FILE, SCHEDULE_FILE) != 0)
    {
        remove(SCHEDULE_TEMP_FILE);
        printf("\nCould not replace %s with updated schedule data.\n",
               SCHEDULE_FILE);
        return;
    }
}

/*=========================================================
    ADD SCHEDULE
=========================================================*/

void addTrainSchedule(void)
{
    TrainSchedule *newNode;

    char journeyDate[11];

    newNode =
    (TrainSchedule *)malloc(
    sizeof(TrainSchedule));

    if(newNode == NULL)
    {
        printf(
        "\nMemory Allocation Failed\n");

        return;
    }

    while(1)
    {
        newNode->trainNumber =
        getIntegerInput(
        "\nTrain number: ");

        if(trainExists(
           newNode->trainNumber))
        {
            break;
        }

        printf(
        "Train Not Found.\n");
    }

    while(1)
    {
        getStringInput(
        "Journey date DD:MM:YYYY: ",
        journeyDate,
        11);

        if(validateJourneyDate(
           journeyDate))
        {
            break;
        }

        printf(
        "Invalid Date.\n");
    }

    if(scheduleExists(
       newNode->trainNumber,
       journeyDate))
    {
        printf(
        "\nSchedule Already Exists\n");

        free(newNode);

        return;
    }

    strcpy(
    newNode->journeyDate,
    journeyDate);

    printf("\n1. Running");
printf("\n2. Special");

while (1) {
    int type = getIntegerInput("\nSchedule type (1=Running, 2=Special): ");
    if (type == 1 || type == 2) { newNode->status = type; break; }
    printf("Choose 1 or 2.\n");
}

if(newNode->status == 2)
{
    newNode->status =
        TRAIN_SPECIAL;
}
else
{
    newNode->status =
        TRAIN_RUNNING;
}

newNode->delayMinutes = 0;
    
    newNode->prev = NULL;

    newNode->next =
        scheduleHead;

    if(scheduleHead != NULL)
    {
        scheduleHead->prev =
            newNode;
    }

    scheduleHead =newNode;
        
    printf("\nSchedule Added Successfully\n");

    saveTrainScheduleData();

Train *train = findTrain(newNode->trainNumber);

if (train != NULL)
{
    createAvailabilityRecord(train, newNode->journeyDate);
}

}

/*=========================================================
    VIEW SCHEDULES
=========================================================*/

void viewTrainSchedules(void)
{
    TrainSchedule *current =
        scheduleHead;

    if(current == NULL)
    {
        printf(
        "\nNo Schedule Available\n");

        return;
    }

    while(current != NULL)
    {
        printf(
        "\n================================");

        printf(
        "\nTrain Number : %d",
        current->trainNumber);

        printf(
        "\nJourney Date : %s",
        current->journeyDate);

        printf("\nStatus : ");

switch(current->status)
{
    case TRAIN_RUNNING:
        printf("Running");
        break;

    case TRAIN_RESCHEDULED:
        printf("Rescheduled");
        break;

    case TRAIN_CANCELLED:
        printf("Cancelled");
        break;

    case TRAIN_SPECIAL:
        printf("Special");
        break;

    default:
        printf("Unknown");
}

        printf(
        "\nDelay Minutes : %d",
        current->delayMinutes);

        printf(
        "\n================================\n");

        current =
            current->next;
    }
}

/*=========================================================
    SEARCH SCHEDULE
=========================================================*/

void searchTrainSchedule(void)
{
    int trainNumber;

    char date[11];

    TrainSchedule *current =
        scheduleHead;

    trainNumber =
        getIntegerInput(
        "\nTrain number: ");

    while (1) {
        getStringInput("Journey date DD:MM:YYYY: ", date, sizeof date);
        if (isValidJourneyDate(date)) break;
        printf("Invalid calendar date. Retry.\n");
    }

    while(current != NULL)
    {
        if(current->trainNumber ==
           trainNumber
           &&
           strcmp(
           current->journeyDate,
           date) == 0)
        {
            printf(
            "\nTrain Number : %d",
            current->trainNumber);

            printf(
            "\nJourney Date : %s",
            current->journeyDate);

            printf(
            "\nStatus : %d",
            current->status);

            printf(
            "\nDelay Minutes : %d\n",
            current->delayMinutes);

            return;
        }

        current =
            current->next;
    }

    printf(
    "\nSchedule Not Found\n");
}

/*=========================================================
        UPDATE SCHEDULE
=========================================================*/

void updateTrainSchedule(void)
{
    int trainNumber;
    int choice;

    char date[11];

    TrainSchedule *current =
        scheduleHead;

    trainNumber =
        getIntegerInput(
        "\nTrain number: ");

    while (1) {
        getStringInput("Journey date DD:MM:YYYY: ", date, sizeof date);
        if (isValidJourneyDate(date)) break;
        printf("Invalid calendar date. Retry.\n");
    }

    while(current != NULL)
    {
        if(current->trainNumber ==
           trainNumber &&
           strcmp(
           current->journeyDate,
           date) == 0)
        {
            printf(
            "\nCurrent Status : %d",
            current->status);

            printf(
            "\nCurrent Delay : %d Minutes\n",
            current->delayMinutes);

            printf(
            "\n1. Reschedule Train");

            printf(
            "\n2. Cancel Train");

            printf(
            "\n3. Mark As Special");

            printf(
            "\n4. Restore Running");

            while (1) {
                choice = getIntegerInput("\nUpdate choice (1-4): ");
                if (choice >= 1 && choice <= 4) break;
                printf("Choose 1, 2, 3 or 4.\n");
            }

            if (choice < 1 || choice > 4) { printf("Invalid schedule update choice.\n"); return; }
            char confirm[8];
            if (choice == 2) {
                getStringInput("Confirm train cancellation? (Y/N): ", confirm, sizeof confirm);
                if (strcmp(confirm, "Y") != 0 && strcmp(confirm, "y") != 0) { printf("Cancellation aborted.\n"); return; }
            }
            switch(choice)
            {
                case 1:
                {
                    int delayHours;

                    do {
                        delayHours = getIntegerInput("\nDelay hours (0-720, e.g., 2): ");
                        if (delayHours < 0 || delayHours > 720) printf("Delay must be between 0 and 720 hours.\n");
                    } while (delayHours < 0 || delayHours > 720);

                    current->status =
                        TRAIN_RESCHEDULED;

                    current->delayMinutes =
                        delayHours * 60;

                    printf(
                    "\nUpdated Delay : %d Minutes",
                    current->delayMinutes);

                    break;
                }

                case 2:
                {
                    current->status =
                        TRAIN_CANCELLED;

                    current->delayMinutes =
                        0;

                    printf(
                    "\nTrain Cancelled");

                    break;
                }

                case 3:
                {
                    current->status =
                        TRAIN_SPECIAL;

                    current->delayMinutes =
                        0;

                    printf(
                    "\nMarked As Special Train");

                    break;
                }

                case 4:
                {
                    current->status =
                        TRAIN_RUNNING;

                    current->delayMinutes =
                        0;

                    printf(
                    "\nTrain Restored To Running");

                    break;
                }

                default:
                {
                    printf(
                    "\nInvalid Choice\n");

                    return;
                }
            }

            saveTrainScheduleData();

            printf(
            "\nSchedule Updated Successfully\n");

            return;
        }

        current =
            current->next;
    }

    printf(
    "\nNo Schedule Exists For This Train On This Date\n");
}

/*=========================================================
    DELETE SCHEDULE
=========================================================*/

void deleteTrainSchedule(void)
{
    int trainNumber;

    char date[11];

    TrainSchedule *current =
        scheduleHead;

    trainNumber =
        getIntegerInput(
        "\nTrain number: ");

    while (1) {
        getStringInput("Journey date DD:MM:YYYY: ", date, sizeof date);
        if (isValidJourneyDate(date)) break;
        printf("Invalid calendar date. Retry.\n");
    }

    while(current != NULL)
    {
        if(current->trainNumber ==
           trainNumber
           &&
           strcmp(
           current->journeyDate,
           date) == 0)
        {
            char confirm[8];
            getStringInput("Confirm schedule deletion? (Y/N): ", confirm, sizeof confirm);
            if (strcmp(confirm, "Y") != 0 && strcmp(confirm, "y") != 0) { printf("Deletion aborted.\n"); return; }
            if(current->prev == NULL)
            {
                scheduleHead =
                    current->next;

                if(scheduleHead != NULL)
                {
                    scheduleHead->prev =
                        NULL;
                }
            }
            else
            {
                current->prev->next =
                    current->next;

                if(current->next != NULL)
                {
                    current->next->prev =
                        current->prev;
                }
            }

            free(current);

            saveTrainScheduleData();

            printf(
            "\nSchedule Deleted Successfully\n");

            return;
        }

        current =
            current->next;
    }

    printf(
    "\nSchedule Not Found\n");
}