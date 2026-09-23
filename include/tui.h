#ifndef TUI_H
#define TUI_H

#include <mudcore.h>

// ---------------------------------------------------------------------------
// mudcore TUI — public contract. See docs/TUI.md.
//
// Key events are ints (special keys / modified keys) or strings (printable
// text; a whole bracketed paste arrives as one string).  Mouse reports are
// arrays: ({ TUI_EV_MOUSE, button, x, y, pressed }).
// ---------------------------------------------------------------------------

// Object paths (extension-less: the driver resolves .lpc/.c)
#define CORE_TUI_DIR CORE_DIR "inherit/tui"
#define CORE_TUI_ANSI CORE_TUI_DIR "/ansi"
#define CORE_TUI_KEYS CORE_TUI_DIR "/keys"
#define CORE_TUI_READLINE CORE_TUI_DIR "/readline"
#define CORE_TUI_SCREEN CORE_TUI_DIR "/screen"
#define CORE_TUI_WIDGET CORE_TUI_DIR "/widget"
#define CORE_TUI_APP CORE_TUI_DIR "/app"
#define CORE_TUI_TERMINAL CORE_TUI_DIR "/terminal"
#define CORE_TUI_PRINT CORE_TUI_DIR "/print"
#define CORE_TUI_MENU CORE_TUI_DIR "/menu"
#define CORE_TUI_CANVAS CORE_TUI_DIR "/canvas"
#define CORE_TUI_W_BUTTON CORE_TUI_DIR "/w/button"
#define CORE_TUI_W_CHART CORE_TUI_DIR "/w/chart"
#define CORE_TUI_W_CHECKLIST CORE_TUI_DIR "/w/checklist"
#define CORE_TUI_W_LABEL CORE_TUI_DIR "/w/label"
#define CORE_TUI_W_LIST CORE_TUI_DIR "/w/list"
#define CORE_TUI_W_LOG CORE_TUI_DIR "/w/log"
#define CORE_TUI_W_PROGRESS CORE_TUI_DIR "/w/progress"
#define CORE_TUI_W_RADIOLIST CORE_TUI_DIR "/w/radiolist"
#define CORE_TUI_W_SPINNER CORE_TUI_DIR "/w/spinner"
#define CORE_TUI_W_TABLE CORE_TUI_DIR "/w/table"
#define CORE_TUI_W_TEXTFIELD CORE_TUI_DIR "/w/textfield"
#define CORE_TUI_W_TREE CORE_TUI_DIR "/w/tree"
#ifndef TUI_DIR
#define TUI_DIR CORE_TUI_DIR
#endif
#ifndef TUI_ANSI
#define TUI_ANSI TUI_DIR "/ansi"
#endif
#ifndef TUI_KEYS
#define TUI_KEYS TUI_DIR "/keys"
#endif
#ifndef TUI_READLINE
#define TUI_READLINE TUI_DIR "/readline"
#endif
#ifndef TUI_SCREEN
#define TUI_SCREEN TUI_DIR "/screen"
#endif
#ifndef TUI_WIDGET
#define TUI_WIDGET TUI_DIR "/widget"
#endif
#ifndef TUI_APP
#define TUI_APP TUI_DIR "/app"
#endif
#ifndef TUI_TERMINAL
#define TUI_TERMINAL TUI_DIR "/terminal"
#endif
#ifndef TUI_PRINT
#define TUI_PRINT TUI_DIR "/print"
#endif
#ifndef TUI_MENU
#define TUI_MENU TUI_DIR "/menu"
#endif
#ifndef TUI_CANVAS
#define TUI_CANVAS TUI_DIR "/canvas"
#endif
#ifndef TUI_W_BUTTON
#define TUI_W_BUTTON TUI_DIR "/w/button"
#endif
#ifndef TUI_W_CHART
#define TUI_W_CHART TUI_DIR "/w/chart"
#endif
#ifndef TUI_W_CHECKLIST
#define TUI_W_CHECKLIST TUI_DIR "/w/checklist"
#endif
#ifndef TUI_W_LABEL
#define TUI_W_LABEL TUI_DIR "/w/label"
#endif
#ifndef TUI_W_LIST
#define TUI_W_LIST TUI_DIR "/w/list"
#endif
#ifndef TUI_W_LOG
#define TUI_W_LOG TUI_DIR "/w/log"
#endif
#ifndef TUI_W_PROGRESS
#define TUI_W_PROGRESS TUI_DIR "/w/progress"
#endif
#ifndef TUI_W_RADIOLIST
#define TUI_W_RADIOLIST TUI_DIR "/w/radiolist"
#endif
#ifndef TUI_W_SPINNER
#define TUI_W_SPINNER TUI_DIR "/w/spinner"
#endif
#ifndef TUI_W_TABLE
#define TUI_W_TABLE TUI_DIR "/w/table"
#endif
#ifndef TUI_W_TEXTFIELD
#define TUI_W_TEXTFIELD TUI_DIR "/w/textfield"
#endif
#ifndef TUI_W_TREE
#define TUI_W_TREE TUI_DIR "/w/tree"
#endif

// Interactive TUI is optional; missing driver efuns must not break compilation.
#if !defined(TUI_DISABLE_INPUT) && efun_defined(get_char) && efun_defined(remove_get_char) && efun_defined(query_charmode)
#define TUI_HAS_INPUT 1
#else
#define TUI_HAS_INPUT 0
#endif

// Special keys sit just above the Unicode range (max codepoint 0x10FFFF),
// so they can never collide with a real character.
#define TUI_KEY_SPECIAL   0x110000
#define TUI_KEY_UP        (TUI_KEY_SPECIAL + 1)
#define TUI_KEY_DOWN      (TUI_KEY_SPECIAL + 2)
#define TUI_KEY_RIGHT     (TUI_KEY_SPECIAL + 3)
#define TUI_KEY_LEFT      (TUI_KEY_SPECIAL + 4)
#define TUI_KEY_HOME      (TUI_KEY_SPECIAL + 5)
#define TUI_KEY_END       (TUI_KEY_SPECIAL + 6)
#define TUI_KEY_PGUP      (TUI_KEY_SPECIAL + 7)
#define TUI_KEY_PGDN      (TUI_KEY_SPECIAL + 8)
#define TUI_KEY_INSERT    (TUI_KEY_SPECIAL + 9)
#define TUI_KEY_DELETE    (TUI_KEY_SPECIAL + 10)
#define TUI_KEY_ENTER     (TUI_KEY_SPECIAL + 11)
#define TUI_KEY_TAB       (TUI_KEY_SPECIAL + 12)
#define TUI_KEY_BACKSPACE (TUI_KEY_SPECIAL + 13)
#define TUI_KEY_ESC       (TUI_KEY_SPECIAL + 14)
#define TUI_KEY_F1        (TUI_KEY_SPECIAL + 21)
#define TUI_KEY_F2        (TUI_KEY_SPECIAL + 22)
#define TUI_KEY_F3        (TUI_KEY_SPECIAL + 23)
#define TUI_KEY_F4        (TUI_KEY_SPECIAL + 24)
#define TUI_KEY_F5        (TUI_KEY_SPECIAL + 25)
#define TUI_KEY_F6        (TUI_KEY_SPECIAL + 26)
#define TUI_KEY_F7        (TUI_KEY_SPECIAL + 27)
#define TUI_KEY_F8        (TUI_KEY_SPECIAL + 28)
#define TUI_KEY_F9        (TUI_KEY_SPECIAL + 29)
#define TUI_KEY_F10       (TUI_KEY_SPECIAL + 30)
#define TUI_KEY_F11       (TUI_KEY_SPECIAL + 31)
#define TUI_KEY_F12       (TUI_KEY_SPECIAL + 32)
#define TUI_KEY_UNKNOWN   (TUI_KEY_SPECIAL + 63)

// Modifier bits, above the special-key range.
#define TUI_MOD_SHIFT     0x200000
#define TUI_MOD_ALT       0x400000
#define TUI_MOD_CTRL      0x800000
#define TUI_MODS          (TUI_MOD_SHIFT | TUI_MOD_ALT | TUI_MOD_CTRL)

// Convenience: TUI_CTRL('a') is what Ctrl-A decodes to.
#define TUI_CTRL(c)       (TUI_MOD_CTRL | (c))
#define TUI_ALT(c)        (TUI_MOD_ALT | (c))

// Array-event kinds (first element of an array event)
#define TUI_EV_MOUSE      1
#define TUI_EV_CPR        2   /* cursor position report: ({ kind, row, col }) */

// readline engine states (rl_feed() return / rl_state())
#define TUI_RL_MORE       0   /* still editing */
#define TUI_RL_DONE       1   /* line accepted (Enter) */
#define TUI_RL_ABORT      2   /* Ctrl-C */
#define TUI_RL_EOF        3   /* Ctrl-D on an empty line */

// Box styles for scr_box()
#define TUI_BOX_SINGLE    0
#define TUI_BOX_DOUBLE    1
#define TUI_BOX_ROUND     2
#define TUI_BOX_ASCII     3

#endif
