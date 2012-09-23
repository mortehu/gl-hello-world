#include <err.h>
#include <string.h>
#include <stdio.h>
#include <sysexits.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "x11.h"

static unsigned int bufferWidth, bufferHeight;
static unsigned char *buffer;

static void
paint (void)
{
  unsigned int x, y, i;

  for (y = 0, i = 0; y < bufferHeight; ++y)
    {
      for (x = 0; x < bufferWidth; ++x, i += 4)
        {
          buffer[i] = x * 255 / (bufferWidth - 1);
          buffer[i + 1] = y * 255 / (bufferHeight - 1);
          buffer[i + 2] = 0;
        }
    }
}

int
main (int argc, char **argv)
{
  X11_Init ();

  bufferWidth = X11_window_width;
  bufferHeight = X11_window_height;

  if (!(buffer = malloc (bufferWidth * bufferHeight * 4)))
    err (EX_OSERR, "malloc failed");

  for (;;)
    {
      X11_ProcessEvents ();

      glClear (GL_COLOR_BUFFER_BIT);

      paint ();

      glDrawPixels (bufferWidth, bufferHeight, GL_BGRA, GL_UNSIGNED_BYTE, buffer);

      X11_SwapBuffers();
    }
}
