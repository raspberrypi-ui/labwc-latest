#include "labwc.h"
#include "config/rcxml.h"

/* These functions could be extended to strength in the future. */
void
move_resistance(struct view *view, double *x, double *y, bool screen_edge)
{
	struct server *server = view->server;
	struct wlr_box mgeom;
	struct output *output;
	struct border border = view_border(view);
	int l, r, t, b; /* The edges of the current view */
	int tl, tr, tt, tb; /* The desired edges */
	int ml, mr, mt, mb; /* The edges of the monitor/other view */

	l = view->x - border.left - rc.gap;
	t = view->y - border.top - rc.gap;
	r = view->x + view->w + border.right + rc.gap;
	b = view->y + view->h + border.bottom + rc.gap;

	tl = *x - border.left - rc.gap;
	tt = *y - border.top - rc.gap;
	tr = *x + view->w + border.right + rc.gap;
	tb = *y + view->h + border.bottom + rc.gap;

	if (screen_edge) {
		if (!rc.screen_edge_strength) {
			return;
		}

		wl_list_for_each(output, &server->outputs, link) {
			mgeom = output_usable_area_in_layout_coords(output);
	
			ml = mgeom.x;
			mt = mgeom.y;
			mr = mgeom.x + mgeom.width;
			mb = mgeom.y + mgeom.height;
	
			if (l >= ml && tl < ml && tl >= ml
					- rc.screen_edge_strength) {
				*x = ml + border.left + rc.gap;
			} else if (r <= mr && tr > mr && tr <= mr
					+ rc.screen_edge_strength) {
				*x = mr - view->w - border.right - rc.gap;
			}

			if (t >= mt && tt < mt && tt >= mt
					- rc.screen_edge_strength) {
				*y = mt + border.top + rc.gap;
			} else if (b <= mb && tb > mb && tb <= mb
					+ rc.screen_edge_strength) {
				*y = mb - view->h - border.bottom - rc.gap;
			}
		}
	}
}

void
resize_resistance(struct view *view, struct wlr_box *new_view_geo,
		bool screen_edge)
{
	struct server *server = view->server;
	struct output *output;
	struct wlr_box mgeom;
	struct border border = view_border(view);
	int l, r, t, b; /* The edges of the current view */
	int tl, tr, tt, tb; /* The desired edges */
	int ml, mr, mt, mb; /* The edges of the monitor/other view */

	l = view->x - border.left - rc.gap;
	t = view->y - border.top - rc.gap;
	r = view->x + view->w + border.right + rc.gap;
	b = view->y + view->h + border.bottom + rc.gap;

	tl = new_view_geo->x - border.left - rc.gap;
	tt = new_view_geo->y - border.top - rc.gap;
	tr = new_view_geo->x + new_view_geo->width + border.right + rc.gap;
	tb = new_view_geo->y + new_view_geo->height + border.bottom + rc.gap;

	if (screen_edge) {
		if (!rc.screen_edge_strength) {
			return;
		}
		wl_list_for_each(output, &server->outputs, link) {
			mgeom = output_usable_area_in_layout_coords(output);
			ml = mgeom.x;
			mt = mgeom.y;
			mr = mgeom.x + mgeom.width;
			mb = mgeom.y + mgeom.height;

			if (server->resize_edges & WLR_EDGE_LEFT) {
				if (l >= ml && tl < ml && tl >= ml
						- rc.screen_edge_strength) {
					new_view_geo->x = ml + border.left
						+ rc.gap;
					new_view_geo->width = view->w;
				}
			} else if (server->resize_edges & WLR_EDGE_RIGHT) {
				if (r <= mr && tr > mr && tr <= mr
						+ rc.screen_edge_strength) {
					new_view_geo->width = mr - l
						- (border.right + rc.gap) * 2 ;
				}
			}

			if (server->resize_edges & WLR_EDGE_TOP) {
				if (t >= mt && tt < mt && tt >= mt
						- rc.screen_edge_strength) {
					new_view_geo->y = mt + border.top
						+ rc.gap;
					new_view_geo->height = view->h;
				}
			} else if (server->resize_edges & WLR_EDGE_BOTTOM) {
				if (b <= mb && tb > mb && tb <= mb
						+ rc.screen_edge_strength) {
					new_view_geo->height = mb - t
						- border.bottom - border.top
						- rc.gap * 2;
				}
			}
		}
	}
}