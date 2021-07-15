#include "buffer.h"

#define LINE_INIT_CAPACITY 1024

static void line_grow(Line *line, size_t n) {
  size_t new_capacity = line->capacity;
  while (new_capacity - line->size < n) {
    if (new_capacity == 0) {
      new_capacity = LINE_INIT_CAPACITY;
    } else {
      new_capacity = new_capacity << 1;
    }
  }
  if (new_capacity > line->capacity) {
    line->chars = realloc(line->chars, new_capacity);
    line->capacity = new_capacity;
  }
}

void line_insert_text_before_col(Line *line, const char *text, size_t col) {
  size_t text_size = strlen(text);
  line_grow(line, text_size);

  memmove(line->chars + col + text_size, line->chars + col, line->size - col);
  memcpy(line->chars + col, text, text_size);
  line->size += text_size;
}

void line_backspace(Line *line, size_t col) {
  if (line->size > 0 && col > 0) {
    memmove(line->chars + col - 1, line->chars + col, line->size - col);
    line->size -= 1;
  }
}

void line_delete(Line *line, size_t col) {
  if (col < line->size) {
    memmove(line->chars + col, line->chars + col + 1, line->size - col - 1);
    line->size -= 1;
  }
}
