#include "backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <time.h>
#include <xcb/xcb.h>

static xcb_connection_t *conn = NULL;
static xcb_gcontext_t foreground = 0;
static xcb_window_t window = 0;

static xcb_intern_atom_cookie_t cookie, cookie2;
static xcb_intern_atom_reply_t *reply, *reply2;

void open_window()
{
	/* Grab the screen. */
	conn = xcb_connect(NULL, NULL);
	const xcb_setup_t *setup = xcb_get_setup(conn);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	xcb_screen_t *screen = iter.data;
	/* Create graphics context. */
	{
		window = screen->root;
		foreground = xcb_generate_id(conn);
		const uint32_t mask = XCB_GC_FOREGROUND
				    | XCB_GC_GRAPHICS_EXPOSURES;
		const uint32_t values[2] = {
			[0] = screen->black_pixel,
			[1] = 0
		};
		xcb_create_gc(conn, foreground, window, mask, values);
	}
	/* Create a window. */
	{
		window = xcb_generate_id(conn);
		const uint32_t mask = XCB_CW_BACK_PIXEL
				    | XCB_CW_EVENT_MASK;
		const uint32_t values[2] = {
			[0] = screen->white_pixel,
			[1] = XCB_EVENT_MASK_EXPOSURE
			    | XCB_EVENT_MASK_KEY_PRESS
		};
		xcb_create_window(conn,
				  XCB_COPY_FROM_PARENT, /* depth */
				  window, /* window id */
				  screen->root, /* parent window */
				  0, 0, /* starting position */
				  64, 32, /* dimensions */
				  10, /* border width */
				  XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class */
				  screen->root_visual, /* visual */
				  mask, values); /* unused masks */

	}
	/* Reply ref for window closing. */
	{
		cookie = xcb_intern_atom(conn, 1, 12, "WM_PROTOCOLS");
		reply = xcb_intern_atom_reply(conn, cookie, 0);
		cookie2 = xcb_intern_atom(conn, 0, 16, "WM_DELETE_WINDOW");
		reply2 = xcb_intern_atom_reply(conn, cookie2, 0);
		xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
				    reply->atom, 4, 32, 1, &reply2->atom);
	}
	xcb_map_window(conn, window);
	xcb_flush(conn);
}

void close_window()
{
	xcb_disconnect(conn);
}

bool keys(chip8_t *c)
{
	static bool ks[0xff];
	memset(ks, false, sizeof(bool) * 0xff);
	xcb_generic_event_t *event = NULL;
	/* Event loop. */
	while ((event = xcb_poll_for_event(conn))) {
		switch (event->response_type & ~0x80) {
		case XCB_CLIENT_MESSAGE:
			if (((xcb_client_message_event_t*)event)->data.data32[0]
			    == reply2->atom)
				return true;
			break;
		case XCB_KEY_PRESS:
			ks[((xcb_key_press_event_t*)event)->detail] = true;
			break;
		default:
			break;
		}
		free(event);
	}
	/* Copy key states. */
	c->key[0x0] = ks[59]; /* , */
	c->key[0x1] = ks[16]; /* 7 */
	c->key[0x2] = ks[17]; /* 8 */
	c->key[0x3] = ks[18]; /* 9 */
	c->key[0x4] = ks[30]; /* u */
	c->key[0x5] = ks[31]; /* i */
	c->key[0x6] = ks[32]; /* o */
	c->key[0x7] = ks[44]; /* j */
	c->key[0x8] = ks[45]; /* k */
	c->key[0x9] = ks[46]; /* l */
	c->key[0xa] = ks[58]; /* m */
	c->key[0xb] = ks[60]; /* . */
	c->key[0xc] = ks[19]; /* 0 */
	c->key[0xd] = ks[33]; /* p */
	c->key[0xe] = ks[47]; /* ; */
	c->key[0xf] = ks[61]; /* / */
	return false;
}

void draw(const chip8_t *c)
{
	/* Clear screen. */
	xcb_clear_area(conn, 0, window, 0, 0, 64, 32);
	/* Find and draw pixels. */
	static xcb_point_t point_buf[64 * 32];
	int numpoint = 0;
	for (int y = 0; y < 32; y++)
		for (int x = 0; x < 64; x++)
			if (c->display[y * 64 + x]) {
				point_buf[numpoint] = (xcb_point_t) {
					.x = x,
					.y = y
				};
				numpoint++;
			}
	xcb_poly_point(conn, XCB_COORD_MODE_ORIGIN, window, foreground,
		       numpoint, point_buf);
}

void sync()
{
	/* Draw to window. */
	xcb_flush(conn);
	/* Delay ~16 milliseconds == 60Hz. */
	const struct timespec req = {
		.tv_sec = 0,
		.tv_nsec = 16 * 1000 * 1000,
	};
	struct timespec rem;
	const int s = nanosleep(&req, &rem);
	if (s == -1)
		switch (errno) {
		case EFAULT:
			puts("EFAULT");
			break;
		case EINTR:
			puts("EINTER");
			break;
		case EINVAL:
			puts("EINVAL");
			break;
		default:
			break;
		}
}
