#ifndef REALLOCATION_H
#define REALLOCATION_H
#include "reserved_booking.h"
#include "availability.h"
typedef struct {
    int oldTicketStatus;
    int oldAvailableSeats;
    int oldAvailableRAC;
    int oldAvailableWL;
    Booking *promotedToCNF;
    Booking *promotedToRAC;
    int promotedToCNFOldStatus;
} ReallocationResult;
int reallocateAfterCancellation(Booking *cancelled, Availability *inventory,
                                int maxSeats, int maxRac, ReallocationResult *result);
void rollbackReallocation(Booking *cancelled, Availability *inventory,
                          const ReallocationResult *result);
#endif
