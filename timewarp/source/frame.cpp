
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <allegro.h>
#include "melee.h"
REGISTER_FILE
#include "frame.h"
#include "libs.h"

extern FILE* debug_file;
#define LIST_INCREMENT 10

int BACKGROUND_COLOR = 0;

int item_cmp(const void* p1, const void* p2)
{
	return( ( ((DirtyItem*) p1)->y) -  ( ((DirtyItem*) p2)->y) );
}

void erase_pixel(DirtyItem *item, BITMAP *frame)
{
	putpixel(frame, item->x, item->y, BACKGROUND_COLOR);
}

void draw_pixel(DirtyItem *item, BITMAP *frame, BITMAP *child)
{
	putpixel(child, item->x, item->y, getpixel(frame, item->x, item->y));
}

void erase_box(DirtyItem *item, BITMAP *frame)
{
    rectfill(frame, item->x, item->y, item->x + item->a - 1, item->y + item->b - 1, BACKGROUND_COLOR);
}

void draw_box(DirtyItem *item, BITMAP *frame, BITMAP *child)
{
	if((item->a == 1) && (item->b == 1))
		putpixel(child, item->x, item->y, getpixel(frame, item->x, item->y));
	else
		blit(frame, child, 
			item->x,
			item->y, 
			item->x, 
			item->y, 
			item->a, 
			item->b
		);
}

void erase_line(DirtyItem *item, BITMAP *frame)
{
	if((item->x == item->a) && (item->y == item->b))
		putpixel(frame, item->x, item->y, BACKGROUND_COLOR);
	else
		line(frame, item->x, item->y, item->a, item->b, BACKGROUND_COLOR);
}

static BITMAP *line_frame;

void line_pixel(BITMAP *line_child, int x, int y, int d)
{
	putpixel(line_child, x, y, getpixel(line_frame, x, y));
}

void erase_a_pixel(BITMAP *child, int x, int y, int d)
{
	putpixel(child, x, y, BACKGROUND_COLOR);
}

void erase_circle(DirtyItem *item, BITMAP *frame)
{
	do_circle(frame, item->x, item->y, item->a, 0, erase_a_pixel);
}

void draw_circle(DirtyItem *item, BITMAP *frame, BITMAP *child)
{
    line_frame = frame;
	do_circle(child, item->x, item->y, item->a, 0, line_pixel);
}

void draw_line(DirtyItem *item, BITMAP *frame, BITMAP *child)
{
	if((item->x == item->a) && (item->y == item->b))
		putpixel(child, item->x, item->y, getpixel(frame, item->x, item->y));
	else {
		line_frame = frame;
		do_line(child, item->x, item->y, item->a, item->b, 0, line_pixel);
	}
}


Frame::Frame(int max_items) {
	window = new VideoWindow();
	window->preinit();
	surface = NULL;
	full_redraw = 0;

	background_red = background_green = background_blue = 0;
	list_size = max_items;

	item_count = 0;
	item       = new DirtyItem[list_size];
	old_item_count = 0;
	old_item       = new DirtyItem[list_size];
	drawn_items = 0;

}

Frame::~Frame() {
	destroy_bitmap(surface);
	window->deinit();
	delete window;

	delete item;
	item_count = 0;

	delete old_item;
	old_item_count = 0;
}

void Frame::set_background ( int r, int g, int b) {
	background_red = r;
	background_green = g;
	background_blue = b;
	return;
}

void Frame::enlarge_list(int increment) {
		
	DirtyItem *temp;

	temp = item;
	item = new DirtyItem[list_size + increment];
	memcpy(item, temp, item_count * sizeof(DirtyItem));
	delete temp;

	temp = old_item;
	old_item = new DirtyItem[list_size + increment];
	memcpy(old_item, temp, old_item_count * sizeof(DirtyItem));
	delete temp;

	list_size += increment;
	return;
	}

void Frame::add_to_list(int x, int y, int a, int b,
  void (*erase_item)(DirtyItem *item, BITMAP *frame),
  void (*draw_item)(DirtyItem *item, BITMAP *frame, BITMAP *child))
{
	if(item_count == list_size)
		enlarge_list(LIST_INCREMENT);

	item[item_count].x = x;
	item[item_count].y = y;
	item[item_count].a = a;
	item[item_count].b = b;

	item[item_count].erase_item = erase_item;
	item[item_count].draw_item  = draw_item;

	item_count++;
}

void Frame::add_to_old_list(int x, int y, int a, int b,
  void (*erase_item)(DirtyItem *item, BITMAP *frame),
  void (*draw_item)(DirtyItem *item, BITMAP *frame, BITMAP *child))
{
	if(old_item_count == list_size)
		enlarge_list(LIST_INCREMENT);

	old_item[old_item_count].x = x;
	old_item[old_item_count].y = y;
	old_item[old_item_count].a = a;
	old_item[old_item_count].b = b;

	old_item[old_item_count].erase_item = erase_item;
	old_item[old_item_count].draw_item  = draw_item;

	old_item_count++;
}

void Frame::add_pixel(int x, int y)
{
	add_to_list(x, y, 0, 0, erase_pixel, draw_pixel);
}

void Frame::add_box(int x, int y, int a, int b)
{
	add_to_list(x, y, a, b, erase_box, draw_box);
}

void Frame::add_circle(int x, int y, int a, int b)
{
	add_to_list(x, y, a, b, erase_circle, draw_circle);
}

void Frame::add_old_circle(int x, int y, int a, int b)
{
	add_to_old_list(x, y, a, b, erase_circle, draw_circle);
}

void Frame::add_line(int x, int y, int a, int b)
{
	add_to_list(x, y, a, b, erase_line, draw_line);
}

void Frame::add_old_pixel(int x, int y)
{
	add_to_old_list(x, y, 0, 0, erase_pixel, draw_pixel);
}

void Frame::add_old_box(int x, int y, int a, int b)
{
	add_to_old_list(
		x, 
		y, 
		a, 
		b, 
		erase_box, draw_box
	);
}

void Frame::add_old_line(int x, int y, int a, int b)
{
	add_to_old_list(x, y, a, b, erase_line, draw_line);
}

void Frame::erase()
{
	int c;
	if (!surface) return;
	BACKGROUND_COLOR = makecol ( background_red, background_green, background_blue );

	//window->parent->update_pos();//WTF?  why did I do that?

	if (full_redraw) {
		clear_to_color(surface, BACKGROUND_COLOR);
		old_item_count = 0;
		item_count = 0;
		return;
	}

	for(c = 0; c < item_count; c++) {
		item[c].erase_item(&item[c], surface);
	}

	if (full_redraw) {
		item_count = 0;
		return;
	}

	if (old_item_count == 0) {
		DirtyItem *tmp = old_item;
		old_item = item;
		item = tmp;
		old_item_count = item_count;
		item_count = 0;
		drawn_items = 0;
	}
	else {
		if (old_item_count + item_count > list_size) {
			full_redraw = true;
		}
		memcpy ( &old_item[old_item_count], &item[0], item_count * sizeof(DirtyItem));
		old_item_count += item_count;
		item_count = 0;
		drawn_items = 0;
	}

/*	item_count = 0;
	drawn_items = 0;
	old_item_count = 0;
	full_redraw = true;*/

	return;
}

void Frame::prepare () {
	int w = 0, h = 0;
	if (surface) {
		w = surface->w; 
		h = surface->h;
	}
	if ((window->w != w) || (window->h != h)) {
		if (surface) destroy_bitmap(surface);
		surface = NULL;
		w = window->w;
		h = window->h;
		if (w && h) {
			surface = create_bitmap(w, h);
			clear_to_color(surface, BACKGROUND_COLOR);
		}
		full_redraw = true;
	}
}

void Frame::draw()
{
	int c;
	BACKGROUND_COLOR = tw_color ( background_red, background_green, background_blue );

	BITMAP *tmp = window->surface;
	if (!tmp) return;

	prepare();

	if (!surface) {
		old_item_count = 0;
		item_count = 0;
		drawn_items = 0;
		return;
	}

	if (full_redraw) {
		old_item_count = 0;
		window->lock();
		blit(surface, tmp, 0, 0, window->x, window->y, window->w, window->h);
		window->unlock();
		full_redraw -= 1;
	}
	else {

		tmp = create_sub_bitmap (
			tmp, window->x, window->y, window->w, window->h);
		acquire_bitmap(tmp);

		for(c = drawn_items; c < item_count; c++) {
			item[c].draw_item(&item[c], surface, tmp);
		}
		for(c = 0; c < old_item_count; c++) {
			old_item[c].draw_item(&old_item[c], surface, tmp);
		}

		release_bitmap(tmp);
		destroy_bitmap(tmp);

		//drawn_items = item_count;
		old_item_count = 0;
	}
	return;
}


/*
struct ShortIntRect {
	short int x, y, w, h;
};

class Frame2 {

	//front surface
	BITMAP *front;

	//back surface
	BITMAP *back;

	//background / clear
	Color background_color;
	BITMAP *background_image;

	//mode
	int full_redraw;//0 = false, 1 = this frame only, -1 = always
	char clear;     //bit 0: 0=off,1=on;   bit 1: 0=flat color, 1=bitmap

	//clipping region
	int x_min, x_max; //x_min inclusive, x_max exclusive
	int y_min, y_max; //y_min inclusive, y_max exclusive

	//dirtyness:
	//dirty rectangles
	int num_dirty_rects;
	ShortIntRect *dirty_rects;
	//dirty non-rectangles
	int num_dirty_items;
	DirtyItem *dirty_items;

	//clears:
	//clear rectangles
	int num_clear_rects;
	ShortIntRect *clear_rects;
	//clear non-rectangles
	int num_clear_items;
	ShortIntRect *clear_items;
};
*/
