#include "reserved_booking.h"
#include "train.h"
#include "passenger.h"
#include "trainschedule.h"
#include "utils.h"
#include "availability.h"
#include <ctype.h>
#include <time.h>
#include <math.h>

#define BOOKING_FILE "bookings.txt"
#define BOOKING_TEMP_FILE "bookings.txt.tmp"
Booking *bookingHead = NULL;
static unsigned long pnrCounter = 1;

static TrainSchedule *scheduleFor(int no, const char *date) {
    TrainSchedule *s=scheduleHead;
    while(s){ if(s->trainNumber==no && strcmp(s->journeyDate,date)==0) return s; s=s->next; }
    return NULL;
}
static Passenger *passengerFor(int id) {
    Passenger *p=passengerHead; while(p){ if(p->passengerId==id) return p; p=p->next; } return NULL;
}
static int stationIndex(Train *t,const char *name) {
    for(int i=0;i<t->totalStations;i++) if(strcmp(t->stations[i].stationName,name)==0) return i;
    return -1;
}
static int distanceFor(Train *t,int a,int b) {
    int d=t->stations[b].distanceFromSource-t->stations[a].distanceFromSource;
    return d<0?-d:d;
}
static float fareFor(Train *t,int a,int b,const char *className) {
    int d=distanceFor(t,a,b);
    for(int i=0;i<t->classCount;i++) if(strcmp(t->coaches[i].className,className)==0) return d*t->coaches[i].farePerKM;
    return -1.0f;
}
static int wlLimitFor(Train *t,const char *className) {
    for(int i=0;i<t->classCount;i++) if(strcmp(t->coaches[i].className,className)==0) return t->coaches[i].wlCapacity;
    return 0;
}
void generateBookingPNR(char out[20]) {
    int duplicate;
    do {
        snprintf(out, 20, "PNR%08lx%02lu", (unsigned long)time(NULL), pnrCounter++ % 100);
        duplicate = 0;
        for (Booking *b = bookingHead; b; b = b->next)
            if (strcmp(b->pnrNumber, out) == 0) { duplicate = 1; break; }
    } while (duplicate);
}
static const char *statusName(int s) { return s==CNF?"CNF":s==RAC?"RAC":s==WL?"WL":"CANCELLED"; }
static int supportedClass(const char *name) {
    static const char *allowed[] = {"SL", "1A", "2A", "3A", "3E", "2S", "EC", "CC"};
    for (size_t i = 0; i < sizeof allowed / sizeof allowed[0]; ++i) if (strcmp(name, allowed[i]) == 0) return 1;
    return 0;
}

void printBookingTicket(const Booking *b) {
    if (strcmp(b->bookingType, "UNRESERVED") == 0) {
        printf("\n============= UNRESERVED TICKET =============\n"
               "Ticket Type     : UNRESERVED (No PNR)\n"
               "Train           : %d - %s\nSource          : %s\nDestination     : %s\n"
               "Adults          : %d\nChildren        : %d\nDistance        : %d km\nFare Paid       : Rs. %.2f\n"
               "Payment Status  : PAID\n=============================================\n",
               b->trainNumber, b->trainName, b->source, b->destination,
               b->adults, b->children, b->distance, b->fare);
        return;
    }
    printf("\n=============== RESERVED E-TICKET ===============\nPNR             : %s\nPassenger ID    : %d\nPassenger Name  : %s\nAge / Gender    : %d / %s\nTrain           : %d - %s\nSource          : %s\nDestination     : %s\nJourney Date    : %s\nDistance        : %d km\nClass           : %s\nFare Paid       : Rs. %.2f\nStatus          : %s\n=================================================\n",
           b->pnrNumber,b->passengerId,b->passengerName,b->age,b->gender,
           b->trainNumber,b->trainName,b->source,b->destination,b->journeyDate,
           b->distance,b->className,b->fare,statusName(b->ticketStatus));
}

void initializeBookingModule(void) {
    FILE *fp=fopen(BOOKING_FILE,"a"); if(!fp){printf("Unable to create bookings file.\n");return;} fclose(fp); loadBookingData();
}
int saveBookingData(void) {
    FILE *fp = fopen(BOOKING_TEMP_FILE, "w");
    if (fp == NULL) {
        printf("Unable to open temporary booking file.\n");
        return FAILURE;
    }

    for (Booking *b = bookingHead; b != NULL; b = b->next) {
        if (fprintf(fp,
            "BOOKING_START\n%s\n%d\n%s\n%d\n%s\n%d\n%s\n%s\n%s\n%s\n%d\n%s\n%.2f\n%d\n%s\n%d\n%d\nBOOKING_END\n",
            b->pnrNumber, b->passengerId, b->passengerName, b->age,
            b->gender, b->trainNumber, b->trainName, b->source,
            b->destination, b->journeyDate, b->distance, b->className,
            b->fare, b->ticketStatus, b->bookingType[0] ? b->bookingType : "RESERVED", b->adults, b->children) < 0) {
            fclose(fp);
            remove(BOOKING_TEMP_FILE);
            printf("Failed writing booking data.\n");
            return FAILURE;
        }
    }

    if (fflush(fp) != 0 || ferror(fp)) {
        fclose(fp);
        remove(BOOKING_TEMP_FILE);
        printf("Failed flushing booking data.\n");
        return FAILURE;
    }
    if (fclose(fp) != 0) {
        remove(BOOKING_TEMP_FILE);
        printf("Failed closing booking data.\n");
        return FAILURE;
    }
    if (rename(BOOKING_TEMP_FILE, BOOKING_FILE) != 0) {
        remove(BOOKING_TEMP_FILE);
        printf("Failed replacing bookings.txt.\n");
        return FAILURE;
    }
    return SUCCESS;
}
void loadBookingData(void) {
    FILE *fp = fopen(BOOKING_FILE, "r");
    if (!fp) return;
    char line[256];
    while (fgets(line, sizeof line, fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strcmp(line, "BOOKING_START") != 0) continue;
        Booking *b = calloc(1, sizeof *b);
        if (!b) break;
#define READ_LINE() do { if (!fgets(line, sizeof line, fp)) goto bad; line[strcspn(line, "\r\n")] = 0; } while (0)
#define READ_STR(field) do { READ_LINE(); if (strlen(line) >= sizeof b->field) goto bad; strcpy(b->field, line); } while (0)
#define READ_INT(field) do { READ_LINE(); b->field = atoi(line); } while (0)
        READ_STR(pnrNumber); READ_INT(passengerId); READ_STR(passengerName);
        READ_INT(age); READ_STR(gender); READ_INT(trainNumber); READ_STR(trainName);
        READ_STR(source); READ_STR(destination); READ_STR(journeyDate); READ_INT(distance);
        READ_STR(className); READ_LINE(); b->fare = (float)atof(line); READ_INT(ticketStatus);
        /* Backward-compatible: old records end immediately after ticketStatus. */
        READ_LINE();
        if (strcmp(line, "BOOKING_END") == 0) {
            snprintf(b->bookingType, sizeof b->bookingType, "RESERVED");
        } else {
            if (strlen(line) >= sizeof b->bookingType) goto bad;
            strcpy(b->bookingType, line);
            READ_INT(adults); READ_INT(children); READ_LINE();
            if (strcmp(line, "BOOKING_END") != 0) goto bad;
        }
        int validType = strcmp(b->bookingType, "RESERVED") == 0 || strcmp(b->bookingType, "UNRESERVED") == 0;
        int valid = validType && b->passengerId > 0 && b->trainNumber > 0 && b->source[0] != '\0' && b->destination[0] != '\0' && b->distance > 0 && isfinite(b->fare) && b->fare > 0.0f && b->ticketStatus >= CNF && b->ticketStatus <= CANCELLED;
        if (strcmp(b->bookingType, "UNRESERVED") == 0) {
            /* Older releases generated a PNR for unreserved tickets; normalize it away on load. */
            b->pnrNumber[0] = '\0';
            valid = valid && strcmp(b->journeyDate, "N/A") == 0 && strcmp(b->className, "UNRES") == 0 && b->adults >= 0 && b->children >= 0 && b->adults + b->children >= 1 && b->adults + b->children <= 100;
        } else {
            valid = valid && b->pnrNumber[0] != '\0' && isValidJourneyDate(b->journeyDate) && supportedClass(b->className) && b->age >= 1 && b->age <= 120 && (strcmp(b->gender, "Male") == 0 || strcmp(b->gender, "Female") == 0 || strcmp(b->gender, "Other") == 0);
            for (Booking *old = bookingHead; old; old = old->next) if (strcmp(old->pnrNumber, b->pnrNumber) == 0) valid = 0;
        }
        if (!valid) {
            printf("\nWARNING: Invalid booking record skipped.\n");
            free(b);
            continue;
        }
        b->next = bookingHead;
        if (bookingHead) bookingHead->prev = b;
        bookingHead = b;
        continue;
    bad:
        free(b);
        /* Keep scanning for the next marker instead of aborting all later records. */
#undef READ_LINE
#undef READ_STR
#undef READ_INT
        continue;
#undef READ_LINE
#undef READ_STR
#undef READ_INT
    }
    fclose(fp);
}

void bookTicket(int passengerId) {
    Passenger *passenger=passengerFor(passengerId);
    if(!passenger){printf("Passenger profile not found. Create your profile before booking.\n");return;}
    char source[MAX_STATION_LEN],dest[MAX_STATION_LEN],date[11];
    getStringInput("Source station: ", source, sizeof source);
    getStringInput("Destination station: ", dest, sizeof dest);
    if (strcmp(source, dest) == 0) { printf("Source and destination must differ.\n"); return; }
    while (1) {
        getStringInput("Journey date DD:MM:YYYY: ", date, sizeof date);
        if (isValidJourneyDate(date)) break;
        printf("Invalid calendar date. Please retry.\n");
    }
    int matching=0;
    printf("\n--------------- AVAILABLE TRAINS ---------------\n");
    for(Train *t=trainHead;t;t=t->next) {
        int a=stationIndex(t,source), b=stationIndex(t,dest); TrainSchedule *s=scheduleFor(t->trainNumber,date);
        if(a<0||b<0||a>=b||!s||s->status==TRAIN_CANCELLED)continue;
        matching++; printf("\n%d  %s | Distance: %d km\n",t->trainNumber,t->trainName,distanceFor(t,a,b));
        for(int i=0;i<t->classCount;i++) { Availability *av=findAvailability(t->trainNumber,date,t->coaches[i].className); printf("  %-5s ",t->coaches[i].className); if(!av)printf("No inventory"); else if(av->availableSeats>0)printf("Available: %d",av->availableSeats); else if(av->availableRAC>0)printf("RAC: %d",av->availableRAC); else if(av->availableWL<wlLimitFor(t,t->coaches[i].className))printf("WL: %d",av->availableWL+1); else printf("Not available"); printf("\n"); }
    }
    if(!matching){printf("No scheduled trains found for this route/date. Check station spelling and schedule.\n");return;}
    int trainNo;
    Train *train;
    int si, di;
    TrainSchedule *sch;
    while (1) {
        trainNo = getIntegerInput("\nSelect train number from the list (0 to go back): ");
        if (trainNo == 0) return;
        train = findTrain(trainNo); si = train ? stationIndex(train, source) : -1; di = train ? stationIndex(train, dest) : -1; sch = scheduleFor(trainNo, date);
        if (train && si >= 0 && di > si && sch && sch->status != TRAIN_CANCELLED) break;
        printf("Invalid train for this route/date. Choose a listed train or enter 0 to go back.\n");
    }
    printf("\nSelected: %d %s\n",train->trainNumber,train->trainName);
    for(int i=0;i<train->classCount;i++){Availability *av=findAvailability(trainNo,date,train->coaches[i].className); printf("%-5s ",train->coaches[i].className); if(!av)printf("No inventory"); else if(av->availableSeats>0)printf("Available: %d",av->availableSeats); else if(av->availableRAC>0)printf("RAC: %d",av->availableRAC); else if(av->availableWL<wlLimitFor(train,train->coaches[i].className))printf("WL: %d",av->availableWL+1); else printf("Not available"); printf("\n");}
    char className[MAX_CLASS_LEN];
    Availability *av = NULL;
    float fare = -1.0f;
    while (1) {
        getStringInput("Select class [SL/1A/2A/3A/3E/2S/EC/CC] (0 to go back): ", className, sizeof className);
        if (strcmp(className, "0") == 0) return;
        int allowed = strcmp(className,"SL")==0 || strcmp(className,"1A")==0 || strcmp(className,"2A")==0 || strcmp(className,"3A")==0 || strcmp(className,"3E")==0 || strcmp(className,"2S")==0 || strcmp(className,"EC")==0 || strcmp(className,"CC")==0;
        if (!allowed) { printf("Unsupported class. Choose one of the listed classes.\n"); continue; }
        av = findAvailability(trainNo, date, className); fare = fareFor(train, si, di, className);
        if (av && fare >= 0) break;
        printf("This train has no inventory for that class. Retry.\n");
    }
    int status=0, paid=0;
    if(av->availableSeats>0) status=CNF;
    else if(av->availableRAC>0) status=RAC;
    else if(av->availableWL<wlLimitFor(train,className)) status=WL;
    else { printf("No CNF/RAC/WL availability in this class.\n"); return; }
    printf("\nFare calculation: %d km x class rate = Rs. %.2f\n",distanceFor(train,si,di),fare);
    for (int tries=1; tries<=3; ++tries) {
        float entered = getFloatInput("Enter exact fare amount to pay in rupees (e.g., 125.50): ");
        if (entered >= fare - 0.005f && entered <= fare + 0.005f) { paid=1; break; }
        printf("Payment declined: amount must be Rs. %.2f. Attempt %d of 3.\n", fare, tries);
    }
    if (!paid) { printf("Payment failed after 3 attempts. Booking cancelled; inventory unchanged.\n"); return; }
    /* Allocate only after payment succeeds. WL is a monotonically increasing number, not a decreasing inventory. */
    if(status==CNF) av->availableSeats--;
    else if(status==RAC) av->availableRAC--;
    else av->availableWL++;

    /* Do not confirm a ticket unless the new inventory was persisted. */
    if (saveAvailabilityData() != SUCCESS)
    {
        if(status==CNF) av->availableSeats++;
        else if(status==RAC) av->availableRAC++;
        else av->availableWL--;
        printf("Booking stopped: seat inventory could not be saved to running.txt.\n");
        return;
    }

    Booking *b=calloc(1,sizeof *b);
    if(!b)
    {
        if(status==CNF) av->availableSeats++;
        else if(status==RAC) av->availableRAC++;
        else av->availableWL--;
        saveAvailabilityData();
        printf("Memory allocation failed; no ticket created.\n");
        return;
    }
    generateBookingPNR(b->pnrNumber); b->passengerId=passengerId; snprintf(b->passengerName,sizeof b->passengerName,"%s",passenger->passengerName); b->age=passenger->age; snprintf(b->gender,sizeof b->gender,"%s",passenger->gender); b->trainNumber=trainNo; snprintf(b->trainName,sizeof b->trainName,"%s",train->trainName); snprintf(b->source,sizeof b->source,"%s",source); snprintf(b->destination,sizeof b->destination,"%s",dest); snprintf(b->journeyDate,sizeof b->journeyDate,"%s",date); b->distance=distanceFor(train,si,di); snprintf(b->className,sizeof b->className,"%s",className); b->fare=fare; b->ticketStatus=status; snprintf(b->bookingType,sizeof b->bookingType,"RESERVED"); b->adults=1; b->children=0;
    b->next=bookingHead; if(bookingHead)bookingHead->prev=b; bookingHead=b; if (saveBookingData() != SUCCESS) { bookingHead=b->next; if (bookingHead) bookingHead->prev=NULL; if(status==CNF) av->availableSeats++; else if(status==RAC) av->availableRAC++; else av->availableWL--; saveAvailabilityData(); free(b); printf("Booking record could not be saved.\n"); return; } printBookingTicket(b);
}
void viewMyBookings(int passengerId) { int count=0; for(Booking *b=bookingHead;b;b=b->next)if(b->passengerId==passengerId){printBookingTicket(b);count++;} if(!count)printf("No bookings found for this user.\n"); }
