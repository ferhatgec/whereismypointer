/* MIT License
#
# Copyright (c) 2020 Ferhat Geçdoğan All Rights Reserved.
# Distributed under the terms of the MIT License.
#
# */

#define _POSIX_C_SOURCE 200809L  /* for clock_* */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <X11/Xlib.h>
#include <string.h>

Display *dpy;
Window root;
GC gc;
Window locators[4] = {0};
int screen;
double distance = 20, speed = 70;
double time_start;
int window_size = 3;
uint32_t argb = 0x00FF0000;

Window create_window(void) {
    Window w;
    XSetWindowAttributes wa = {
        .override_redirect = True,
        .background_pixmap = ParentRelative,
        .event_mask = ExposureMask,
    };

    w = XCreateWindow(dpy, root, -10, -10, 5, 5, 0,
                      DefaultDepth(dpy, screen),
                      CopyFromParent, DefaultVisual(dpy, screen),
                      CWOverrideRedirect | CWBackPixmap | CWEventMask,
                      &wa);
    XMapRaised(dpy, w);

    return w;
}

bool update(void) {
    struct timespec now;
    double time_now, time_passed, dist_now;
    size_t i;
    int x, y, di;
    unsigned int dui;
    Window dummy;

    XQueryPointer(dpy, root, &dummy, &dummy, &x, &y, &di, &di, &dui);

    clock_gettime(CLOCK_MONOTONIC, &now);
    time_now = now.tv_sec + now.tv_nsec / 1e9;
    time_passed = time_now - time_start;

    dist_now = distance - time_passed * speed;
    if (dist_now < window_size)
        return true;

    if ((argb & 0xFF) != 0)
        argb = 0x00FF0000;
    else
        argb >>= 8;

    for (i = 0; i < 4; i++) {
        if (locators[i] == 0)
            locators[i] = create_window();
	/* Locators. */
        switch (i) {
            case 0:
                XMoveResizeWindow(dpy, locators[i],
                                  x - dist_now,
                                  y - dist_now,
                                  2 * dist_now,
                                  window_size);
                break;
            case 1:
                XMoveResizeWindow(dpy, locators[i],
                                  x - dist_now,
                                  y + dist_now,
                                  2 * dist_now,
                                  window_size);
                break;
            case 2:
                XMoveResizeWindow(dpy, locators[i],
                                  x - dist_now,
                                  y - dist_now + window_size,
                                  window_size,
                                  2 * dist_now - window_size);
                break;
            case 3:
                XMoveResizeWindow(dpy, locators[i],
                                  x + dist_now - window_size,
                                  y - dist_now + window_size,
                                  window_size,
                                  2 * dist_now - window_size);
                break;
        }
    }

    return false;
}

void draw(XEvent *ev) {
    XExposeEvent *e = &ev->xexpose;

    XSetForeground(dpy, gc, argb);
    XFillRectangle(dpy, e->window, gc, e->x, e->y, e->width, e->height);
}

void HelpFunction() {
	printf("WhereIsMyPointer?\n");
        printf("wimpointer --l || --loop : Find your pointer with color loop.\n");
	printf("wimpointer --c || --custom : Customize WhereIsMyPointer.\n");
}

int main(int argc, char* argv[]) {
    /* HelpFunction() */
    if(argc == 1) {
	HelpFunction();
	return 0;
    }
	
    char* settings = argv[1];
    int sret, xfd;
    XEvent ev;
    fd_set fds;
    struct timespec start;
    struct timeval timeout;
    int fps = 60;
    bool loop = false;
    (void)argc;
    (void)argv;
    char ch;

    printf(settings);
    if(strstr(settings, "--l")) {
	loop = true;
    } else if(strstr(settings, "--c")) {
	printf("FPS : ");	
	scanf("%d", &fps);
    	printf("Loop (y | n) : ");
	scanf(" %c", &ch);
	if(ch == 'y') {
		loop = true;
	} else if(ch == 'n') {
		loop = false;
	} else {
		loop = false;
	}
	printf("Window size : ");
	scanf("%d", &window_size);
	printf("Distance : ");
	scanf("%lf", &distance);
	printf("Speed : ");
	scanf("%lf", &speed);
	printf("Ok");
    }

    dpy = XOpenDisplay(NULL);
    test: if (!dpy) { /* Loop. */
        fprintf(stderr, "WhereIsMyPointer: Cannot open display\n");
        exit(EXIT_FAILURE);
    }
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    gc = XCreateGC(dpy, root, 0, NULL);
    xfd = ConnectionNumber(dpy);

    clock_gettime(CLOCK_MONOTONIC, &start);
    time_start = start.tv_sec + start.tv_nsec / 1e9;

    for (;;) {
        FD_ZERO(&fds);
        FD_SET(xfd, &fds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 1000000 / fps;

        sret = select(xfd + 1, &fds, NULL, NULL, &timeout);
        if (sret == -1) {
            perror("WhereIsMyPointer: select() returned with error");
            exit(EXIT_FAILURE);
        }
        else if (sret == 0)

            if (update()) {
		if(loop == true) {		
			goto test;
		} else {
			exit(EXIT_SUCCESS);
		}
	    }

        while (XPending(dpy)) {
            XNextEvent(dpy, &ev);
            switch (ev.type) {
                case Expose:
                    draw(&ev);
                    break;
            }
        }
    }
}
