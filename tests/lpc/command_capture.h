// Capture presentation; production command logic and driver operations still run.
void capture_write(mixed value);
void capture_line(string value);
int capture_fail(mixed value);
void capture_printf(string fmt, mixed *args...);
void capture_print(mixed value);
#define write capture_write
#define cecho capture_line
#define notify_fail capture_fail
#define printf capture_printf
#define print_r capture_print
// Command behavior tests use a non-interactive actor; production guards are unchanged.
int capture_wizard(object who);
#define wizardp capture_wizard
