#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "gl_extra.h"
#include "editor.h"
#include "la.h"
#include "stb_image.h"
#include "sdl_extra.h"
#include "free_font.h"
#include "cursor.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define DELTA_TIME (1.0f / FPS)

Editor editor = {0};
Vec2f camera_pos = {0};
Vec2f camera_vel = {0};
Free_Render fr;

int main(int argc, char** argv)
{
  const char* file_path = NULL;

  if (argc > 1) {
    file_path = argv[1];
  }

  if (file_path) {
    FILE* file = fopen(file_path, "r");
    if (file != NULL) {
      editor_load_from_file(&editor, file);
      fclose(file);
    }
  }

  scc(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window* window =
      scp(SDL_CreateWindow("jed", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    printf("GL Version %d.%d\n", major, minor);
  }

  scp(SDL_GL_CreateContext(window));

  if (GLEW_OK != glewInit()) {
    fprintf(stderr, "Could not initialize GLEW!");
    exit(EXIT_FAILURE);
  }

  if (!GLEW_ARB_draw_instanced) {
    fprintf(stderr, "ARB_draw_instanced is not supported; game may not "
                    "work properly!!\n");
    exit(1);
  }

  if (!GLEW_ARB_instanced_arrays) {
    fprintf(stderr,
            "ARB_instanced_arrays is not supported; game may not work "
            "properly!!\n");
    exit(1);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (GLEW_ARB_debug_output) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
  }

	const char *font_file = "/usr/share/fonts/liberation/LiberationMono-Italic.ttf";
	fr_init(&fr, font_file, SCREEN_WIDTH, SCREEN_HEIGHT);

  Vec2i cursor = vec2is(0);
  bool quit = false;
  while (!quit) {
    const Uint32 start = SDL_GetTicks();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        quit = true;
      } break;

      case SDL_KEYDOWN: {
        switch (event.key.keysym.sym) {
        case SDLK_BACKSPACE: {
          editor_backspace(&editor);
        } break;

        case SDLK_F2: {
          if (file_path) {
            editor_save_to_file(&editor, file_path);
          }
        } break;

        case SDLK_RETURN: {
          editor_insert_new_line(&editor);
        } break;

        case SDLK_DELETE: {
          editor_delete(&editor);
        } break;

        case SDLK_UP: {
          if (editor.cursor_row > 0) {
            editor.cursor_row -= 1;
          }
        } break;

        case SDLK_DOWN: {
          editor.cursor_row += 1;
        } break;

        case SDLK_LEFT: {
          if (editor.cursor_col > 0) {
            editor.cursor_col -= 1;
          }
        } break;

        case SDLK_RIGHT: {
          editor.cursor_col += 1;
        } break;
        }
      } break;

      case SDL_TEXTINPUT: {
        editor_insert_text_before_cursor(&editor, event.text.text);
      } break;

      case SDL_MOUSEBUTTONDOWN: {
        Vec2f mouse_click =
            vec2f((float)event.button.x, (float)event.button.y);
        switch (event.button.button) {
        case SDL_BUTTON_LEFT: {
          Vec2f cursor_click =
              vec2f_add(vec2f_mul(vec2f(1.0f, -1.0f),
                                  vec2f_sub(mouse_click,
                                            vec2f_div(window_size(window),
                                                      vec2f(2.0f, 2.0f)))),
                        camera_pos);
          if (cursor_click.x >= 0.0f &&
              cursor_click.y <= fr.glyph_info.ch * FONT_SCALE) {
            editor.cursor_col = (size_t)floorf(
                cursor_click.x / (fr.glyph_info.cw * FONT_SCALE));
            editor.cursor_row = (size_t)floorf(
                (cursor_click.y - fr.glyph_info.ch * FONT_SCALE) /
                (-1.0f * fr.glyph_info.ch * FONT_SCALE));
          }
        } break;
        }
      } break;

      case SDL_WINDOWEVENT: {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED: {
          Vec2f ws = window_size(window);
          glViewport(0, 0, ws.x, ws.y);
          glUniform2f(fr.resolution_uniform, ws.x, ws.y);
        } break;
        }
      } break;

      case SDL_KEYUP: {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          quit = true;
        }
      } break;
      }
    }

    {
      const Vec2f cursor_pos = vec2f(
          (float)editor.cursor_col * fr.glyph_info.cw,
          (float)(-(int)editor.cursor_row) * fr.glyph_info.ch);
      camera_vel =
          vec2f_mul(vec2f_sub(cursor_pos, camera_pos), vec2fs(2.0f));
      camera_pos =
          vec2f_add(camera_pos, vec2f_mul(camera_vel, vec2fs(DELTA_TIME)));
    }

    fr_glyph_buffer_clear(&fr);
    {
      for (size_t row = 0; row < editor.size; ++row) {
        const Line* line = editor.lines + row;
        fr_render_text_sized(&fr, line->chars, line->size, vec2f(0, -(int)row*fr.glyph_info.th),
                             vec4fs(1.0f), vec4fs(0.0f));
      }
    }
    fr_glyph_buffer_sync(&fr);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUniform1f(fr.time_uniform, (float)SDL_GetTicks() / 1000.0f);
    glUniform2f(fr.camera_uniform, camera_pos.x, camera_pos.y);
    glUniform2i(fr.cursor_uniform, cursor.x, cursor.y);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, fr.glyph_buffer_count);
    /* fr_glyph_buffer_clear(); */
    /* gl_render_cursor(&glyph_info, &editor); */
    /* glyph_buffer_sync(); */
    /* glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, glyph_buffer_count); */

    /* glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); */
    SDL_GL_SwapWindow(window);

    const Uint32 duration = SDL_GetTicks() - start;
    const Uint32 delta_time_ms = 1000 / FPS;
    if (duration < delta_time_ms) {
      SDL_Delay(delta_time_ms - duration);
    }
  }
  return 0;
}
