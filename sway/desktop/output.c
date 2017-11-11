#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <time.h>
#include <wayland-server.h>
#include <wlr/types/wlr_output.h>
#include <wlr/render.h>
#include "sway/server.h"
#include "sway/container.h"
#include "sway/workspace.h"
#include "sway/output.h"
#include "log.h"

static void output_frame_notify(struct wl_listener *listener, void *data) {
	struct sway_output *soutput = wl_container_of(
			listener, soutput, frame);
	struct wlr_output *wlr_output = data;
	struct sway_server *server = soutput->server;

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	wlr_output_make_current(wlr_output);
	wlr_renderer_begin(server->renderer, wlr_output);

	wlr_renderer_end(server->renderer);
	wlr_output_swap_buffers(wlr_output);

	soutput->last_frame = now;
}

void output_add_notify(struct wl_listener *listener, void *data) {
	struct sway_server *server = wl_container_of(listener, server, output_add);
	struct wlr_output *wlr_output = data;
	sway_log(L_DEBUG, "New output %p: %s", wlr_output, wlr_output->name);

	struct sway_output *output = calloc(1, sizeof(struct sway_output));
	output->wlr_output = wlr_output;
	output->server = server;

	swayc_t *node = new_output(output);
	if (!sway_assert(node, "Failed to allocate output")) {
		return;
	}

	// Switch to workspace if we need to
	if (swayc_active_workspace() == NULL) {
		swayc_t *ws = node->children->items[0];
		workspace_switch(ws);
	}

	output->frame.notify = output_frame_notify;
	wl_signal_add(&wlr_output->events.frame, &output->frame);
}

void output_remove_notify(struct wl_listener *listener, void *data) {
	struct sway_server *server = wl_container_of(listener, server, output_remove);
	struct wlr_output *wlr_output = data;
	sway_log(L_DEBUG, "Output %p %s removed", wlr_output, wlr_output->name);
	// TODO
}