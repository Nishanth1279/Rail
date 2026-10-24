#include "train.h"
#include "utils.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>

#define TRAIN_FILE "trains.txt"

Train *trainHead = NULL;

/*=========================================================
    PRIVATE FUNCTION PROTOTYPES
=========================================================*/

static int validateStationName(const char *name);
static int trainNameExists(const char *trainName);
static int validateTimeFormat(const char *time);
static int convertToMinutes(const char *time);
static void updateCoachDetails(Train *train);
static void addMinutesToTime(char *time,int delayMinutes);
static void getValidStationName(
    const char *message,
    char *buffer);

static void getValidTime(
    const char *message,
    char *buffer);

static void manageRouteDetails(
    Train *train);

static void manageCoachDetails(
    Train *train);

static void manageCapacity(
    Train *train);

static void manageFareDetails(
    Train *train);

/*=========================================================
    TRAIN FILE INITIALIZATION
=========================================================*/

void initializeTrainModule(void)
{
    FILE *fp;

    fp = fopen(TRAIN_FILE, "a");

    if(fp == NULL)
    {
        printf(
        "\nUnable To Create Train File.\n");

        return;
    }

    fclose(fp);

    loadTrainData();
}

Train *findTrain(
    int trainNumber)
{
    Train *current =
        trainHead;

    while(current != NULL)
    {
        if(current->trainNumber ==
           trainNumber)
        {
            return current;
        }

        current =
            current->next;
    }

    return NULL;
}

static void addMinutesToTime(
    char *time,
    int delayMinutes)
{
    int hh;
    int mm;

    sscanf(
        time,
        "%d:%d",
        &hh,
        &mm);

    int totalMinutes =
        (hh * 60) +
        mm +
        delayMinutes;

    totalMinutes =
        totalMinutes % 1440;

    hh =
        totalMinutes / 60;

    mm =
        totalMinutes % 60;

    sprintf(
        time,
        "%02d:%02d",
        hh,
        mm);
}

void applyTrainDelay(
    Train *train,
    int delayMinutes)
{
    for(int i = 0;
        i < train->totalStations;
        i++)
    {
        if(strcmp(
           train->stations[i]
           .arrivalTime,
           "00:00") != 0)
        {
            addMinutesToTime(
                train->stations[i]
                .arrivalTime,
                delayMinutes);
        }

        if(strcmp(
           train->stations[i]
           .departureTime,
           "00:00") != 0)
        {
            addMinutesToTime(
                train->stations[i]
                .departureTime,
                delayMinutes);
        }
    }
}

static int allowedClassName(const char *name)
{
    static const char *allowed[] = {"SL", "1A", "2A", "3A", "3E", "2S", "EC", "CC"};
    for (size_t i = 0; i < sizeof allowed / sizeof allowed[0]; ++i)
        if (strcmp(name, allowed[i]) == 0) return SUCCESS;
    return FAILURE;
}

static int normalizeStoredTime(char *time)
{
    size_t n = strlen(time);
    int hh, mm;
    if (n == 4 && time[1] == ':') {
        memmove(time + 1, time, n + 1);
        time[0] = '0';
        n = 5;
    }
    if (n != 5 || time[2] != ':' ||
        time[0] < '0' || time[0] > '9' || time[1] < '0' || time[1] > '9' ||
        time[3] < '0' || time[3] > '9' || time[4] < '0' || time[4] > '9') return FAILURE;
    if (sscanf(time, "%2d:%2d", &hh, &mm) != 2 || hh > 23 || mm > 59) return FAILURE;
    return SUCCESS;
}

void loadTrainData(void)
{
    FILE *fp;

    fp = fopen(TRAIN_FILE,"r");

    if(fp == NULL)
    {
        return;
    }

    char line[200];

    while(fgets(line,sizeof(line),fp))
    {
        line[strcspn(line,"\n")] = '\0';

        if(strcmp(line,"TRAIN_START") == 0)
        {
            Train *newTrain =
                (Train *)malloc(sizeof(Train));

            if(newTrain == NULL)
            {
                fclose(fp);
                return;
            }

            memset(newTrain,
                   0,
                   sizeof(Train));

            fgets(line,sizeof(line),fp);
            newTrain->trainNumber =
                atoi(line);

            fgets(newTrain->trainName,
                  MAX_NAME_LEN,
                  fp);

            newTrain->trainName[
                strcspn(
                newTrain->trainName,
                "\n")] = '\0';

            fgets(line,sizeof(line),fp);

            newTrain->totalStations = atoi(line);
            if (newTrain->trainNumber <= 0 || newTrain->totalStations < 2 || newTrain->totalStations > MAX_STATIONS) {
                printf("\nWARNING: Invalid train record skipped.\n");
                free(newTrain);
                while (fgets(line, sizeof line, fp)) { line[strcspn(line, "\r\n")] = '\0'; if (strcmp(line, "TRAIN_END") == 0) break; }
                continue;
            }

            for(int i=0;
                i<newTrain->totalStations;
                i++)
            {
                fgets(
                newTrain->stations[i]
                .stationName,
                MAX_STATION_LEN,
                fp);

                newTrain->stations[i]
                .stationName[
                strcspn(
                newTrain->stations[i]
                .stationName,
                "\n")] = '\0';

                fgets(
    newTrain->stations[i].arrivalTime,
    sizeof(
        newTrain->stations[i]
        .arrivalTime),
    fp);

newTrain->stations[i]
.arrivalTime[
strcspn(
newTrain->stations[i]
.arrivalTime,
"\n")] = '\0';


fgets(
    newTrain->stations[i].departureTime,
    sizeof(
        newTrain->stations[i]
        .departureTime),
    fp);

newTrain->stations[i]
.departureTime[
strcspn(
newTrain->stations[i]
.departureTime,
"\n")] = '\0';

                fgets(line,
                      sizeof(line),
                      fp);

                newTrain->stations[i]
                .distanceFromSource =
                atoi(line);
            }

            fgets(line,sizeof(line),fp);

            newTrain->classCount = atoi(line);
            if (newTrain->classCount < 1 || newTrain->classCount > 8) {
                printf("\nWARNING: Invalid train class count; record skipped.\n");
                free(newTrain);
                while (fgets(line, sizeof line, fp)) { line[strcspn(line, "\r\n")] = '\0'; if (strcmp(line, "TRAIN_END") == 0) break; }
                continue;
            }

            for(int i=0;
                i<newTrain->classCount;
                i++)
            {
                fgets(
                newTrain->coaches[i]
                .className,
                MAX_CLASS_LEN,
                fp);

                newTrain->coaches[i]
                .className[
                strcspn(
                newTrain->coaches[i]
                .className,
                "\n")] = '\0';

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .noOfCoaches =
                atoi(line);

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .seatsPerCoach =
                atoi(line);

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .totalSeats =
                atoi(line);

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .availableSeats =
                atoi(line);

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .racCapacity =
                atoi(line);

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .availableRAC =
                atoi(line);

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .wlCapacity =
                atoi(line);

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .availableWL =
                atoi(line);

                fgets(line,sizeof(line),fp);
                newTrain->coaches[i]
                .farePerKM =
                (float)atof(line);
            }

            while(fgets(line,
                        sizeof(line),
                        fp))
            {
                line[strcspn(line,"\n")] = '\0';

                if(strcmp(line,
                          "TRAIN_END") == 0)
                {
                    break;
                }
            }

            int recordValid = newTrain->trainNumber > 0 && newTrain->totalStations >= 2 && newTrain->totalStations <= MAX_STATIONS && newTrain->classCount >= 1 && newTrain->classCount <= 8;
            for (int i = 0; i < newTrain->totalStations; ++i) {
                if (newTrain->stations[i].stationName[0] == '\0' ||
                    !normalizeStoredTime(newTrain->stations[i].arrivalTime) ||
                    !normalizeStoredTime(newTrain->stations[i].departureTime)) recordValid = 0;
                if (i > 0 && newTrain->stations[i].distanceFromSource <= newTrain->stations[i-1].distanceFromSource) recordValid = 0;
                for (int j = 0; j < i; ++j)
                    if (strcmp(newTrain->stations[i].stationName, newTrain->stations[j].stationName) == 0) recordValid = 0;
            }
            for (int i = 0; i < newTrain->classCount; ++i) {
                if (!allowedClassName(newTrain->coaches[i].className) || newTrain->coaches[i].noOfCoaches <= 0 || newTrain->coaches[i].seatsPerCoach <= 0 || newTrain->coaches[i].totalSeats < 0 || newTrain->coaches[i].availableSeats < 0 || newTrain->coaches[i].availableSeats > newTrain->coaches[i].totalSeats || newTrain->coaches[i].racCapacity < 0 || newTrain->coaches[i].availableRAC < 0 || newTrain->coaches[i].availableRAC > newTrain->coaches[i].racCapacity || newTrain->coaches[i].wlCapacity < 0 || newTrain->coaches[i].availableWL < 0 || newTrain->coaches[i].availableWL > newTrain->coaches[i].wlCapacity || newTrain->coaches[i].farePerKM <= 0) recordValid = 0;
                for (int j = 0; j < i; ++j) if (strcmp(newTrain->coaches[i].className, newTrain->coaches[j].className) == 0) recordValid = 0;
            }
            if (!recordValid) {
                printf("\nWARNING: Invalid train data found; record skipped.\n");
                free(newTrain);
                continue;
            }
            if (findTrain(newTrain->trainNumber) != NULL || trainNameExists(newTrain->trainName)) {
                printf("\nWARNING: Duplicate train number/name skipped.\n");
                free(newTrain);
                continue;
            }

            newTrain->prev = NULL;
            newTrain->next = trainHead;

            if(trainHead != NULL)
            {
                trainHead->prev =
                    newTrain;
            }

            trainHead = newTrain;
            printf(
"\nLoaded Train : %d - %s",
newTrain->trainNumber,
newTrain->trainName);
        }
    }

    fclose(fp);
}

/*=========================================================
    SAVE TRAIN DATA
=========================================================*/

void saveTrainData(void)
{
    printf("\nsaveTrainData called\n");

printf("\ntrainHead = %p\n",
       (void *)trainHead);
    FILE *fp;

    fp = fopen(TRAIN_FILE, "w");

    if(fp == NULL)
    {
        printf("\nUnable To Save Train Data\n");
        return;
    }
    printf("\nsaveTrainData() Called\n");
    Train *current = trainHead;

    while(current != NULL)
    {
        fprintf(fp,"TRAIN_START\n");
        printf(
"\nWriting Train : %d %s",
current->trainNumber,
current->trainName);

        fprintf(fp,"%d\n",
                current->trainNumber);

        fprintf(fp,"%s\n",
                current->trainName);

        fprintf(fp,"%d\n",
                current->totalStations);

        for(int i=0;
            i<current->totalStations;
            i++)
        {
            fprintf(fp,"%s\n",
                    current->stations[i].stationName);

            fprintf(fp,"%s\n",
                    current->stations[i].arrivalTime);

            fprintf(fp,"%s\n",
                    current->stations[i].departureTime);

            fprintf(fp,"%d\n",
                    current->stations[i].distanceFromSource);
        }

        fprintf(fp,"%d\n",
                current->classCount);

        for(int i=0;
            i<current->classCount;
            i++)
        {
            fprintf(fp,"%s\n",
                    current->coaches[i].className);

            fprintf(fp,"%d\n",
                    current->coaches[i].noOfCoaches);

            fprintf(fp,"%d\n",
                    current->coaches[i].seatsPerCoach);

            fprintf(fp,"%d\n",
                    current->coaches[i].totalSeats);

            fprintf(fp,"%d\n",
                    current->coaches[i].availableSeats);

            fprintf(fp,"%d\n",
                    current->coaches[i].racCapacity);

            fprintf(fp,"%d\n",
                    current->coaches[i].availableRAC);

            fprintf(fp,"%d\n",
                    current->coaches[i].wlCapacity);

            fprintf(fp,"%d\n",
                    current->coaches[i].availableWL);

            fprintf(fp,"%.2f\n",
                    current->coaches[i].farePerKM);
        }

        fprintf(fp,"TRAIN_END\n");

        current = current->next;
    }
    fflush(fp);

printf(
"\nTrain Data Saved Successfully\n");

    fclose(fp);
}
/*=========================================================
    VALIDATION FUNCTIONS
=========================================================*/

static int validateStationName(
    const char *name)
{
    int i;

    if(strlen(name) == 0)
    {
        return FAILURE;
    }

    for(i = 0;
        name[i] != '\0';
        i++)
    {
        if(
           (name[i] >= 'A' &&
            name[i] <= 'Z')
           ||
           (name[i] >= 'a' &&
            name[i] <= 'z')
           ||
           (name[i] == ' ') || name[i] == '\'' || name[i] == '-'
          )
        {
            continue;
        }

        return FAILURE;
    }

    return SUCCESS;
}

static int validateTimeFormat(const char *time)
{
    int hh, mm;
    char extra;
    if (time == NULL || strlen(time) != 5 || time[2] != ':' ||
        time[0] < '0' || time[0] > '9' || time[1] < '0' || time[1] > '9' ||
        time[3] < '0' || time[3] > '9' || time[4] < '0' || time[4] > '9')
        return FAILURE;
    if (sscanf(time, "%2d:%2d%c", &hh, &mm, &extra) != 2) return FAILURE;
    return (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59) ? SUCCESS : FAILURE;
}

static int convertToMinutes(
    const char *time)
{
    int hh;
    int mm;

    sscanf(
           time,
           "%d:%d",
           &hh,
           &mm);

    return (hh * 60) + mm;
}

static void getValidStationName(
    const char *message,
    char *buffer)
{
    while(1)
    {
        getStringInput(
        message,
        buffer,
        MAX_STATION_LEN);

        if(
           validateStationName(
           buffer))
        {
            break;
        }

        printf(
        "Invalid Station Name.\n");
    }
}

static void getValidTime(
    const char *message,
    char *buffer)
{
    while(1)
    {
        getStringInput(
        message,
        buffer,
        6);

        if(
           validateTimeFormat(
           buffer))
        {
            break;
        }

        printf(
        "Invalid Time Format. "
        "Use HH:MM\n");
    }
}

/*=========================================================
    ROUTE DETAILS
=========================================================*/

static void manageRouteDetails(
    Train *train)
{
    char source[MAX_STATION_LEN];
    char destination[MAX_STATION_LEN];

    int previousDistance = 0;
    int previousTime = 0;

    getValidStationName("Source station: ", source);
    while (1) {
        getValidStationName("Destination station: ", destination);
        if (strcmp(source, destination) != 0) break;
        printf("Source and destination must differ.\n");
    }

    int intermediateCount;
    while (1) {
        intermediateCount = getIntegerInput("Intermediate stations (0-18): ");
        if (intermediateCount >= 0 && intermediateCount <= MAX_STATIONS - 2) break;
        printf("Enter a value from 0 to %d.\n", MAX_STATIONS - 2);
    }
    train->totalStations = intermediateCount + 2;

    strcpy(
        train->stations[0].stationName,
        source);

    strcpy(
        train->stations[0].arrivalTime,
        "00:00");

    printf(
    "\nSOURCE DETAILS\n");

    getValidTime(
        "Departure time HH:MM: ",
        train->stations[0]
        .departureTime);

    train->stations[0]
        .distanceFromSource = 0;

    previousTime =
        convertToMinutes(
        train->stations[0]
        .departureTime);

    for(int i = 1;
        i <= intermediateCount;
        i++)
    {
        printf(
        "\nINTERMEDIATE STATION %d\n",
        i);

        while (1) {
            int duplicate = 0;
            getValidStationName("Station name: ", train->stations[i].stationName);
            for (int j = 0; j < i; ++j)
                if (strcmp(train->stations[j].stationName, train->stations[i].stationName) == 0) duplicate = 1;
            if (!duplicate && strcmp(train->stations[i].stationName, destination) != 0) break;
            printf("Route stations must be unique. Enter a different station.\n");
        }

        while(1)
        {
            getValidTime(
                "Arrival time HH:MM: ",
                train->stations[i]
                .arrivalTime);

            int arrivalTime =
                convertToMinutes(
                train->stations[i]
                .arrivalTime);

            while(arrivalTime <
                  previousTime)
            {
                arrivalTime += 1440;
            }

            if(arrivalTime >
               previousTime)
            {
                previousTime =
                    arrivalTime;

                break;
            }

            printf(
            "Arrival Time Must "
            "Be After Previous "
            "Station Departure.\n");
        }

        while(1)
        {
            getValidTime(
                "Departure time HH:MM: ",
                train->stations[i]
                .departureTime);

            int arrivalTime =
                convertToMinutes(
                train->stations[i]
                .arrivalTime);

            int departureTime =
                convertToMinutes(
                train->stations[i]
                .departureTime);

            while(departureTime <
                  arrivalTime)
            {
                departureTime += 1440;
            }

            if(departureTime >=
                arrivalTime + 2)
            {
                previousTime =
                    departureTime;

                break;
            }

            printf(
            "Departure Must Be "
            "At Least 2 Minutes "
            "After Arrival.\n");
        }

        while(1)
        {
            int distance =
                getIntegerInput(
                "Distance From Source : ");

            if(distance >
               previousDistance)
            {
                train->stations[i]
                .distanceFromSource =
                    distance;

                previousDistance =
                    distance;

                break;
            }

            printf(
            "Distance Must Increase.\n");
        }
    }

    int last =
        train->totalStations - 1;

    printf(
    "\nDESTINATION DETAILS\n");

    strcpy(
        train->stations[last]
        .stationName,
        destination);

    while(1)
    {
        getValidTime(
            "Arrival time HH:MM: ",
            train->stations[last]
            .arrivalTime);

        int arrivalTime =
            convertToMinutes(
            train->stations[last]
            .arrivalTime);

        while(arrivalTime <
              previousTime)
        {
            arrivalTime += 1440;
        }

        if(arrivalTime >
           previousTime)
        {
            break;
        }

        printf(
        "Arrival Time Must "
        "Be After Previous "
        "Station Departure.\n");
    }

    strcpy(
        train->stations[last]
        .departureTime,
        "00:00");

    while(1)
    {
        int distance =
            getIntegerInput(
            "Distance From Source : ");

        if(distance >
           previousDistance)
        {
            train->stations[last]
            .distanceFromSource =
            distance;

            break;
        }

        printf(
        "Distance Must Increase.\n");
    }
}
/*=========================================================
    COACH DETAILS
=========================================================*/

static void manageCoachDetails(Train *train)
{
    while (1) {
        train->classCount = getIntegerInput("Number of classes (1-8): ");
        if (train->classCount >= 1 && train->classCount <= 8) break;
        printf("Enter a class count from 1 to 8.\n");
    }

    for (int i = 0; i < train->classCount; i++) {
        int duplicate;
        printf("\nClass %d\n", i + 1);
        while (1) {
            getStringInput("Class Name [SL/1A/2A/3A/3E/2S/EC/CC]: ", train->coaches[i].className, MAX_CLASS_LEN);
            duplicate = 0;
            if (!allowedClassName(train->coaches[i].className)) {
                printf("Unsupported class. Choose only SL, 1A, 2A, 3A, 3E, 2S, EC or CC.\n");
                continue;
            }
            for (int j = 0; j < i; ++j)
                if (strcmp(train->coaches[j].className, train->coaches[i].className) == 0) duplicate = 1;
            if (!duplicate) break;
            printf("This class was already entered. Choose another class.\n");
        }
        while (1) {
            train->coaches[i].noOfCoaches = getIntegerInput("Number of coaches (positive integer): ");
            if (train->coaches[i].noOfCoaches > 0 && train->coaches[i].noOfCoaches <= 100) break;
            printf("Coach count must be 1-100.\n");
        }
        while (1) {
            train->coaches[i].seatsPerCoach = getIntegerInput("Seats per coach (positive integer): ");
            if (train->coaches[i].seatsPerCoach > 0 && train->coaches[i].seatsPerCoach <= 500) break;
            printf("Seats per coach must be 1-500.\n");
        }
    }
}

/*=========================================================
    CAPACITY DETAILS
=========================================================*/

static void manageCapacity(
    Train *train)
{
    for(int i = 0;
        i < train->classCount;
        i++)
    {
        train->coaches[i]
        .totalSeats =
        train->coaches[i]
        .noOfCoaches *
        train->coaches[i]
        .seatsPerCoach;

        train->coaches[i]
        .availableSeats =
        train->coaches[i]
        .totalSeats;

        printf(
        "\n[%s] Total Seat Capacity : %d\n",
        train->coaches[i]
        .className,
        train->coaches[i]
        .totalSeats);

        while (1) {
            train->coaches[i].racCapacity = getIntegerInput("RAC capacity (0 or more): ");
            if (train->coaches[i].racCapacity >= 0 && train->coaches[i].racCapacity <= 10000) break;
            printf("RAC capacity must be 0-10000.\n");
        }

        train->coaches[i]
        .availableRAC =
        train->coaches[i]
        .racCapacity;

        while (1) {
            train->coaches[i].wlCapacity = getIntegerInput("Waiting-list capacity (0 or more): ");
            if (train->coaches[i].wlCapacity >= 0 && train->coaches[i].wlCapacity <= 100000) break;
            printf("Waiting-list capacity must be 0-100000.\n");
        }

        train->coaches[i]
        .availableWL =
        train->coaches[i]
        .wlCapacity;
    }
}

/*=========================================================
    FARE DETAILS
=========================================================*/

static void manageFareDetails(Train *train)
{
    char buffer[50];
    float fare;

    for(int i = 0;
        i < train->classCount;
        i++)
    {
        while(1)
        {
            printf(
            "\n%s Fare Details\n",
            train->coaches[i].className);

            printf(
            "Fare per km (positive number, e.g., 2.50): ");

            fgets(
                buffer,
                sizeof(buffer),
                stdin);

            char *endptr;
            errno = 0;
            fare = strtof(buffer, &endptr);
            while (*endptr && isspace((unsigned char)*endptr)) endptr++;

            if(errno == 0 && endptr != buffer && *endptr == '\0' && isfinite(fare) && fare > 0)
            {
                train->coaches[i]
                .farePerKM =
                fare;

                break;
            }

            printf(
            "Invalid Fare. "
            "Enter Positive Value.\n");
        }
    }
}

static int trainNumberExists(
    int trainNumber)
{
    Train *current =
        trainHead;

    while(current != NULL)
    {
        if(current->trainNumber
           == trainNumber)
        {
            return SUCCESS;
        }

        current =
            current->next;
    }

    return FAILURE;
}

static int trainNameExists(
    const char *trainName)
{
    Train *current =
        trainHead;

    while(current != NULL)
    {
        if(strcmp(
           current->trainName,
           trainName) == 0)
        {
            return SUCCESS;
        }

        current =
            current->next;
    }

    return FAILURE;
}
static void updateCoachDetails(
    Train *train)
{
    printf(
    "\nCurrent Classes : %d\n",
    train->classCount);

    while (1) {
        train->classCount = getIntegerInput("Total classes (1-8): ");
        if (train->classCount >= 1 && train->classCount <= 8) break;
        printf("Enter a class count from 1 to 8.\n");
    }

    for (int i = 0; i < train->classCount; i++)
    {
        int duplicate;
        printf("\nCLASS %d\n", i + 1);
        while (1) {
            getStringInput("Class Name [SL/1A/2A/3A/3E/2S/EC/CC]: ", train->coaches[i].className, MAX_CLASS_LEN);
            duplicate = !allowedClassName(train->coaches[i].className);
            for (int j = 0; j < i; ++j) if (strcmp(train->coaches[j].className, train->coaches[i].className) == 0) duplicate = 1;
            if (!duplicate) break;
            printf("Invalid or duplicate class. Choose a supported class not already entered.\n");
        }
        while (1) {
            train->coaches[i].noOfCoaches = getIntegerInput("Number of coaches: ");
            if (train->coaches[i].noOfCoaches > 0 && train->coaches[i].noOfCoaches <= 100) break;
            printf("Enter 1-100 coaches.\n");
        }
        while (1) {
            train->coaches[i].seatsPerCoach = getIntegerInput("Seats per coach: ");
            if (train->coaches[i].seatsPerCoach > 0 && train->coaches[i].seatsPerCoach <= 500) break;
            printf("Enter 1-500 seats per coach.\n");
        }

        train->coaches[i]
        .totalSeats =
        train->coaches[i]
        .noOfCoaches *
        train->coaches[i]
        .seatsPerCoach;

        train->coaches[i]
        .availableSeats =
        train->coaches[i]
        .totalSeats;

        printf(
        "\nTotal Seats : %d\n",
        train->coaches[i]
        .totalSeats);

        while (1) {
            train->coaches[i].racCapacity = getIntegerInput("RAC capacity (0 or more): ");
            if (train->coaches[i].racCapacity >= 0 && train->coaches[i].racCapacity <= 10000) break;
            printf("RAC capacity must be 0-10000.\n");
        }

        train->coaches[i]
        .availableRAC =
        train->coaches[i]
        .racCapacity;

        while (1) {
            train->coaches[i].wlCapacity = getIntegerInput("Waiting-list capacity (0 or more): ");
            if (train->coaches[i].wlCapacity >= 0 && train->coaches[i].wlCapacity <= 100000) break;
            printf("Waiting-list capacity must be 0-100000.\n");
        }

        train->coaches[i]
        .availableWL =
        train->coaches[i]
        .wlCapacity;

        train->coaches[i]
        .farePerKM =
        getFloatInput(
        "Fare per km (positive number, e.g., 2.50): ");
    }
}


/*=========================================================
    ADD TRAIN
=========================================================*/

void addTrain(void)
{
    Train *newTrain;

    newTrain =
    (Train *)malloc(
    sizeof(Train));

    if(newTrain == NULL)
    {
        printf(
        "Memory Allocation Failed\n");

        return;
    }

    while(1)
{
    newTrain->trainNumber =
        getIntegerInput(
        "\nTrain number (e.g., 1279): ");

    if (newTrain->trainNumber > 0 && !trainNumberExists(newTrain->trainNumber)) break;
    if (newTrain->trainNumber <= 0) printf("Train number must be positive.\n");
    else printf("Train Number Already Exists.\n");
}
    while(1)
{
    getStringInput(
    "Train name: ",
    newTrain->trainName,
    MAX_NAME_LEN);

    if(!trainNameExists(
        newTrain->trainName))
    {
        break;
    }

    printf(
    "Train Name Already Exists.\n");
}

    manageRouteDetails(
    newTrain);

    manageCoachDetails(
    newTrain);

    manageCapacity(
    newTrain);

    manageFareDetails(
    newTrain);

    newTrain->prev = NULL;

    newTrain->next =
    trainHead;

    if(trainHead != NULL)
    {
        trainHead->prev =
        newTrain;
    }

    trainHead = newTrain;

    saveTrainData();

    printf(
    "\nTrain Added Successfully\n");
}

/*=========================================================
    VIEW TRAINS
=========================================================*/

void viewTrainDetails(void)
{
    Train *current = trainHead;

    if(current == NULL)
    {
        printf("\nNo Train Available.\n");
        return;
    }

    while(current != NULL)
    {
        printf("\n==================================================");
        printf("\nTrain Number : %d", current->trainNumber);
        printf("\nTrain Name   : %s", current->trainName);

        printf("\n\nROUTE DETAILS");
        printf("\n--------------------------------------------------");

        for(int i = 0;
            i < current->totalStations;
            i++)
        {
            printf(
            "\nStation : %s",
            current->stations[i].stationName);

            printf(
            "\nArrival : %s",
            current->stations[i].arrivalTime);

            printf(
            "\nDeparture : %s",
            current->stations[i].departureTime);

            printf(
            "\nDistance : %d KM\n",
            current->stations[i].distanceFromSource);
        }

        printf("\nCOACH DETAILS");
        printf("\n--------------------------------------------------");

        for(int i = 0;
            i < current->classCount;
            i++)
        {
            printf(
            "\nClass             : %s",
            current->coaches[i].className);

            printf(
            "\nNo Of Coaches     : %d",
            current->coaches[i].noOfCoaches);

            printf(
            "\nSeats Per Coach   : %d",
            current->coaches[i].seatsPerCoach);

            printf(
            "\nTotal Seats       : %d",
            current->coaches[i].totalSeats);

            printf(
            "\nAvailable Seats   : %d",
            current->coaches[i].availableSeats);

            printf(
            "\nRAC Capacity      : %d",
            current->coaches[i].racCapacity);

            printf(
            "\nAvailable RAC     : %d",
            current->coaches[i].availableRAC);

            printf(
            "\nWL Capacity       : %d",
            current->coaches[i].wlCapacity);

            printf(
            "\nAvailable WL      : %d",
            current->coaches[i].availableWL);

            printf(
            "\nFare Per KM       : %.2f\n",
            current->coaches[i].farePerKM);
        }

        printf("\n==================================================\n");

        current = current->next;
    }
}
/*=========================================================
    PENDING MODULES
=========================================================*/
static void displayTrain(
    Train *train)
{
    printf(
    "\n================================");

    printf(
    "\nTrain Number : %d",
    train->trainNumber);

    printf(
    "\nTrain Name   : %s",
    train->trainName);

    printf(
    "\n\nRoute:");

    for(int i = 0;
        i < train->totalStations;
        i++)
    {
        printf(
        "\n%s (%d KM)",
        train->stations[i].stationName,
        train->stations[i].distanceFromSource);
    }

    printf(
    "\n================================\n");
}

void searchTrain(void)
{
    int choice;

    Train *current;

    char trainName[MAX_NAME_LEN];

    char source[MAX_STATION_LEN];
    char destination[MAX_STATION_LEN];

    choice =
        getIntegerInput(
        "\n1. Search By Train Number"
        "\n2. Search By Train Name"
        "\n3. Search By Source & Destination"
        "\nEnter Choice : ");

    current = trainHead;

    switch(choice)
    {
        case 1:
        {
            int trainNo =
                getIntegerInput(
                "Enter Train Number : ");

            while(current != NULL)
            {
                if(current->trainNumber ==
                   trainNo)
                {
                    printf(
                    "\nTrain Found");

                    printf(
                    "\nTrain Number : %d",
                    current->trainNumber);

                    printf(
                    "\nTrain Name   : %s\n",
                    current->trainName);
                    displayTrain(current);

                    return;
                }

                current =
                    current->next;
            }
            break;
        }

        case 2:
        {
            getStringInput(
                "Enter Train Name : ",
                trainName,
                MAX_NAME_LEN);

            while(current != NULL)
            {
                if(strcmp(
                   current->trainName,
                   trainName) == 0)
                {
                    printf(
                    "\nTrain Found");

                    printf(
                    "\nTrain Number : %d",
                    current->trainNumber);

                    printf(
                    "\nTrain Name   : %s\n",
                    current->trainName);
                    displayTrain(
            current);

                    return;
                }

                current =
                    current->next;
            }
         

            break;
        }

        case 3:
{
    getStringInput(
        "Source station: ",
        source,
        MAX_STATION_LEN);

    getStringInput(
        "Destination station: ",
        destination,
        MAX_STATION_LEN);

    while(current != NULL)
    {
        int sourceIndex = -1;
        int destinationIndex = -1;

        for(int i = 0;
            i < current->totalStations;
            i++)
        {
            if(strcmp(
                current->stations[i]
                .stationName,
                source) == 0)
            {
                sourceIndex = i;
            }

            if(strcmp(
                current->stations[i]
                .stationName,
                destination) == 0)
            {
                destinationIndex = i;
            }
        }

        if(sourceIndex != -1 &&
           destinationIndex != -1 &&
           sourceIndex < destinationIndex)
        {
            printf(
            "\nTrain Found\n");

            displayTrain(
            current);

            return;
        }

        current =
        current->next;
    }

    break;
}

        default:
        {
            printf(
            "\nInvalid Choice\n");

            return;
        }
    }

    printf(
    "\nTrain Not Found\n");
}

void updateTrain(void)
{
    int trainNumber;
    int choice;

    Train *current = trainHead;

    trainNumber =
        getIntegerInput(
        "\nTrain number to update: ");

    while(current != NULL)
    {
        if(current->trainNumber ==
           trainNumber)
        {
            printf(
            "\nTrain Found");

            printf(
            "\n1. Update / Reschedule Route");

            printf(
            "\n2. Update Coach Details");

            choice =
                getIntegerInput(
                "\nEnter Choice : ");

            switch(choice)
            {
                case 1:
                {
                    manageRouteDetails(
                    current);

                    break;
                }

                case 2:
                {
                    updateCoachDetails(
                    current);

                    break;
                }

                default:
                {
                    printf(
                    "\nInvalid Choice\n");

                    return;
                }
            }

            saveTrainData();

            printf(
            "\nTrain Updated Successfully\n");

            return;
        }

        current =
            current->next;
    }

    printf(
    "\nTrain Not Found\n");
}

void deleteTrain(void)
{
    int trainNumber;

    Train *current = trainHead;

    trainNumber =
        getIntegerInput(
        "\nTrain number to delete: ");

    while(current != NULL)
    {
        if(current->trainNumber ==
           trainNumber)
        {
            char confirm[8];
            getStringInput("Confirm train deletion? (Y/N): ", confirm, sizeof confirm);
            if (strcmp(confirm, "Y") != 0 && strcmp(confirm, "y") != 0) { printf("Deletion aborted.\n"); return; }
            if(current->prev == NULL)
            {
                trainHead =
                    current->next;

                if(trainHead != NULL)
                {
                    trainHead->prev =
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

            saveTrainData();

            printf(
            "\nTrain Deleted Successfully\n");

            return;
        }

        current =
            current->next;
    }

    printf(
    "\nTrain Not Found\n");
}