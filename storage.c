#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "storage.h"
#include "asc.h"
#include "context_menu.h"
#include "dialogues.h"
#include "elwindows.h"
#include "filter.h"
#include "gamewin.h"
#include "hud.h"
#include "init.h"
#include "items.h"
#include "item_info.h"
#include "item_lists.h"
#include "misc.h"
#include "multiplayer.h"
#include "named_colours.h"
#include "sound.h"
#include "textures.h"
#include "translate.h"
#include "widgets.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

#define STORAGE_CATEGORIES_SIZE 50
#define STORAGE_CATEGORIES_DISPLAY 13
#define STORAGE_SCROLLBAR_CATEGORIES 1200
#define STORAGE_SCROLLBAR_ITEMS 1201

struct storage_category {
	char name[25];
	int id;
	int color;
} storage_categories[STORAGE_CATEGORIES_SIZE];

int no_storage_categories=0;
int selected_category=-1;
int view_only_storage=0;
Uint32 drop_fail_time = 0;
int sort_storage_categories = 0;

int active_storage_item=-1;

ground_item storage_items[STORAGE_ITEMS_SIZE]={{0,0,0}};
int no_storage;

#define MAX_DESCR_LEN 202
char storage_text[MAX_DESCR_LEN]={0};
static char last_storage_text[MAX_DESCR_LEN]={0};
static char wrapped_storage_text[MAX_DESCR_LEN+10]={0};

static int print_quanities[STORAGE_ITEMS_SIZE];
static int number_to_print = 0;
static int next_item_to_print = 0;
static int printing_category = -1;
static int mouse_over_titlebar = 0;
static int mouse_over_storage = 0;

int disable_storage_filter = 0;
static char filter_item_text[40] = "";
static size_t filter_item_text_size = 0;
static Uint8 storage_items_filter[STORAGE_ITEMS_SIZE];

//	Look though the category for the selected item, pick it up if found.
//
static void select_item(int image_id, Uint16 item_id)
{
	int i;
	int found_at = -1;
	
	for (i=0; i<no_storage; i++)
	{
		if ((item_id != unset_item_uid) && (storage_items[i].id != unset_item_uid) && (storage_items[i].quantity > 0))
		{
			if (storage_items[i].id == item_id)
			{
				found_at = i;
				break;
			}
		}
		else
		if ((storage_items[i].image_id == image_id) && (storage_items[i].quantity > 0))
		{
			found_at = i;
			break;
		}
	}

	if (found_at < 0)
	{
		do_alert1_sound();
		item_lists_reset_pickup_fail_time();
	}
	else
	{
		active_storage_item=storage_items[found_at].pos;
		if (!view_only_storage)
		{
			storage_item_dragged=found_at;
			do_drag_item_sound();
		}
		else
		{
			do_click_sound();
		}
	}
}


static int wanted_category = -1;
static Uint16 wanted_item_id = -1;
static int wanted_image_id = -1;
static void move_to_category(int cat);
static int find_category(int id);


//	Called when the category is changed.
//	- Update the store of items/category.
//	- If in the process of picking up an item go for it if this is the category.
//
static void category_updated(void)
{
	int i;
	for (i=0; i<no_storage; i++)
		update_category_maps(storage_items[i].image_id, storage_items[i].id, storage_categories[selected_category].id);
	if (selected_category == wanted_category)
	{
		select_item(wanted_image_id, wanted_item_id);
		wanted_category = -1;
	}
}


//	Start the process of picking up the specified item from a specified category.
//	If the category is already selected, try picking up the item now, otherwise
//	set the requird category, the pick will continue when the category is availble.
//
void pickup_storage_item(int image_id, Uint16 item_id, int cat_id)
{
	if ((storage_win<0) || (find_category(cat_id) == -1))
	{
		do_alert1_sound();
		item_lists_reset_pickup_fail_time();
		return;
	}
	wanted_category = find_category(cat_id);
	wanted_image_id = image_id;
	wanted_item_id = item_id;	
	if (selected_category == wanted_category)
	{
		select_item(wanted_image_id, wanted_item_id);
		wanted_category = -1;
	}
	else
		move_to_category(wanted_category);
}



void get_storage_text (const Uint8 *in_data, int len)
{
	safe_snprintf(storage_text, sizeof(storage_text), "%.*s", len, in_data);
	if ((len > 0) && (printing_category > -1) && (next_item_to_print < number_to_print))
	{
		char the_text[MAX_DESCR_LEN+20];
		if (!next_item_to_print)
		{
			safe_snprintf(the_text, sizeof(the_text), "%s:", &storage_categories[printing_category].name[1] );
			LOG_TO_CONSOLE(c_green2, the_text);
		}
		safe_snprintf(the_text, sizeof(the_text), "%d %s", print_quanities[next_item_to_print++], &storage_text[1] );
		LOG_TO_CONSOLE(c_grey1, the_text);
		storage_text[0] = '\0';
	}
}

static int category_cmp(const void *a, const void *b)
{
	return strcmp(((const struct storage_category *)a)->name,
				((const struct storage_category *)b)->name);
}

void get_storage_categories (const char *in_data, int len)
{
	int i;
	int idx, idxp;

	idx = 1;
	for (i = 0; i < in_data[0] && i < STORAGE_CATEGORIES_SIZE && idx < len; i++)
	{
		storage_categories[i].id = (Uint8)in_data[idx++];
		storage_categories[i].name[0] = to_color_char (c_orange1);
		idxp = 1;
		while (idx < len && idxp < sizeof (storage_categories[i].name) - 1 && in_data[idx] != '\0')
		{
			storage_categories[i].name[idxp++] = in_data[idx++];
		}
		// always make sure the string is terminated
		storage_categories[i].name[idxp] = '\0';

		// was the string too long?
		if (idxp >= sizeof (storage_categories[i].name) - 1)
		{
			// skip rest of string
			while (idx < len && in_data[idx] != '\0') idx++;
		}
		idx++;
	}
	if (sort_storage_categories)
		qsort(storage_categories, i, sizeof(*storage_categories), category_cmp);
	for (i = in_data[0]; i < STORAGE_CATEGORIES_SIZE; i++)
	{
		storage_categories[i].id = -1;
		storage_categories[i].name[0] = 0;
	}

	no_storage_categories = in_data[0];
	if (storage_win > 0) vscrollbar_set_bar_len(storage_win, STORAGE_SCROLLBAR_CATEGORIES, ( no_storage_categories - STORAGE_CATEGORIES_DISPLAY ) > 1 ? (no_storage_categories - STORAGE_CATEGORIES_DISPLAY) : 1);

	selected_category=-1;
	active_storage_item=-1;

	display_storage_menu();
	if (!view_only_storage)
		display_items_menu();
}

int find_category(int id)
{
	int i;

	for(i=0;i<no_storage_categories;i++){
		if(storage_categories[i].id==id) return i;
	}

	return -1;
}

void set_window_name(char *extra_sep, char *extra_name)
{
	safe_snprintf(windows_list.window[storage_win].window_name,
		sizeof(windows_list.window[storage_win].window_name),
		"%s%s%s%s", win_storage, extra_sep, extra_name, ((view_only_storage) ?win_storage_vo:""));
}

void move_to_category(int cat)
{
	Uint8 str[4];
	
	if(cat<0||cat>=no_storage_categories) return;
	storage_categories[cat].name[0] = to_color_char (c_red3);
	if (selected_category!=-1 && cat!=selected_category) 
		storage_categories[selected_category].name[0] = to_color_char (c_orange1);
	set_window_name(" - ", storage_categories[cat].name+1);

	str[0]=GET_STORAGE_CATEGORY;
	*((Uint8 *)(str+1))=storage_categories[cat].id;

	my_tcp_send(my_socket, str, 2);
}

static void update_item_filter(void)
{
	if (!disable_storage_filter && (no_storage > 0) && (filter_item_text_size > 0))
		filter_items_by_description(storage_items_filter, storage_items, filter_item_text, no_storage);
	else
	{
		size_t i;
		for (i=0; i<STORAGE_ITEMS_SIZE; i++)
			storage_items_filter[i] = 0;
	}
}

void get_storage_items (const Uint8 *in_data, int len)
{
	int i;
	int cat, pos;
	int idx;
	int plen;

	if (item_uid_enabled)
		plen=10;
	else
		plen=8;

	if (in_data[0] == 255)
	{
		// It's just an update - make sure we're in the right category
		idx = 2;
		active_storage_item = SDL_SwapLE16(*((Uint16*)(&in_data[idx+6])));
		
		for (i = 0; i < STORAGE_ITEMS_SIZE; i++)
		{
			if ((storage_items[i].pos == SDL_SwapLE16(*((Uint16*)(&in_data[idx+6])))) && (storage_items[i].quantity > 0))
			{
				storage_items[i].image_id = SDL_SwapLE16(*((Uint16*)(&in_data[idx])));
				storage_items[i].quantity = SDL_SwapLE32(*((Uint32*)(&in_data[idx+2])));
				if (item_uid_enabled)
					storage_items[i].id = SDL_SwapLE16(*((Uint16*)(&in_data[idx+8])));
				else
					storage_items[i].id = unset_item_uid;
				update_item_filter();
				return;
			}
		}

		for (i = 0; i < STORAGE_ITEMS_SIZE; i++)
		{
			if (storage_items[i].quantity == 0)
			{
				if (item_uid_enabled)
					storage_items[i].id = SDL_SwapLE16(*((Uint16*)(&in_data[idx+8])));
				else
					storage_items[i].id = unset_item_uid;
				storage_items[i].pos = SDL_SwapLE16(*((Uint16*)(&in_data[idx+6])));
				storage_items[i].image_id = SDL_SwapLE16(*((Uint16*)(&in_data[idx])));
				storage_items[i].quantity = SDL_SwapLE32(*((Uint32*)(&in_data[idx+2])));
				no_storage++;
				update_item_filter();
				return;
			}
		}
	}

	no_storage = (len - 2) / plen;

	cat = find_category(in_data[1]);
	if (cat >= 0)
	{
		storage_categories[cat].name[0] = to_color_char (c_red3);
		if (selected_category != -1 && cat != selected_category)
			storage_categories[selected_category].name[0] = to_color_char (c_orange1);
		set_window_name(" - ", storage_categories[cat].name+1);
		selected_category = cat;
	}

	idx = 2;
	for (i = 0; i < no_storage && i < STORAGE_ITEMS_SIZE; i++, idx += plen)
	{
		storage_items[i].image_id = SDL_SwapLE16(*((Uint16*)(&in_data[idx])));
		storage_items[i].quantity = SDL_SwapLE32(*((Uint32*)(&in_data[idx+2])));
		storage_items[i].pos = SDL_SwapLE16(*((Uint16*)(&in_data[idx+6])));
		if (item_uid_enabled)
			storage_items[i].id = SDL_SwapLE16(*((Uint16*)(&in_data[idx+8])));
		else
			storage_items[i].id = unset_item_uid;
	}
	
	for ( ; i < STORAGE_ITEMS_SIZE; i++)
	{
		storage_items[i].quantity=0;
	}
	
	vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_ITEMS, 0);
	pos = vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES);
	if (cat < pos) {
		vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES, cat);
	} else	if (cat >= pos + STORAGE_CATEGORIES_DISPLAY) {
		vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES, cat - STORAGE_CATEGORIES_DISPLAY + 1);
	}

	if (selected_category != -1)
		category_updated();

	update_item_filter();
}

int storage_win=-1;
int storage_win_x=100;
int storage_win_y=100;
int storage_win_x_len=400;
int storage_win_y_len=272;

int cur_item_over=-1;
int storage_item_dragged=-1;

int display_storage_handler(window_info * win)
{
	int i;
	int n=0;
	int pos;
	int help_text_line = 0;

	static int have_colours = 0;
	static size_t c_selected = ELGL_INVALID_COLOUR;
	static size_t c_highlighted = ELGL_INVALID_COLOUR;

	if (!have_colours)
	{
		c_selected = elglGetColourId("global.mouseselected");
		c_highlighted = elglGetColourId("global.mousehighlight");
		have_colours = 1;
	}

	have_storage_list = 0;	//We visited storage, so we may have changed something

	glColor3f(0.77f, 0.57f, 0.39f);
	glEnable(GL_TEXTURE_2D);
	
	for(i=pos=vscrollbar_get_pos(storage_win,STORAGE_SCROLLBAR_CATEGORIES); i<no_storage_categories && storage_categories[i].id!=-1 && i<pos+STORAGE_CATEGORIES_DISPLAY; i++,n++){
		int the_colour = from_color_char(storage_categories[i].name[0]);
		size_t offset = 1;
		if (the_colour == c_red3)
			elglColourI(c_selected);
		else if (the_colour == c_green2)
			elglColourI(c_highlighted);
		else
			offset = 0;
		draw_string_small(20, 20+n*13, (unsigned char*)&storage_categories[i].name[offset],1);
	}
	glColor3f(0.77f, 0.57f, 0.39f);
	if(storage_text[0]){
		if (strcmp(storage_text, last_storage_text) != 0) {
			safe_strncpy(last_storage_text, storage_text, sizeof(last_storage_text));
			put_small_text_in_box ((Uint8 *)storage_text, strlen(storage_text), win->len_x - 18*2, wrapped_storage_text);
		}
		draw_string_small(18, 220, (unsigned char*)wrapped_storage_text, 2);
	}

	glColor3f(1.0f,1.0f,1.0f);
	for(i=pos=6*vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_ITEMS); i<pos+36 && i<no_storage;i++){
		GLfloat u_start, v_start, u_end, v_end;
		int x_start, x_end, y_start, y_end;
		int cur_item;
		GLuint this_texture;

		if(!storage_items[i].quantity)continue;
		cur_item=storage_items[i].image_id%25;
#ifdef	NEW_TEXTURES
		get_item_uv(cur_item, &u_start, &v_start, &u_end, &v_end);
#else	/* NEW_TEXTURES */
		u_start=0.2f*(cur_item%5);
		u_end=u_start+(float)50/255;
		v_start=(1.0f+((float)50/255)/255.0f)-((float)50/255*(cur_item/5));
		v_end=v_start-(float)50/255;
#endif	/* NEW_TEXTURES */
		
		this_texture=get_items_texture(storage_items[i].image_id/25);

#ifdef	NEW_TEXTURES
		if (this_texture != -1)
		{
			bind_texture(this_texture);
		}
#else	/* NEW_TEXTURES */
		if(this_texture!=-1) get_and_set_texture_id(this_texture);
#endif	/* NEW_TEXTURES */

		x_start=(i%6)*32+161;
		x_end=x_start+31;
		y_start=((i-pos)/6)*32+10;
		y_end=y_start+31;

		glBegin(GL_QUADS);
		draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
		glEnd();

		if (!disable_storage_filter && filter_item_text_size && storage_items_filter[i])
			gray_out(x_start,y_start,32);
	}

	if(cur_item_over!=-1 && mouse_in_window(win->window_id, mouse_x, mouse_y) == 1){
		char str[20];
		Uint16 item_id = storage_items[cur_item_over].id;
		int image_id = storage_items[cur_item_over].image_id;
		if (show_item_desc_text && item_info_available() && (get_item_count(item_id, image_id) == 1))
			show_help(get_item_description(item_id, image_id), 0, win->len_y + 10 + (help_text_line++) * SMALL_FONT_Y_LEN);

		if (active_storage_item!=storage_items[cur_item_over].pos) {
			safe_snprintf(str, sizeof(str), "%d",storage_items[cur_item_over].quantity);
			if (enlarge_text())
				show_sized_help(str, mouse_x-win->pos_x-(strlen(str)/2)*DEFAULT_FONT_X_LEN,mouse_y-win->pos_y-DEFAULT_FONT_Y_LEN-1, 1);
			else
				show_help(str,mouse_x-win->pos_x-(strlen(str)/2)*8,mouse_y-win->pos_y-14);
		}
	}
	
	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	
	glColor3f(0.77f, 0.57f, 0.39f);
	
	glBegin(GL_LINE_LOOP);
		glVertex2i(10,  10);
		glVertex2i(10,  202);
		glVertex2i(130, 202);
		glVertex2i(130, 10);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex2i(10, 212);
		glVertex2i(10, 262);
		glVertex2i(392, 262);
		glVertex2i(392, 212);
	glEnd();
	
	if (view_only_storage)
	{
		Uint32 currentticktime = SDL_GetTicks();
		if (currentticktime < drop_fail_time)
			drop_fail_time = 0; 				/* trap wrap */
		if ((currentticktime - drop_fail_time) < 250)
			glColor3f(0.8f,0.2f,0.2f);			/* flash red if tried to drop into */
		else
			glColor3f(0.37f, 0.37f, 0.39f);		/* otherwise draw greyed out */
	}

	rendergrid(6, 6, 160, 10, 32, 32);
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f,1.0f,1.0f);
	if(active_storage_item >= 0) {
		/* Draw the active item's quantity on top of everything else. */
		for(i = pos = 6*vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_ITEMS); i < pos+36 && i < no_storage; i++) {
			if(storage_items[i].pos == active_storage_item) {
				if (storage_items[i].quantity) {
					char str[20];
					int x = (i%6)*32+161;

					safe_snprintf(str, sizeof(str), "%d", storage_items[i].quantity);
					if(x > 353) {
						x = 321;
					}
					if ((mouse_in_window(win->window_id, mouse_x, mouse_y) == 1) && enlarge_text())
						show_sized_help(str, x, ((i-pos)/6)*32+18, 1);
					else
						show_help(str, x, ((i-pos)/6)*32+18);
				}
				break;
			}
		}
	}
	
	if (!disable_storage_filter && !mouse_over_titlebar)
	{
		if(filter_item_text_size > 0)
		{
			static char tmp[50];
			safe_snprintf(tmp, sizeof(tmp), "%s[%s]", storage_filter_prompt_str, filter_item_text);
			show_help(tmp, 0, win->len_y + 10 + (help_text_line++) * SMALL_FONT_Y_LEN);
		}
		else if (show_help_text && mouse_over_storage)
			show_help(storage_filter_help_str, 0, win->len_y + 10 + (help_text_line++) * SMALL_FONT_Y_LEN);
	}

	mouse_over_storage = mouse_over_titlebar = 0;

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int click_storage_handler(window_info * win, int mx, int my, Uint32 flags)
{
	if(flags&ELW_WHEEL_UP) {
		if(mx>10 && mx<130) {
			vscrollbar_scroll_up(storage_win, STORAGE_SCROLLBAR_CATEGORIES);
		} else if(mx>150 && mx<352){
			vscrollbar_scroll_up(storage_win, STORAGE_SCROLLBAR_ITEMS);
		}
	} else if(flags&ELW_WHEEL_DOWN) {
		if(mx>10 && mx<130) {
			vscrollbar_scroll_down(storage_win, STORAGE_SCROLLBAR_CATEGORIES);
		} else if(mx>150 && mx<352){
			vscrollbar_scroll_down(storage_win, STORAGE_SCROLLBAR_ITEMS);
		}
	}
	else if ( (flags & ELW_MOUSE_BUTTON) == 0) {
		return 0;
	}
	else {
		if(my>10 && my<202){
			if(mx>10 && mx<130){
				int cat=-1;
		
				cat=(my-20)/13 + vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES);
				move_to_category(cat);
				do_click_sound();
			} else if(mx>150 && mx<352){
				if(view_only_storage && item_dragged!=-1 && left_click){
					drop_fail_time = SDL_GetTicks();
					do_alert1_sound();
				} else if(!view_only_storage && item_dragged!=-1 && left_click){
					Uint8 str[6];

					str[0]=DEPOSITE_ITEM;
					str[1]=item_list[item_dragged].pos;
					*((Uint32*)(str+2))=SDL_SwapLE32(item_quantity);

					my_tcp_send(my_socket, str, 6);
					do_drop_item_sound();

					if(item_list[item_dragged].quantity<=item_quantity) item_dragged=-1;//Stop dragging this item...
				} else if(right_click || (view_only_storage && left_click)){
					storage_item_dragged=-1;
					item_dragged=-1;

					if(cur_item_over!=-1) {
						Uint8 str[3];

						str[0]=LOOK_AT_STORAGE_ITEM;
						*((Uint16*)(str+1))=SDL_SwapLE16(storage_items[cur_item_over].pos);
	
						my_tcp_send(my_socket, str, 3);
	
						active_storage_item=storage_items[cur_item_over].pos;
						do_click_sound();
					}
				} else if(!view_only_storage && cur_item_over!=-1){
					storage_item_dragged=cur_item_over;
					active_storage_item=storage_items[cur_item_over].pos;
					do_drag_item_sound();
				}
			}
		}
	}

	return 1;
}

int mouseover_storage_handler(window_info *win, int mx, int my)
{
	static int last_pos;
	int last_category;

	cur_item_over=-1;

	if (my < 0)
		mouse_over_titlebar = 1;
	else
		mouse_over_storage = 1;

	if(my>10 && my<202){
		if(mx>10 && mx<130){
			int i;
			int pos=last_pos=(my-20)/13;
			int p;

			for(i=p=vscrollbar_get_pos(storage_win,STORAGE_SCROLLBAR_CATEGORIES);i<no_storage_categories;i++){
				if(i==selected_category) {
				} else if(i!=p+pos) {
					storage_categories[i].name[0]  = to_color_char (c_orange1);
				} else {
					storage_categories[i].name[0] = to_color_char (c_green2);
				}
			}
			
			return 0;
		} else if (mx>150 && mx<352){
			cur_item_over = get_mouse_pos_in_grid(mx, my, 6, 6, 160, 10, 32, 32)+vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_ITEMS)*6;
			if(cur_item_over>=no_storage||cur_item_over<0||!storage_items[cur_item_over].quantity) cur_item_over=-1;
		}
	}
	
	last_category = last_pos+vscrollbar_get_pos(storage_win,STORAGE_SCROLLBAR_CATEGORIES);
	if(last_pos>=0 && last_pos<13 && last_category != selected_category) {
		storage_categories[last_category].name[0] = to_color_char (c_orange1);
		last_pos=-1;
	}

	return 0;
}


static int keypress_storage_handler(window_info *win, int mx, int my, Uint32 key, Uint32 unikey, Uint16 mods)
{
	char keychar = tolower(key_to_char(unikey));
	if (disable_storage_filter || (keychar == '`') || (mods & KMOD_CTRL) || (mods & KMOD_ALT))
		return 0;
	if (key == SDLK_ESCAPE)
	{
		filter_item_text[0] = '\0';
		filter_item_text_size = 0;
		return 1;
	}
	item_info_help_if_needed();
	if (string_input(filter_item_text, sizeof(filter_item_text), key, keychar))
	{
		filter_item_text_size = strlen(filter_item_text);
		if (filter_item_text_size > 0)
			filter_items_by_description(storage_items_filter, storage_items, filter_item_text, no_storage);
		return 1;
	}
	return 0;
}

void print_items(void)
{
	int i;
	actor *me;

	me = get_our_actor();
	if (me)
		if(me->fighting)
		{
			LOG_TO_CONSOLE(c_red1, "You can't do this during combat!");
			return;
		}
	
	/* request the description for each item */
	number_to_print = next_item_to_print = 0;
	printing_category = selected_category;
	for (i = 0; i < no_storage && i < STORAGE_ITEMS_SIZE; i++)
	{
		if (storage_items[i].quantity)
		{		
			Uint8 str[3];
			print_quanities[number_to_print++] = storage_items[i].quantity;
			str[0]=LOOK_AT_STORAGE_ITEM;
			*((Uint16*)(str+1))=SDL_SwapLE16(storage_items[i].pos);
			my_tcp_send(my_socket, str, 3);
		}
	}
}

static int context_storage_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (option<ELW_CM_MENU_LEN)
		return cm_title_handler(win, widget_id, mx, my, option);
	switch (option)
	{
		case ELW_CM_MENU_LEN+1: print_items(); break;
		case ELW_CM_MENU_LEN+2: safe_strncpy(storage_text, reopen_storage_str, MAX_DESCR_LEN) ; break;
	}
	return 1;
}

void display_storage_menu()
{
	int i;

	/* Entropy suggested hack to determine if this is the view only "#sto" opened storage */
	view_only_storage = 0;
	for (i = 0; i < no_storage_categories; i++)
	{
		if ((storage_categories[i].id != -1) && (strcmp(&storage_categories[i].name[1], "Quest") == 0))
		{
			view_only_storage = 1;
			break;
		}
	}

	if(storage_win<=0){
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		storage_win=create_window(win_storage, our_root_win, 0, storage_win_x, storage_win_y, storage_win_x_len, storage_win_y_len, ELW_WIN_DEFAULT|ELW_TITLE_NAME);
		set_window_handler(storage_win, ELW_HANDLER_DISPLAY, &display_storage_handler);
		set_window_handler(storage_win, ELW_HANDLER_CLICK, &click_storage_handler);
		set_window_handler(storage_win, ELW_HANDLER_MOUSEOVER, &mouseover_storage_handler);
		set_window_handler(storage_win, ELW_HANDLER_KEYPRESS, &keypress_storage_handler );

		vscrollbar_add_extended(storage_win, STORAGE_SCROLLBAR_CATEGORIES, NULL, 130, 10, 20, 192, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, 
				max2i(no_storage_categories - STORAGE_CATEGORIES_DISPLAY, 0));
		vscrollbar_add_extended(storage_win, STORAGE_SCROLLBAR_ITEMS, NULL, 352, 10, 20, 192, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, 28);
		
		cm_add(windows_list.window[storage_win].cm_id, cm_storage_menu_str, context_storage_handler);
		cm_add(windows_list.window[storage_win].cm_id, cm_dialog_options_str, context_storage_handler);
		cm_bool_line(windows_list.window[storage_win].cm_id, ELW_CM_MENU_LEN+2, &sort_storage_categories, NULL);
		cm_bool_line(windows_list.window[storage_win].cm_id, ELW_CM_MENU_LEN+3, &disable_storage_filter, NULL);
		cm_bool_line(windows_list.window[storage_win].cm_id, ELW_CM_MENU_LEN+4, &autoclose_storage_dialogue, NULL);
		cm_bool_line(windows_list.window[storage_win].cm_id, ELW_CM_MENU_LEN+5, &auto_select_storage_option, NULL);
	} else {
		no_storage=0;
		
		for(i = 0; i < no_storage_categories; i++)
			storage_categories[i].name[0] = to_color_char (c_orange1);

		show_window(storage_win);
		select_window(storage_win);

		vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES, 0);
		vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_ITEMS, 0);
	}

	storage_text[0] = '\0';
	set_window_name("", "");
}

void close_storagewin()
{
	if(storage_win >= 0 && !view_only_storage) {
		hide_window(storage_win);
	}
}
