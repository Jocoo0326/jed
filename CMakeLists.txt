cmake_minimum_required(VERSION 3.10)

set(APP jed)
set(CMAKE_C_COMPILER "clang")

project(${APP})

find_package(Freetype REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)


set(SRC
  main.c la.c editor.c file.c gl_extra.c sdl_extra.c free_font.c cursor.c
  )

add_executable(${APP} ${SRC})

target_link_libraries(${APP}
  ${FREETYPE_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  -lm)

target_include_directories(${APP}
  PRIVATE
  ${FREETYPE_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  )

