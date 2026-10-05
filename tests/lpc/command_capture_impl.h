private string command_output = "";
void capture_write(mixed value) { command_output += stringp(value) ? value : sprintf("%O", value); }
void capture_line(string value) { capture_write(value + "\n"); }
int capture_fail(mixed value) { capture_write(value); return 0; }
void capture_printf(string fmt, mixed *args...) { capture_write(sprintf(fmt, args...)); }
void capture_print(mixed value) { capture_write(sprintf("%O", value)); }
string take_output() { string value = command_output; command_output = ""; return value; }
int capture_wizard(object who) {
    return objectp(who) && (userp(who) || member_array(
        base_name(who),
        ({ "/tests/command_actor", "/tests/which_actor" })
    ) != -1);
}
