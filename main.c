#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <sysexits.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "x11.h"

/**
 * string: the shader source code
 * type: GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
 */
static GLuint
load_shader (const char *error_context, const char *string, GLenum type)
{
  GLuint result;
  GLint string_length, compile_status;

  result = glCreateShader (type);

  string_length = strlen (string);
  glShaderSource (result, 1, &string, &string_length);
  glCompileShader (result);

  glGetShaderiv (result, GL_COMPILE_STATUS, &compile_status);

  if (compile_status != GL_TRUE)
    {
      GLchar log[1024];
      GLsizei logLength;
      glGetShaderInfoLog (result, sizeof (log), &logLength, log);

      errx (EXIT_FAILURE, "%s: glCompileShader failed: %.*s",
            error_context, (int) logLength, log);
    }

  return result;
}

struct shader
{
  GLuint handle;
  GLuint vertex_position_attribute;
  GLuint texture_coord_attribute;
};

static struct shader
load_program (const char *vertex_shader_source,
              const char *fragment_shader_source)
{
  GLuint vertex_shader, fragment_shader;
  struct shader result;
  GLint link_status;

  vertex_shader = load_shader ("vertex", vertex_shader_source, GL_VERTEX_SHADER);
  fragment_shader = load_shader ("fragment", fragment_shader_source, GL_FRAGMENT_SHADER);

  result.handle = glCreateProgram ();
  glAttachShader (result.handle, vertex_shader);
  glAttachShader (result.handle, fragment_shader);
  glLinkProgram (result.handle);

  glGetProgramiv (result.handle, GL_LINK_STATUS, &link_status);

  if (link_status != GL_TRUE)
    {
      GLchar log[1024];
      GLsizei logLength;
      glGetProgramInfoLog (result.handle, sizeof (log), &logLength, log);

      errx (EXIT_FAILURE, "glLinkProgram failed: %.*s",
            (int) logLength, log);
    }

  glUseProgram (result.handle);

  result.vertex_position_attribute = glGetAttribLocation (result.handle, "attr_VertexPosition");
  result.texture_coord_attribute = glGetAttribLocation (result.handle, "attr_TextureCoord");

  return result;
}

static GLuint
create_texture (const void *data, unsigned int width, unsigned int height)
{
  GLuint result;

  glGenTextures (1, &result);
  glBindTexture (GL_TEXTURE_2D, result);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, data);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static GLuint
create_gradient_texture (void)
{
  const unsigned int bufferWidth = 256, bufferHeight = 256;
  unsigned char *buffer;
  unsigned int x, y, i;

  GLuint result;

  if (!(buffer = malloc (bufferWidth * bufferHeight * 4)))
    err (EX_OSERR, "malloc failed");

  for (y = 0, i = 0; y < bufferHeight; ++y)
    {
      for (x = 0; x < bufferWidth; ++x, i += 4)
        {
          buffer[i] = x * 255 / (bufferWidth - 1);
          buffer[i + 1] = y * 255 / (bufferHeight - 1);
          buffer[i + 2] = 0;
          buffer[i + 3] = 0xff;
        }
    }

  result = create_texture (buffer, bufferWidth, bufferHeight);

  free (buffer);

  return result;
}

int
main (int argc, char **argv)
{
  static const char *vertex_shader_source =
    "attribute vec2 attr_VertexPosition;\n"
    "attribute vec2 attr_TextureCoord;\n"
    "uniform vec2 uniform_WindowSize;\n"
    "varying vec2 var_TextureCoord;\n"
    "void main (void)\n"
    "{\n"
    "  gl_Position = vec4(-1.0 + (attr_VertexPosition.x * uniform_WindowSize.x) * 2.0,\n"
    "                      1.0 - (attr_VertexPosition.y * uniform_WindowSize.y) * 2.0, 0.0, 1.0);\n"
    "  var_TextureCoord = attr_TextureCoord;\n"
    "}";

  static const char *fragment_shader_source =
    "varying vec2 var_TextureCoord;\n"
    "uniform sampler2D uniform_Sampler;\n"
    "void main (void)\n"
    "{\n"
    "  gl_FragColor = texture2D (uniform_Sampler, vec2 (var_TextureCoord.s, var_TextureCoord.t));\n"
    "}";

  GLuint texture;
  struct shader shader;

  X11_Init ();

  shader = load_program (vertex_shader_source, fragment_shader_source);
  glUniform2f (glGetUniformLocation (shader.handle, "uniform_WindowSize"),
               1.0f / X11_window_width,
               1.0f / X11_window_height);
  glUniform1i (glGetUniformLocation (shader.handle, "uniform_Sampler"),
               0);

  texture = create_gradient_texture ();

  glDisable(GL_CULL_FACE);

  for (;;)
    {
      X11_ProcessEvents ();

      glClear (GL_COLOR_BUFFER_BIT);

      glBegin (GL_QUADS);
      glVertexAttrib2f (shader.texture_coord_attribute, 0.0f, 0.0f); glVertexAttrib2f (shader.vertex_position_attribute, 0.0f, 0.0f);
      glVertexAttrib2f (shader.texture_coord_attribute, 0.0f, 1.0f); glVertexAttrib2f (shader.vertex_position_attribute, 0.0f, X11_window_height);
      glVertexAttrib2f (shader.texture_coord_attribute, 1.0f, 1.0f); glVertexAttrib2f (shader.vertex_position_attribute, X11_window_width, X11_window_height);
      glVertexAttrib2f (shader.texture_coord_attribute, 1.0f, 0.0f); glVertexAttrib2f (shader.vertex_position_attribute, X11_window_width, 0.0f);
      glEnd ();

      X11_SwapBuffers ();
    }
}
