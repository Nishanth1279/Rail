#include "utils.h"
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

void initializeSystem(void)
{
    printf("\n======================================\n");
    printf(" Railway Reservation System\n");
    printf("======================================\n");
}

void cleanupSystem(void)
{
    printf("\nSystem Closed Successfully...\n");
}

void clearInputBuffer(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}

static int readLine(const char *message, char *buffer, size_t size)
{
    size_t len;
    int ch;
    if (!buffer || size < 2) return FAILURE;
    printf("%s", message);
    fflush(stdout);
    if (!fgets(buffer, (int)size, stdin)) {
        buffer[0] = '\0';
        return FAILURE;
    }
    len = strlen(buffer);
    if (len && buffer[len - 1] == '\n') {
        buffer[--len] = '\0';
        if (len && buffer[len - 1] == '\r') buffer[--len] = '\0';
        return SUCCESS;
    }
    ch = getchar();
    if (ch != EOF && ch != '\n') {
        while (ch != '\n' && ch != EOF) ch = getchar();
        buffer[0] = '\0';
        printf("Input too long. Please enter no more than %lu characters.\n", (unsigned long)(size - 1));
        return FAILURE;
    }
    return SUCCESS;
}

int getIntegerInput(const char *message)
{
    char buffer[128], *end;
    long value;
    while (1) {
        if (!readLine(message, buffer, sizeof buffer)) {
            if (feof(stdin)) return 0;
            continue;
        }
        char *p = buffer;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') { printf("Input cannot be empty. Enter a whole number (e.g., 1279).\n"); continue; }
        errno = 0;
        value = strtol(p, &end, 10);
        while (isspace((unsigned char)*end)) end++;
        if (errno == ERANGE || end == p || *end != '\0' || value < INT_MIN || value > INT_MAX) {
            printf("Invalid integer. Enter digits only (e.g., 1279). Enter 0 to go back where supported.\n");
            continue;
        }
        return (int)value;
    }
}

void getStringInput(const char *message, char *buffer, int size)
{
    while (1) {
        if (!readLine(message, buffer, (size_t)size)) {
            if (feof(stdin)) { if (size > 0) buffer[0] = '\0'; return; }
            continue;
        }
        char *p = buffer;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') { printf("Input cannot be empty. Please try again.\n"); continue; }
        if (p != buffer) memmove(buffer, p, strlen(p) + 1);
        return;
    }
}

float getFloatInput(const char *message)
{
    char buffer[128], *end;
    double value;
    while (1) {
        if (!readLine(message, buffer, sizeof buffer)) {
            if (feof(stdin)) return 0.0f;
            continue;
        }
        char *p = buffer;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') { printf("Amount cannot be empty (e.g., 125.50).\n"); continue; }
        errno = 0;
        value = strtod(p, &end);
        while (isspace((unsigned char)*end)) end++;
        if (errno == ERANGE || end == p || *end != '\0' || !isfinite(value) || value > 1.0e9 || value < -1.0e9) {
            printf("Invalid amount. Enter a number only (e.g., 125.50).\n");
            continue;
        }
        return (float)value;
    }
}

int isValidJourneyDate(const char *date)
{
    int dd, mm, yyyy;
    if (!date || strlen(date) != 10 || date[2] != ':' || date[5] != ':') return FAILURE;
    for (int i = 0; i < 10; ++i)
        if (i != 2 && i != 5 && (date[i] < '0' || date[i] > '9')) return FAILURE;
    if (sscanf(date, "%2d:%2d:%4d", &dd, &mm, &yyyy) != 3 || yyyy < 1900 || yyyy > 9999 || mm < 1 || mm > 12) return FAILURE;
    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int maxDay = days[mm - 1];
    if (mm == 2 && ((yyyy % 400 == 0) || (yyyy % 4 == 0 && yyyy % 100 != 0))) maxDay = 29;
    return dd >= 1 && dd <= maxDay ? SUCCESS : FAILURE;
}
