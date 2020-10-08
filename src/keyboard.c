#include "common/log.h"
#include "labwc.h"

static void
keyboard_modifiers_notify(struct wl_listener *listener, void *data)
{
	struct seat *seat = wl_container_of(listener, seat, keyboard_modifiers);
	wlr_seat_keyboard_notify_modifiers(seat->seat,
		&seat->keyboard_group->keyboard.modifiers);
}

static bool
handle_keybinding(struct server *server, uint32_t modifiers, xkb_keysym_t sym)
{
	struct keybind *keybind;
	wl_list_for_each_reverse (keybind, &rc.keybinds, link) {
		if (modifiers ^ keybind->modifiers) {
			continue;
		}
		for (size_t i = 0; i < keybind->keysyms_len; i++) {
			if (sym == keybind->keysyms[i]) {
				action(server, keybind->action,
				       keybind->command);
				return true;
			}
		}
	}
	return false;
}

static void
keyboard_key_notify(struct wl_listener *listener, void *data)
{
	/* This event is raised when a key is pressed or released. */
	struct seat *seat = wl_container_of(listener, seat, keyboard_key);
	struct server *server = seat->server;
	struct wlr_event_keyboard_key *event = data;
	struct wlr_seat *wlr_seat = server->seat.seat;
	struct wlr_input_device *device = seat->keyboard_group->input_device;

	/* Translate libinput keycode -> xkbcommon */
	uint32_t keycode = event->keycode + 8;
	/* Get a list of keysyms based on the keymap for this keyboard */
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(device->keyboard->xkb_state, keycode, &syms);

	bool handled = false;
	uint32_t modifiers =
		wlr_keyboard_get_modifiers(device->keyboard);

	if (server->cycle_view) {
		if ((syms[0] == XKB_KEY_Alt_L) &&
		    event->state == WLR_KEY_RELEASED) {
			/* end cycle */
			desktop_focus_view(&server->seat, server->cycle_view);
			server->cycle_view = NULL;
		} else if (event->state == WLR_KEY_PRESSED) {
			/* cycle to next */
			server->cycle_view =
				desktop_next_view(server, server->cycle_view);
			return;
		}
	}

	/* Handle compositor key bindings */
	if (event->state == WLR_KEY_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(server, modifiers, syms[i]);
		}
	}

	if (!handled) {
		/* Otherwise, we pass it along to the client. */
		wlr_seat_set_keyboard(wlr_seat, device);
		wlr_seat_keyboard_notify_key(wlr_seat, event->time_msec,
					     event->keycode, event->state);
	}
}

void
keyboard_init(struct seat *seat)
{
	seat->keyboard_group = wlr_keyboard_group_create();
	struct wlr_keyboard *kb = &seat->keyboard_group->keyboard;
	struct xkb_rule_names rules = { 0 };
	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules,
		XKB_KEYMAP_COMPILE_NO_FLAGS);
	wlr_keyboard_set_keymap(kb, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(kb, 25, 600);

	seat->keyboard_key.notify = keyboard_key_notify;
	wl_signal_add(&kb->events.key, &seat->keyboard_key);
	seat->keyboard_modifiers.notify = keyboard_modifiers_notify;
	wl_signal_add(&kb->events.modifiers, &seat->keyboard_modifiers);
}
