#include "glad/glad.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "client/constants.h"
#include "client/line_render.h"
#include "client/linmath.h"
#include "client/shader.h"
#include "client/text_render.h"
#include "client/vertex.h"
#include "skat/str_buf.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

float screen_width = WIDTH;
float screen_height = HEIGHT;
str_buf input;

static void
error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s (%d)\n", description, error);
}

static void
resize_callback(GLFWwindow *window, int width, int height) {
  if (height == 0)
	height = 1;// To prevent divide by 0

  GLfloat desiredAspect = (GLfloat) HEIGHT / WIDTH;
  GLfloat aspect = (GLfloat) height / (GLfloat) width;

  GLfloat actualWidth, actualHeight, x, y;

  if (aspect == desiredAspect) {// perfect ratio
	actualWidth = width;
	actualHeight = height;
	x = 0;
	y = 0;
  } else if (aspect < desiredAspect) {// window too high
	actualWidth = (float) height / desiredAspect;
	actualHeight = height;
	x = ((float) width - actualWidth) / 2.0f;
	y = 0;
  } else {// window too broad
	actualWidth = width;
	actualHeight = (float) width * desiredAspect;
	x = 0;
	y = ((float) height - actualHeight) / 2.0f;
  }

  glViewport(x, y, actualWidth, actualHeight);

  screen_width = (float) width;
  screen_height = (float) height;

  printf("Resizing to %d x %d\n", width, height);
}

static void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  const char *action_name = action == 0   ? "release"
							: action == 1 ? "press"
							: action == 2 ? "repeat"
										  : "unknown";
  const char *key_name = glfwGetKeyName(key, scancode);
  int shift = (mods & GLFW_MOD_SHIFT) != 0;
  int ctrl = (mods & GLFW_MOD_CONTROL) != 0;
  int alt = (mods & GLFW_MOD_ALT) != 0;
  int super = (mods & GLFW_MOD_SUPER) != 0;
  int caps_lock = (mods & GLFW_MOD_CAPS_LOCK) != 0;
  int num_lock = (mods & GLFW_MOD_NUM_LOCK) != 0;
  printf("key_callback: key=%s (%d) scancode=%d action=%s mods=[shift=%d, "
		 "ctrl=%d, alt=%d, super=%d, caps_lock=%d, num_lock=%d] (0x%x)\n",
		 key_name, key, scancode, action_name, shift, ctrl, alt, super,
		 caps_lock, num_lock, mods);
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	glfwSetWindowShouldClose(window, GLFW_TRUE);
  else if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
	if (input.len > 0)
	  str_buf_remove(&input, 1);
  } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	str_buf_empty(&input);
}

static void
char_callback(struct GLFWwindow *window, unsigned int codepoint) {
  if (codepoint >= 32 && codepoint <= 126) {// printable ascii character
	printf("char_callback: '%c'\n", codepoint);
	if (input.len >= 31) {
	  str_buf_empty(&input);
	}
	str_buf_append_char(&input, (char) codepoint);
  } else {
	printf("char_callback: (0x%x)\n", codepoint);
  }
}

static void
draw_cross() {
  render_line(GRAY, 0, 0, WIDTH, HEIGHT);
  render_line(BLUE, 0, HEIGHT, WIDTH, 0);

  render_box(RED, WIDTH - 1, HEIGHT - 1, -20, -20);
}

static struct {
  GLuint vertex_buffer;
  shader *program;
  GLint proj_location, model_location, vpos_location, vcol_location;
} triangle_data;

static void
preprare_triangle(void) {
  vertex2f_rgb triangle_vertices[3] = {{0.0f, -0.6f, 1.f, 0.f, 0.f},
									   {-0.6f, 0.4f, 0.f, 1.f, 0.f},
									   {0.6f, 0.4f, 0.f, 0.f, 1.f}};

  // NOTE: OpenGL error checks have been omitted for brevity
  triangle_data.program = shader_create_load_file("./shader/test");
  shader_use(triangle_data.program);

  triangle_data.proj_location =
		  shader_get_uniform_location(triangle_data.program, "projection");
  triangle_data.model_location =
		  shader_get_uniform_location(triangle_data.program, "model");
  triangle_data.vpos_location =
		  shader_get_attrib_location(triangle_data.program, "vPos");
  triangle_data.vcol_location =
		  shader_get_attrib_location(triangle_data.program, "vCol");

  glGenBuffers(1, &triangle_data.vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, triangle_data.vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices,
			   GL_STATIC_DRAW);

  mat4x4 projection;
  mat4x4_identity(projection);
  mat4x4_ortho(projection, 0, WIDTH, HEIGHT, 0, -1, 1);
  glUniformMatrix4fv(triangle_data.proj_location, 1, GL_FALSE,
					 (const GLfloat *) projection);
}

static void
draw_triangle(void) {
  shader_use(triangle_data.program);
  glBindBuffer(GL_ARRAY_BUFFER, triangle_data.vertex_buffer);

  glEnableVertexAttribArray(triangle_data.vpos_location);
  glVertexAttribPointer(triangle_data.vpos_location, 2, GL_FLOAT, GL_FALSE,
						sizeof(vertex2f_rgb), 0);
  glEnableVertexAttribArray(triangle_data.vcol_location);
  glVertexAttribPointer(triangle_data.vcol_location, 3, GL_FLOAT, GL_FALSE,
						sizeof(vertex2f_rgb),
						(const void *) (sizeof(GLfloat) * 2));

  mat4x4 model;
  mat4x4_identity(model);
  mat4x4_translate_in_place(model, WIDTH / 2.0f, HEIGHT / 2.0f, 0);
  mat4x4_rotate_Z(model, model, (float) glfwGetTime());
  mat4x4_scale_aniso(model, model, HEIGHT / 2.0f, HEIGHT / 2.0f, 1);
  glUniformMatrix4fv(triangle_data.model_location, 1, GL_FALSE,
					 (const GLfloat *) model);

  glDrawArrays(GL_TRIANGLES, 0, 3);
}

int
start_GRAPHICAL(void) {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
	exit(EXIT_FAILURE);

  // DO NOT UPDATE: indirect drawing via X Forwarding does not support higher GL
  // versions :(
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  // glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Skat", NULL, NULL);
  if (!window) {
	printf("Failed to create GLFW window\n");
	glfwTerminate();
	exit(EXIT_FAILURE);
  }

  glfwSetWindowSizeCallback(window, resize_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCharCallback(window, char_callback);

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
	printf("Failed to initialize OpenGL context\n");
	glfwTerminate();
	exit(EXIT_FAILURE);
  }

  printf("This is OpenGL version %s with renderer %s\n",
		 glGetString(GL_VERSION), glGetString(GL_RENDERER));

  glfwSwapInterval(1);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  str_buf_new_from_char(&input, "TeQuBrFoJuOpThLgDq0123456789 /j");
  line_render_init();
  text_render_init();
  preprare_triangle();

  while (!glfwWindowShouldClose(window)) {
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	draw_cross();

	draw_triangle();

	render_line(GREEN, 10, 8, 900, 8);
	text_render_print(TRL_TOP_LEFT, GREEN, 10.0f, 8.0f, 1.0f,
					  "TeQuBrFoJuOvThLaDo! /j");

	render_line(BLUE, 10, 96, 900, 96);
	text_render_print(TRL_BOTTOM_LEFT, BLUE, 10.0f, 96.0f, 1.0f,
					  "TeQuBrFoJuOvThLaDo? /j");

    render_line(BLACK, 690, 150, 1222, 150);
	text_render_debug(700, 150.0f, 1.0f);

	render_box(RED, 9, 650 - (18 / 2), 599, 18);
	render_line(GRAY, 9 - 1, 650, (9 - 1) + 599 + 2, 650);
	text_render_print(TRL_CENTER_LEFT, BLACK, 10.0f, 650.0f, 0.5f, "%s",
					  input.buf);

	glfwSwapBuffers(window);
	glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
