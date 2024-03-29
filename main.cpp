/*** includes ***/

#include "undo.h"



// cx: char position, it is a location in terms of chatacters only
// rx: render position, it is a location taking into account that some characters are represented by more or less than 1 character.

#include <terminal.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <memory>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>



/*** defines ***/

#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3

using Term::Terminal;
using Term::cursor_on;
using Term::cursor_off;
using Term::move_cursor;
using Term::erase_to_eol;
using Term::color;
using Term::fg;
using Term::style;
using Term::Key;




typedef struct erow {
  int idx;
  int size;
  int rsize;
  char *chars;
  char *render;
} erow;

struct editorConfig {
  int cx, cy; // cursor location
  int rx;     // x cursor location including the real screen size of characters.
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  int numrows;
  erow *row;
  int dirty;
  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
};

struct editorConfig E;

struct UndoList undoList;

/*** filetypes ***/



/*** prototypes ***/

char *editorPrompt(const Terminal &term, const char *prompt, void (*callback)(char *, int));



/*** row operations ***/

int charRenderSize(char c, int rx)
// return the size in screen of a character. tabs can be more than one and some unicode characters, less.
{
    if (c == '\t') return KILO_TAB_STOP - (rx % KILO_TAB_STOP);  // Expands a tab.
    else return 1;
}


int editorRowCxToRx(erow *row, int cx)
// Converts from cx (char position) to rx (render position).
{
    int rx = 0;

    for (int j = 0; j < cx; j++) rx += charRenderSize(row->chars[j], rx);

    return rx;
}

int editorRowRxToCx(erow *row, int rx)
// Converts from rx (render position) to cx (char position).
{
    int cur_rx = 0;

    for (int cx = 0; cx < row->size; cx++)
    // Starts at cx = 0 and moves right, computing a temporal cur_rc, until rx is reached.
    {
        cur_rx += charRenderSize(row->chars[cx], rx);

        if (cur_rx > rx) return cx;
    }
    return row->size;
}

void editorUpdateRow(erow *row)
{
  int tabs = 0;
  int j;
  for (j = 0; j < row->size; j++)
    if (row->chars[j] == '\t') tabs++;

  free(row->render);
  row->render = (char*)malloc(row->size + tabs*(KILO_TAB_STOP - 1) + 1);

  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % KILO_TAB_STOP != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->rsize = idx;

}

void editorInsertRow(int at, const char *s, size_t len)
// Inserts a row of text at row number "at".
{

  if (at < 0 || at > E.numrows) return;

  addUndoAction(aInsertRow, &undoList);

  E.row = (erow*)realloc(E.row, sizeof(erow) * (E.numrows + 1));
  memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
  for (int j = at + 1; j <= E.numrows; j++) E.row[j].idx++;

  E.row[at].idx = at;

  E.row[at].size = len;
  E.row[at].chars = (char*)malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';

  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  editorUpdateRow(&E.row[at]);

  E.numrows++;
  E.dirty++;
}

void editorFreeRow(erow *row) {
  free(row->render);
  free(row->chars);
}

void editorDelRow(int at) {
  if (at < 0 || at >= E.numrows) return;

  addUndoAction(aDelRow, &undoList);

  editorFreeRow(&E.row[at]);
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
  for (int j = at; j < E.numrows - 1; j++) E.row[j].idx--;
  E.numrows--;
  E.dirty++;
}

void editorRowInsertChar(erow *row, int at, int c) {
  addUndoAction(aRowInsertChar, &undoList);

  if (at < 0 || at > row->size) at = row->size;
  row->chars = (char*)realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  editorUpdateRow(row);
  E.dirty++;
}

void editorRowAppendString(erow *row, char *s, size_t len) {
  addUndoAction(aRowAppendString, &undoList);

  row->chars = (char*)realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  editorUpdateRow(row);
  E.dirty++;
}

void editorRowDelChar(erow *row, int at) {
  if (at < 0 || at >= row->size) return;

  addUndoAction(aRowDelChar, &undoList);
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  editorUpdateRow(row);
  E.dirty++;
}

/*** editor operations ***/

void editorInsertChar(int c) {
  if (E.cy == E.numrows) {
    editorInsertRow(E.numrows, "", 0);
  }
  editorRowInsertChar(&E.row[E.cy], E.cx, c);
  E.cx++;
}

void editorInsertNewline() {
  if (E.cx == 0) {
    editorInsertRow(E.cy, "", 0);
  } else {
    erow *row = &E.row[E.cy];
    editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
    row = &E.row[E.cy];
    row->size = E.cx;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
  }
  E.cy++;
  E.cx = 0;
}

void editorDelChar() {
  if (E.cy == E.numrows) return;
  if (E.cx == 0 && E.cy == 0) return;

  erow *row = &E.row[E.cy];
  if (E.cx > 0) {
    editorRowDelChar(row, E.cx - 1);
    E.cx--;
  } else {
    E.cx = E.row[E.cy - 1].size;
    editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
    editorDelRow(E.cy);
    E.cy--;
  }
}

/*** file i/o ***/

char *editorRowsToString(int *buflen) {
  int totlen = 0;
  int j;
  for (j = 0; j < E.numrows; j++)
    totlen += E.row[j].size + 1;
  *buflen = totlen;

  char *buf = (char*)malloc(totlen);
  char *p = buf;
  for (j = 0; j < E.numrows; j++) {
    memcpy(p, E.row[j].chars, E.row[j].size);
    p += E.row[j].size;
    *p = '\n';
    p++;
  }

  return buf;
}

void editorOpen(char *filename) {
    free(E.filename);
    E.filename = strdup(filename);

//    editorSelectSyntaxHighlight();
    resetUndoList(undoList);

    std::ifstream f(filename);
    if (f.fail()) throw std::runtime_error("File failed to open.");
    std::string line;
    std::getline(f, line);
    while (f.rdstate() == std::ios_base::goodbit) {
        int linelen = line.size();
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r'))
            linelen--;
        editorInsertRow(E.numrows, line.c_str(), linelen);
        std::getline(f, line);
    }
    E.dirty = 0;
}

void editorSetStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

void editorSave(const Terminal &term) {
  if (E.filename == NULL) {
    E.filename = editorPrompt(term, "Save as: %s (ESC to cancel)", NULL);
    if (E.filename == NULL) {
      editorSetStatusMessage("Save aborted");
      return;
    }
//    editorSelectSyntaxHighlight();
  }

  int len;
  char *buf = editorRowsToString(&len);
  std::string s = std::string(buf, len);
  free(buf);

  std::ofstream out;
  out.open(E.filename);
  out << s;
  out.close();
  E.dirty = 0;
  editorSetStatusMessage("%d bytes written to disk", len);
}

/*** find ***/

void editorFindCallback(char *query, int key) {
  static int last_match = -1;
  static int direction = 1;


  if (key == Key::ENTER || key == Key::ESC) {
    last_match = -1;
    direction = 1;
    return;
  } else if (key == Key::ARROW_RIGHT || key == Key::ARROW_DOWN) {
    direction = 1;
  } else if (key == Key::ARROW_LEFT || key == Key::ARROW_UP) {
    direction = -1;
  } else {
    last_match = -1;
    direction = 1;
  }

  if (last_match == -1) direction = 1;
  int current = last_match;
  int i;
  for (i = 0; i < E.numrows; i++) {
    current += direction;
    if (current == -1) current = E.numrows - 1;
    else if (current == E.numrows) current = 0;

    erow *row = &E.row[current];
    char *match = strstr(row->render, query);
    if (match) {
      last_match = current;
      E.cy = current;
      E.cx = editorRowRxToCx(row, match - row->render);
      E.rowoff = E.numrows;

      break;
    }
  }
}

void editorFind(const Terminal &term) {
  int saved_cx = E.cx;
  int saved_cy = E.cy;
  int saved_coloff = E.coloff;
  int saved_rowoff = E.rowoff;

  char *query = editorPrompt(term, "Search: %s (Use ESC/Arrows/Enter)",
                             editorFindCallback);

  if (query) {
    free(query);
  } else {
    E.cx = saved_cx;
    E.cy = saved_cy;
    E.coloff = saved_coloff;
    E.rowoff = saved_rowoff;
  }
}

/*** output ***/

void editorScroll() {
// Scrolls the editor so the cursor is in view.

  E.rx = 0;
  if (E.cy < E.numrows) {
    E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
  }

  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }

  if (E.cy >= E.rowoff + E.screenrows) {
    E.rowoff = E.cy - E.screenrows + 1;
  }

  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }

  if (E.rx >= E.coloff + E.screencols) {
    E.coloff = E.rx - E.screencols + 1;
  }
}

void editorDrawRows(std::string &ab) {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    int filerow = y + E.rowoff;
    if (filerow >= E.numrows) {
      if (E.numrows == 0 && y == E.screenrows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
          "Kilo editor -- version %s", KILO_VERSION);
        if (welcomelen > E.screencols) welcomelen = E.screencols;
        int padding = (E.screencols - welcomelen) / 2;
        if (padding) {
          ab.append("~");
          padding--;
        }
        while (padding--) ab.append(" ");
        ab.append(welcome);
      } else {
        ab.append("~");
      }
    } else {
      int len = E.row[filerow].rsize - E.coloff;
      if (len < 0) len = 0;
      if (len > E.screencols) len = E.screencols;
      char *c = &E.row[filerow].render[E.coloff];
      fg current_color = fg::black; // black is not used in editorSyntaxToColor
      int j;
      for (j = 0; j < len; j++) {
        if (iscntrl(c[j])) {
          char sym = (c[j] <= 26) ? '@' + c[j] : '?';
          ab.append(color(style::reversed));
          ab.append(std::string(&sym,1));
          ab.append(color(style::reset));
          if (current_color != fg::black) {
            ab.append(color(current_color));
          }
        } else {
          if (current_color != fg::black) {
            ab.append(color(fg::reset));
            current_color = fg::black;
          }
          ab.append(std::string(&c[j], 1));
        }
      }
      ab.append(color(fg::reset));
    }

    ab.append(erase_to_eol());
    ab.append("\r\n");
  }
}

void editorDrawStatusBar(std::string &ab) {
  ab.append(color(style::reversed));
  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
    E.filename ? E.filename : "[No Name]", E.numrows,
    E.dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.numrows);
  if (len > E.screencols) len = E.screencols;
  ab.append(std::string(status, len));
  while (len < E.screencols) {
    if (E.screencols - len == rlen) {
      ab.append(std::string(rstatus, rlen));
      break;
    } else {
      ab.append(" ");
      len++;
    }
  }
  ab.append(color(style::reset));
  ab.append("\r\n");
}

void editorDrawMessageBar(std::string &ab) {
  ab.append(erase_to_eol());
  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols) msglen = E.screencols;
  if (msglen && time(NULL) - E.statusmsg_time < 5)
    ab.append(std::string(E.statusmsg, msglen));
}

void editorRefreshScreen(const Terminal &term) {
  editorScroll();

  std::string ab;
  ab.reserve(16*1024);

  ab.append(cursor_off());
  ab.append(move_cursor(1, 1));

  editorDrawRows(ab);
  editorDrawStatusBar(ab);
  editorDrawMessageBar(ab);

  ab.append(move_cursor((E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1));

  ab.append(cursor_on());

  term.write(ab);
}

/*** input ***/

char *editorPrompt(const Terminal &term, const char *prompt, void (*callback)(char *, int)) {
  size_t bufsize = 128;
  char *buf = (char*)malloc(bufsize);

  size_t buflen = 0;
  buf[0] = '\0';

  while (1) {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen(term);

    int c = term.read_key();
    if (c == Key::DEL || c == CTRL_KEY('h') || c == Key::BACKSPACE) {
      if (buflen != 0) buf[--buflen] = '\0';
    } else if (c == Key::ESC) {
      editorSetStatusMessage("");
      if (callback) callback(buf, c);
      free(buf);
      return NULL;
    } else if (c == Key::ENTER) {
      if (buflen != 0) {
        editorSetStatusMessage("");
        if (callback) callback(buf, c);
        return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = (char*)realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }

    if (callback) callback(buf, c);
  }
}

void editorMoveCursor(int key) {
  erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

  switch (key) {
    case Key::ARROW_LEFT:
      if (E.cx != 0) {
        E.cx--;
      } else if (E.cy > 0) {
        E.cy--;
        E.cx = E.row[E.cy].size;
      }
      break;
    case Key::ARROW_RIGHT:
      if (row && E.cx < row->size) {
        E.cx++;
      } else if (row && E.cx == row->size) {
        E.cy++;
        E.cx = 0;
      }
      break;
    case Key::ARROW_UP:
      if (E.cy != 0) {
        E.cy--;
      }
      break;
    case Key::ARROW_DOWN:
      if (E.cy < E.numrows) {
        E.cy++;
      }
      break;
  }

  row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  int rowlen = row ? row->size : 0;
  if (E.cx > rowlen) {
    E.cx = rowlen;
  }
}

bool editorProcessKeypress(const Terminal &term) {
  static int quit_times = KILO_QUIT_TIMES;

  int c = term.read_key();

  switch (c) {
    case Key::ENTER:
      editorInsertNewline();
      break;

    case CTRL_KEY('q'):
      if (E.dirty && quit_times > 0) {
        editorSetStatusMessage("WARNING!!! File has unsaved changes. "
          "Press Ctrl-Q %d more times to quit.", quit_times);
        quit_times--;
        return true;
      }
      return false;
      break;

    case CTRL_KEY('s'):
      editorSave(term);
      break;

    case Key::HOME:
      E.cx = 0;
      break;

    case Key::END:
      if (E.cy < E.numrows)
        E.cx = E.row[E.cy].size;
      break;

    case CTRL_KEY('f'):
      editorFind(term);
      break;

    case Key::BACKSPACE:
    case CTRL_KEY('h'):
    case Key::DEL:
      if (c == Key::DEL) editorMoveCursor(Key::ARROW_RIGHT);
      editorDelChar();
      break;

    case Key::PAGE_UP:
    case Key::PAGE_DOWN:
      {
        if (c == Key::PAGE_UP) {
          E.cy = E.rowoff;
        } else if (c == Key::PAGE_DOWN) {
          E.cy = E.rowoff + E.screenrows - 1;
          if (E.cy > E.numrows) E.cy = E.numrows;
        }

        int times = E.screenrows;
        while (times--)
          editorMoveCursor(c == Key::PAGE_UP ? Key::ARROW_UP : Key::ARROW_DOWN);
      }
      break;

    case Key::ARROW_UP:
    case Key::ARROW_DOWN:
    case Key::ARROW_LEFT:
    case Key::ARROW_RIGHT:
      editorMoveCursor(c);
      break;

    case CTRL_KEY('l'):
    case Key::ESC:
      break;

    case Key::TAB:
      editorInsertChar('\t');
      break;

    default:
      editorInsertChar(c);
      break;
  }

  quit_times = KILO_QUIT_TIMES;
  return true;
}

/*** init ***/

void initEditor(const Terminal &term) {
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.numrows = 0;
  E.row = NULL;
  E.dirty = 0;
  E.filename = NULL;
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;


  term.get_term_size(E.screenrows, E.screencols);
  E.screenrows -= 2;

  initUndoList(&undoList);
}



int main(int argc, char *argv[]) {
  // We must put all code in try/catch block, otherwise destructors are not
  // being called when exception happens and the terminal is not put into
  // correct state.
  try {
    Terminal term(true, false);
    term.save_screen();
    initEditor(term);
    if (argc >= 2) {
      editorOpen(argv[1]);
    }

    editorSetStatusMessage(
      "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");

    editorRefreshScreen(term);
    while (editorProcessKeypress(term)) {
      editorRefreshScreen(term);
    }
  } catch(const std::runtime_error& re) {
    std::cerr << "Runtime error: " << re.what() << std::endl;
    return 2;
  } catch(...) {
    std::cerr << "Unknown error." << std::endl;
    return 1;
  }
  return 0;
}