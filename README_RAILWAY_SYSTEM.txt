RAILWAY RESERVATION SYSTEM — VALIDATION UPDATE

WHAT CHANGED
- Added admin_auth.c/.h. Admin access requires manually created admin.txt credentials; missing/malformed files deny access. Three failed attempts return to the main menu.
- Added strict reusable integer and floating-point input parsing in utils.c. Trailing junk (e.g. 2abc), empty input and overlong input are rejected and retried.
- Added real calendar date validation for DD:MM:YYYY, including leap years.
- Train classes are restricted to SL, 1A, 2A, 3A, 3E, 2S, EC and CC. Class count is 1–8; duplicate classes are rejected.
- Improved train route, station, time, capacity and fare checks. Legacy stored times such as 3:00 are normalized to 03:00 while loading so the existing sample data remains usable.
- Added user login attempt limit (3), duplicate user rejection, passenger data validation and checks for invalid records read from data files.
- Unreserved tickets no longer generate or display a PNR. Older saved unreserved records that had a PNR are normalized to no PNR when loaded. PNR status verification searches reserved tickets only.
- Reserved booking validates date, train route, class, inventory and payment. Three failed payment attempts abort booking without changing inventory.
- Inventory is loaded from running.txt without resetting saved counts; inventory records are checked against trains, schedules, classes and capacities.
- Added examples only to selected critical numeric inputs where they help avoid invalid negative/fractional values. Ordinary text, station, date/time and menu prompts do not display examples. Use 0 to go back/cancel where supported.
- Destructive train/schedule operations ask for confirmation.

ADMIN CREDENTIAL SETUP (MANUAL, REQUIRED)
1. Create a file named admin.txt in the same working directory as the program.
2. Copy the following four-line format and replace CHANGE_THIS_PASSWORD with a password you choose:

ADMIN_START
admin
CHANGE_THIS_PASSWORD
ADMIN_END

3. Do NOT upload admin.txt.example as admin.txt without changing the placeholder password.
4. The program will not create default credentials. If admin.txt is missing or malformed, admin login is denied.
5. The username/password are stored as plain text for this academic project. Do not reuse a real personal password.

CRITICAL INPUT EXAMPLES
- Train number: 1279 (positive whole number)
- Passenger age: 22 (whole number from 1 to 120)
- Phone: 9876543210 (10 digits starting 6–9)
- Seat/passenger count: 2 (whole number; reject negative/fractional values)
- Delay minutes: 30 (whole number, 0 or greater)
- Fare/payment/refund amount: 125.50 (valid amount; decimals allowed, negative values rejected)
- Unreserved passenger counts: 2 adults and 1 child (whole numbers; total must be at least one)

BUILD / ONLINEGDB
- Upload all .c and .h files from this ZIP.
- Compile all .c files together. The code uses standard C library functions; if the compiler asks for it, link with -lm.
- Keep your existing trains.txt, trainschedule.txt, running.txt, bookings.txt, users.txt and passengers.txt. Back them up first; do not replace your existing data with samples.
- The ZIP intentionally does not include actual admin.txt or user data files. Create admin.txt manually as described above.

NOTES
- Existing data files are preserved in their line-based formats. Invalid records are skipped only when the loader can safely recover at a record boundary.
- The system is a local academic C program. Payment is simulated; it does not transfer real money. Plain-text credential files are not suitable for production deployment.
- Input buffers have fixed limits: user names/passwords up to 99 characters, train/station names constrained by existing struct sizes, and overlong input lines are rejected.
