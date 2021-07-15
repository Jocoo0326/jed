#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t capacity;
  size_t size;
  char *chars;
} Line;

void line_insert_text_before_col(Line *line, const char *text, size_t col);
void line_backspace(Line *line, size_t col);
void line_delete(Line *line, size_t col);

#endif /* BUFFER_H */
