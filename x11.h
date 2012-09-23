#ifndef X11_H_
#define X11_H_ 1

extern unsigned int X11_window_width;
extern unsigned int X11_window_height;

void
X11_Init (void);

void
X11_ProcessEvents (void);

void
X11_SwapBuffers(void);

#endif /* !X11_H_ */
