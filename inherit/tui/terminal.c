// Interactive adapter for the FluffOS TUI engines; see docs/TUI.md.
// Inherit into the interactive object, then forward resize and lifecycle hooks.
#include <tui.h>

inherit TUI_ANSI;

#define TUI_IDLE 0
#define TUI_READ 1
#define TUI_SELECT 2
#define TUI_CONFIRM 3
#define TUI_FULL 4

private nosave object keyDecoder, editor, menu, currentApp;
private nosave function reply;
private nosave int mode, generation, confirmDefault, negotiated;
private nosave int termWidth = 80, termHeight = 24;
private nosave string termType = "";
private nosave mixed deferredError;

void tui_close();
protected void tuiInput(string text);

int tui_supported() { return TUI_HAS_INPUT; }
int tui_active() { return mode != TUI_IDLE; }
int tui_width() { return termWidth; }
int tui_height() { return termHeight; }
string tui_term() { return termType; }
object tui_editor() { return editor; }

void tui_send(string text) {
    if (stringp(text) && sizeof(text)) receive(text);
}

private void releaseInput() {
#if TUI_HAS_INPUT
    remove_get_char(this_object());
#endif
}

private void armInput() {
#if TUI_HAS_INPUT
    if (!get_char((: tuiInput :), 1)) error("TUI: another input handler is active.\n");
#else
    error("TUI: interactive mode is not supported by this driver.\n");
#endif
}

private void requireInput() {
    if (!tui_supported()) error("TUI: interactive mode is not supported by this driver.\n");
    if (mode != TUI_IDLE) error("TUI: an interface is already active.\n");
    if (!interactive(this_object()) || this_player() != this_object())
        error("TUI: start from this player's command or input callback.\n");
    if (in_input(this_object())) error("TUI: another input handler is active.\n");
}

private void beginInput(int nextMode, function callback) {
    if (!keyDecoder) keyDecoder = new(TUI_KEYS);
    keyDecoder->reset();
    generation++;
    mode = nextMode;
    reply = callback;
    if (!negotiated) {
        request_term_size();
        start_request_term_type();
        negotiated = 1;
    }
}

// Detach the old session before invoking user callbacks; callbacks may open a new one.
private void finishInput() {
    releaseInput();
    mode = TUI_IDLE;
    generation++;
    reply = 0;
}

private void failInput(mixed err) {
    catch(tui_close());
    error(stringp(err) ? err : "TUI: callback failed.\n");
}

private void finishRead(int state) {
    function callback;
    string line;

    callback = reply;
    line = state == TUI_RL_DONE ? editor->rl_line() : 0;
    tui_send(ansi_bracketed_paste(0));
    finishInput();
    if (functionp(callback)) evaluate(callback, line, state);
}

private void finishMenu(int state) {
    function callback;
    mixed result;
    string *items;
    string item;

    callback = reply;
    result = menu->m_result();
    if (arrayp(result)) {
        if (state != TUI_RL_DONE) result = ({});
        items = map(result, (: menu->m_item($1) :));
        finishInput();
        if (functionp(callback)) evaluate(callback, result, items, state);
    } else {
        if (state != TUI_RL_DONE) result = -1;
        item = result >= 0 ? menu->m_item(result) : 0;
        finishInput();
        if (functionp(callback)) evaluate(callback, result, item, state);
    }
}

private void confirmInput(mixed event) {
    function callback;
    int yes, state;

    yes = 0;
    state = TUI_RL_DONE;
    if (event == "y" || event == "Y") yes = 1;
    else if (event == "n" || event == "N") yes = 0;
    else if (event == TUI_KEY_ENTER) yes = confirmDefault;
    else if (event == TUI_CTRL('c') || event == TUI_KEY_ESC) state = TUI_RL_ABORT;
    else return;
    callback = reply;
    tui_send((state == TUI_RL_ABORT ? "取消" : (yes ? "是" : "否")) + "\r\n");
    finishInput();
    if (functionp(callback)) evaluate(callback, yes, state);
}

private void dispatch(mixed event) {
    int state;

    switch (mode) {
        case TUI_READ:
            state = editor->rl_feed(event);
            tui_send(editor->rl_take_output());
            if (state != TUI_RL_MORE) finishRead(state);
            break;
        case TUI_SELECT:
            state = menu->m_feed(event);
            tui_send(menu->m_take_output());
            if (state != TUI_RL_MORE) finishMenu(state);
            break;
        case TUI_CONFIRM:
            confirmInput(event);
            break;
        case TUI_FULL:
            if (arrayp(event) && sizeof(event) && event[0] == TUI_EV_MOUSE)
                currentApp->on_mouse(event);
            else
                currentApp->on_key(event);
            if (currentApp) tui_send(currentApp->render());
            break;
    }
}

protected void tuiInput(string text) {
    mixed event, err;
    mixed *events;
    int token;

    if (mode == TUI_IDLE) return;
    if (deferredError) {
        err = deferredError;
        deferredError = 0;
        failInput(err);
    }
    token = generation;
    // Ctrl+C must still cancel after an incomplete escape sequence.
    if (text == "\x03") {
        keyDecoder->reset();
        events = ({ TUI_CTRL('c') });
    } else {
        events = keyDecoder->feed(text);
    }
    foreach (event in events) {
        if (mode == TUI_IDLE || generation != token) break;
        err = catch(dispatch(event));
        if (err) failInput(err);
    }
    if (mode == TUI_IDLE || generation != token) return;
    err = catch(armInput());
    if (err) failInput(err);
}

// Hosts with their own driver applies can forward to these named hooks.
void tui_window_size(int width, int height) {
    mixed err;

    if (width < 1 || height < 1) return;
    termWidth = width > 500 ? 500 : width;
    termHeight = height > 200 ? 200 : height;
    if (mode == TUI_READ && editor) {
        editor->rl_set_width(termWidth);
        tui_send(editor->rl_take_output());
    } else if (mode == TUI_SELECT && menu) {
        menu->m_set_width(termWidth);
        tui_send(menu->m_redraw());
    } else if (mode == TUI_FULL && currentApp) {
        err = catch {
            currentApp->on_resize(termWidth, termHeight);
            tui_send(currentApp->render());
        };
        if (err) {
            deferredError = err;
            tui_send("界面发生错误，请按任意键返回。\r\n");
        }
    }
}

void tui_terminal_type(string name) { termType = stringp(name) ? name : ""; }
void window_size(int width, int height) { tui_window_size(width, height); }
void terminal_type(string name) { tui_terminal_type(name); }

varargs void tui_readline(function callback, mapping opts) {
    mixed err;

    requireInput();
    if (!functionp(callback)) error("TUI: callback required.\n");
    opts = mapp(opts) ? copy(opts) : ([]);
    err = catch {
        if (!editor) editor = new(TUI_READLINE);
        editor->rl_set_prompt(stringp(opts["prompt"]) ? opts["prompt"] : "> ");
        editor->rl_set_masked(opts["masked"]);
        editor->rl_set_completer(opts["completer"]);
        if (arrayp(opts["history"])) editor->rl_set_history(opts["history"]);
        editor->rl_set_width(termWidth);
        beginInput(TUI_READ, callback);
        tui_send(ansi_bracketed_paste(1) + editor->rl_begin(opts["initial"]));
        armInput();
    };
    if (err) failInput(err);
}

private void startMenu(function callback, string prompt, string *choices, mapping opts) {
    mixed err;

    requireInput();
    if (!functionp(callback)) error("TUI: callback required.\n");
    err = catch {
        if (!menu) menu = new(TUI_MENU);
        menu->m_set_width(termWidth);
        beginInput(TUI_SELECT, callback);
        tui_send(menu->m_begin(prompt, choices, opts));
        if (menu->m_state() != TUI_RL_MORE) finishMenu(menu->m_state());
        else armInput();
    };
    if (err) failInput(err);
}

varargs void tui_select(function callback, string prompt, string *choices, mapping opts) {
    opts = mapp(opts) ? copy(opts) : ([]);
    map_delete(opts, "multi");
    startMenu(callback, prompt, choices, opts);
}

varargs void tui_multiselect(function callback, string prompt, string *choices, mapping opts) {
    opts = mapp(opts) ? copy(opts) : ([]);
    opts["multi"] = 1;
    startMenu(callback, prompt, choices, opts);
}

varargs void tui_confirm(function callback, string prompt, int defaultValue) {
    mixed err;

    requireInput();
    if (!functionp(callback)) error("TUI: callback required.\n");
    err = catch {
        confirmDefault = defaultValue ? 1 : 0;
        beginInput(TUI_CONFIRM, callback);
        tui_send("? " + prompt + (defaultValue ? " [Y/n] " : " [y/N] "));
        armInput();
    };
    if (err) failInput(err);
}

varargs void tui_open(object app, mapping opts) {
    mixed err;

    requireInput();
    if (!objectp(app)) error("TUI: application required.\n");
    opts = mapp(opts) ? copy(opts) : ([]);
    err = catch {
        currentApp = app;
        beginInput(TUI_FULL, 0);
        tui_send(ansi_alt_screen(1) + ansi_cursor_visible(0) + ansi_bracketed_paste(1) +
            (opts["mouse"] ? ansi_mouse(1) : ""));
        app->on_open(this_object(), termWidth, termHeight);
        if (currentApp) {
            tui_send(currentApp->render());
            armInput();
        }
    };
    if (err) failInput(err);
}

private void closeSession() {
    object app;
    int oldMode;

    if (mode == TUI_IDLE) return;
    app = currentApp;
    oldMode = mode;
    currentApp = 0;
    deferredError = 0;
    finishInput();
    if (oldMode == TUI_FULL)
        tui_send(ansi_mouse(0) + ansi_bracketed_paste(0) + ansi_cursor_visible(1) +
            ansi_reset() + ansi_alt_screen(0));
    else if (oldMode == TUI_READ)
        tui_send(ansi_bracketed_paste(0) + "\r\n");
    else
        tui_send("\r\n");
    if (app) app->app_quit();
}

void tui_close() {
    if (mode == TUI_IDLE) return;
#if TUI_HAS_INPUT
    if (interactive(this_object()) && query_charmode(this_object()) > 0)
        error("TUI: asynchronous close is unsupported; close from a key callback.\n");
#endif
    closeSession();
}

void tui_destroy() {
    mixed err;

    err = catch(closeSession());
    if (keyDecoder) destruct(keyDecoder);
    if (editor) destruct(editor);
    if (menu) destruct(menu);
    currentApp = 0;
    negotiated = 0;
    termWidth = 80;
    termHeight = 24;
    termType = "";
    if (err) error(stringp(err) ? err : "TUI: cleanup failed.\n");
}
