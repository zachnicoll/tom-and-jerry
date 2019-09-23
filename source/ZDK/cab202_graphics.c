/*
**  cab202_graphics.c
** 
**  Simple character-based "graphics" library for CAB202, 2016 semester 2.
** 
**  Authors:
**      Lawrence Buckingham
**      Benjamin Talbot
**      Jenna Riseley
** 
** 	$Revision:Sat Feb 23 00:47:31 EAST 2019$
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <curses.h>
#include <assert.h>
#include "cab202_graphics.h"
#include "cab202_timers.h"

#define ABS(x)	 (((x) >= 0) ? (x) : -(x))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define SIGN(x)	 (((x) > 0) - ((x) < 0))

 // Global variables to support automated testing.
FILE * zdk_save_stream = NULL;
FILE * zdk_input_stream = NULL;
bool zdk_suppress_output = false;

// Private helper functions.
static void save_screen_(FILE * f);
static void destroy_screen(Screen * scr);
static void save_char(int char_code);

/*
 *	Screen buffers. The most recent screen displayed by show_screen
 *	remains in zdk_prev_screen until the next call to show_screen. The
 *	data that will appear next time show_screen is called is stored
 *	in zdk_screen.
 *
 *	Applications can check these objects for example, to see if a
 *	character has changed, or if something else is already present
 *	at a location.
 */
Screen * zdk_screen = NULL;
Screen * zdk_prev_screen = NULL;

/*
 * The current foreground and background colour.
 */
static int foreground = WHITE;
static int background = BLACK;
static int colour_flags = 0;
static int colour_num = 0;

/*
**	Helper function which gets the colour number corresponding to a designated
**	(foreground,background) combination.
**
**	Input:
**		fg - The foreground colour: an integer which must be between 0 and
**			(NUM_COLORS-1), inclusive.
**		bg - The background colour: an integer which must be between 0 and
**			(NUM_COLORS-1), inclusive.
**
**	Output:
**		Returns a unique non-zero integer which represents the colour combination.
*/

static int colour_index(int fg, int bg) {
    const int PAIR_OFFSET = 1;
    return bg * NUM_COLOURS + fg + PAIR_OFFSET;
}

/*
**	Helper function which gets the ncurses attribute corresponding to the
**	current (foreground,background) combination.
**
**	Input:
**		None (uses file-scope variables foreground, background, bright)
**
**	Output:
**		Returns a unique non-zero integer which represents the colour combination.
*/

static void update_colour_num(void) {
    int pair = COLOR_PAIR(colour_index(foreground, background));

    if (colour_flags & BRIGHT) {
        pair |= A_BOLD;
    }

    if (colour_flags & INVERSE) {
        pair |= A_REVERSE;
    }

    colour_num = pair;

    // colour_num = colour_index(foreground, background);
}

/*
**	See graphics.h for documentation.
*/
void set_background(int colour) {
    background = colour & (NUM_COLOURS - 1);
    update_colour_num();
}

/*
**	See graphics.h for documentation.
*/
void set_foreground(int colour) {
    foreground = colour & (NUM_COLOURS - 1);
    colour_flags = colour & (TRANSPARENT | INVERSE | BRIGHT);
    update_colour_num();
}

/*
**	See graphics.h for documentation.
*/
void set_colours(int foreground_, int background_) {
    background = background_ & (NUM_COLOURS - 1);
    foreground = foreground_ & (NUM_COLOURS - 1);
    colour_flags = foreground_ & (TRANSPARENT | INVERSE | BRIGHT);
    update_colour_num();
}

/*
**	See graphics.h for documentation.
*/
void get_colours(int *foreground_, int *background_) {
    *background_ = background;
    *foreground_ = foreground | colour_flags;
}

/*
**	See graphics.h for documentation.
*/
int get_background(void) {
    return background;
}

/*
**	See graphics.h for documentation.
*/
int get_foreground(void) {
    return foreground | colour_flags;
}

/*
**	See graphics.h for documentation.
*/
void setup_screen(void) {
    if (!zdk_suppress_output) {
        // Enter curses mode.
        initscr();
        start_color();

        // Set up a colour pair for each terminal colour.
        // The default background colour is black. 
        for (int fg = 0; fg < NUM_COLOURS; fg++) {
            for (int bg = 0; bg < NUM_COLOURS; bg++) {
                init_pair(colour_index(fg, bg), fg, bg);
            }
        }

        foreground = COLOR_WHITE;
        background = COLOR_BLACK;
        update_colour_num();
        bkgd(colour_num);

        // Do not echo keypresses.
        noecho();

        // Turn off the cursor.
        curs_set(0);

        // Cause getch to return ERR if no key pressed within 0 milliseconds.
        timeout(0);

        // Enable the keypad.
        keypad(stdscr, TRUE);

        // Turn on mouse reporting.
        mousemask(ALL_MOUSE_EVENTS, NULL);

        // Erase any previous content that may be lingering in this screen.
        clear();
    }

    // Create buffers
    fit_screen_to_window();

    // Add exit procedure to cleanup the screen before the program exists.
    // Guard to ensure cleanup_screen is added at most once, because in certain
    // situations (e.g. AMS) this function may be called multiple times.
    static bool deja_vu = false;

    if (!deja_vu) {
        void ctrl_c_handler(int signal_code);
        signal(SIGINT, ctrl_c_handler);
        atexit(cleanup_screen);
        deja_vu = true;
    }
}

/**
 *	Signal handler for ctrl-c to ensure screen is cleaned up properly.
 */
void ctrl_c_handler(int signal_code) {
    // atexit handler has already been installed, so we should be able to
    // use exit();
    exit(1);
}

/*
**	See graphics.h for documentation.
*/
void cleanup_screen(void) {
    if (!zdk_suppress_output) {
        // cleanup curses.
        endwin();
    }

    // cleanup the drawing buffers.
    destroy_screen(zdk_screen);
    zdk_screen = NULL;

    destroy_screen(zdk_prev_screen);
    zdk_prev_screen = NULL;

    // Close the screen-cast file, if open.
    if (zdk_save_stream) {
        fflush(zdk_save_stream);
        fclose(zdk_save_stream);
        zdk_save_stream = NULL;
    }

    // Null the input file (somebody else is responsible for its life cycle).
    if (zdk_save_stream) {
        zdk_input_stream = NULL;
    }
}

/*
**	See graphics.h for documentation.
*/
void clear_screen(void) {
    if (zdk_screen != NULL) {
        int w = zdk_screen->width;
        int h = zdk_screen->height;

        set_foreground(WHITE);

        char * scr = zdk_screen->pixels[0];
        int * colours = zdk_screen->colours[0];

        memset(scr, ' ', w * h);

        for (int i = 0; i < w*h; i++) {
            colours[i] = colour_num;
        }
    }
}

/*
**	See graphics.h for documentation.
*/
void show_screen(void) {
    // Draw parts of the display that are different in the front
    // buffer from the back buffer.
    char ** back_px = zdk_prev_screen->pixels;
    char ** front_px = zdk_screen->pixels;
    int ** back_colour = zdk_prev_screen->colours;
    int ** front_colour = zdk_screen->colours;
    int w = zdk_screen->width;
    int h = zdk_screen->height;
    bool changed = false;

    // Check each character to see if it has changed (either in value or colour)
    // since the last time the function was called.
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (front_px[y][x] != back_px[y][x] || front_colour[y][x] != back_colour[y][x]) {
                // Send changed char data to terminal.
                attrset(front_colour[y][x]);
                // color_set(COLOR_PAIR(front_colour[y][x]), NULL);
                mvaddch(y, x, front_px[y][x]);

                // Save new char data in back buffer.
                back_px[y][x] = front_px[y][x];
                back_colour[y][x] = front_colour[y][x];
                changed = true;
            }
        }
    }

    if (!changed) {
        return;
    }

    // Save a screen shot, if automatic saves are enabled.
    save_screen_(zdk_save_stream);

    // Force an update of the curses display.
    if (!zdk_suppress_output) {
        refresh();
    }
}

/*
**	See graphics.h for documentation.
*/
void draw_char(int x, int y, char value) {
    if (zdk_screen != NULL) {
        int w = zdk_screen->width;
        int h = zdk_screen->height;

        if (x >= 0 && x < w && y >= 0 && y < h) {
            zdk_screen->pixels[y][x] = value;
            zdk_screen->colours[y][x] = colour_num;
        }
    }
}

/*
**	See graphics.h for documentation.
*/
void draw_line(int x1, int y1, int x2, int y2, char value) {
    if (x1 == x2) {
        // Draw vertical line
        int y_min = MIN(y1, y2);
        int y_max = MAX(y1, y2);

        for (int i = y_min; i <= y_max; i++) {
            draw_char(x1, i, value);
        }
    }
    else if (y1 == y2) {
        // Draw horizontal line
        int x_min = MIN(x1, x2);
        int x_max = MAX(x1, x2);

        for (int i = x_min; i <= x_max; i++) {
            draw_char(i, y1, value);
        }
    }
    else {
        // Inserted to ensure that lines are always drawn in the same direction, regardless of
        // the order the endpoints are presented.
        if (x1 > x2) {
            int t = x1;
            x1 = x2;
            x2 = t;
            t = y1;
            y1 = y2;
            y2 = t;
        }

        // Use the Bresenham algorithm to render the line.
        // TODO: Convert to an integer-only implementation such as
        // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
        float dx = x2 - x1;
        float dy = y2 - y1;
        float err = 0.0;
        float derr = ABS(dy / dx);

        for (int x = x1, y = y1; (dx > 0) ? x <= x2 : x >= x2; (dx > 0) ? x++ : x--) {
            draw_char(x, y, value);
            err += derr;
            while (err >= 0.5 && ((dy > 0) ? y <= y2 : y >= y2)) {
                draw_char(x, y, value);
                y += (dy > 0) - (dy < 0);

                err -= 1.0;
            }
        }
    }
}

/*
**	See graphics.h for documentation.
*/
char scrape_char(int x, int y) {
    if (x < 0 || y < 0 ||
        x >= zdk_prev_screen->width ||
        y >= zdk_prev_screen->height
        ) {
        return -1;
    }
    else {
        return zdk_prev_screen->pixels[y][x];
    }
}

/*
**	See graphics.h for documentation.
*/
void draw_solid_line(int x1, int y1, int x2, int y2, int colour) {
    int fg = get_foreground();
    int bg = get_background();
    set_foreground(colour | INVERSE);
    set_background(colour);
    draw_line(x1, y1, x2, y2, ' ');
    set_foreground(fg);
    set_background(bg);
}

/*
**	See graphics.h for documentation.
*/
void draw_string(int x, int y, char * text) {
    for (int i = 0; text[i]; i++) {
        draw_char(x + i, y, text[i]);
    }
}

/*
**	See graphics.h for documentation.
*/
void draw_int(int x, int y, int value) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%d", value);
    draw_string(x, y, buffer);
}

/*
**	See graphics.h for documentation.
*/
void draw_double(int x, int y, double value) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%g", value);
    draw_string(x, y, buffer);
}

/*
**	See graphics.h for documentation.
*/
void draw_formatted(int x, int y, const char * format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1000];
    vsnprintf(buffer, sizeof(buffer), format, args);
    draw_string(x, y, buffer);
}

/* static */ MEVENT mouse_event;

/*
**	See graphics.h for documentation.
*/
int get_char() {
    int current_char;

    if (zdk_input_stream) {
        current_char = fgetc(zdk_input_stream);
    }
    else {
        current_char = getch();
    }

    save_char(current_char);

    if (current_char == KEY_MOUSE) {
        memset(&mouse_event, 0, sizeof(mouse_event));
        getmouse(&mouse_event);
    }

    return current_char;
}

/*
**	See graphics.h for documentation.
*/
int get_mouse_x() {
    return mouse_event.x;
}

/*
**	See graphics.h for documentation.
*/
int get_mouse_y() {
    return mouse_event.y;
}

/*
**	See graphics.h for documentation.
*/
unsigned long get_mouse_buttons() {
    return mouse_event.bstate;
}

/*
**	See graphics.h for documentation.
*/
int wait_char() {
    int current_char;

    if (zdk_input_stream) {
        current_char = fgetc(zdk_input_stream);
    }
    else {
        timeout(-1);
        current_char = getch();
        timeout(0);
    }

    save_char(current_char);

    if (current_char == KEY_MOUSE) {
        memset(&mouse_event, 0, sizeof(mouse_event));
        getmouse(&mouse_event);
    }

    return current_char;
}

/*
**	See graphics.h for documentation.
*/
void get_screen_size(int * width, int * height) {
    *width = screen_width();
    *height = screen_height();
}

/*
**	See graphics.h for documentation.
*/
int screen_width(void) {
    return zdk_screen->width;
}

/*
**	See graphics.h for documentation.
*/
int screen_height(void) {
    return zdk_screen->height;
}

/*
**	See graphics.h for documentation.
*/
void save_screen(const char * file_name) {
    FILE * f = fopen(file_name, "a");
    save_screen_(f);
    fclose(f);
}

/*
**	See graphics.h for documentation.
*/
void save_screen_(FILE * f) {
    if (f == NULL) return;

    if (zdk_screen) {
        int width = zdk_screen->width;
        int height = zdk_screen->height;

        fprintf(f, "Frame(%d,%d,%f)\n", width, height, get_current_time());

        for (int y = 0; y < height; y++) {
            char * row = zdk_screen->pixels[y];

            for (int x = 0; x < width; x++) {
                fputc(row[x], f);
            }

            fputc('\n', f);
        }

        fprintf(f, "EndFrame\n");
    }
}

/**
 *	Saves the current character to an automatically named local file.
 */
void save_char(int char_code) {
    if (zdk_save_stream && char_code != ERR) {
        fprintf(zdk_save_stream, "Char(%d,%f)\n", char_code, get_current_time());
    }
}

/**
 *	This function is provided to support programmatic emulation
 *	of a resized terminal window.
 *	Subsequent calls to screen_width() and screen_height() will
 *	return the supplied values of width and height.
 */
void override_screen_size(int width, int height) {
    void update_buffer(Screen ** buffer, int width, int height, char character, char colour_num);

    update_buffer(&zdk_screen, width, height, ' ', colour_num);
    update_buffer(&zdk_prev_screen, width, height, ' ', colour_num);
}

// Private helper function to allocate sccreen buffer.
static void ** allocate_screen_buffer(int width, int height, char data, size_t element_size);

/**
 *	Private helper function which reallocates and clears the designated buffer.
 *	PRE:	buffer &ne; NULL
 *		AND	width &gt; 0
 *		AND height &gt; 0.
 */

void update_buffer(Screen ** screen, int width, int height, char character, char colour_num) {
    assert(width > 0);
    assert(height > 0);

    if (screen == NULL) {
        return;
    }

    Screen * old_screen = (*screen);

    if (old_screen != NULL && width == old_screen->width && height == old_screen->height) {
        return;
    }

    Screen * new_screen = calloc(1, sizeof(Screen));

    if (!new_screen) {
        *screen = NULL;
        return;
    }

    new_screen->width = width;
    new_screen->height = height;

    new_screen->pixels = (char**)allocate_screen_buffer(width, height, character, sizeof(char));

    if (!new_screen->pixels) {
        destroy_screen(new_screen);
        return;
    }

    new_screen->colours = (int**)allocate_screen_buffer(width, height, colour_num, sizeof(int));

    if (!new_screen->colours) {
        destroy_screen(new_screen);
        return;
    }

    void copy_screen(Screen * old_scr, Screen * new_scr);

    copy_screen(old_screen, new_screen);

    destroy_screen(old_screen);

    (*screen) = new_screen;
}

/*
**	Creates a table organised as a 2D "array of arrays", having elements of a
**	designated size. This may be used either as a character buffer or a colour
**	info buffer.
**
**	Input:
**		width, height: the number of columns and rows respectively in the table.
**
**		default_value: a char code which will be used to initialise all cells
**			in the table.
**
**		element_size: the number of bytes required for each element.
**
**	Output:
**		Returns the address of the 2D aray structure.
*/
static void ** allocate_screen_buffer(int width, int height, char default_value, size_t element_size) {
    void ** buffer = calloc(height, sizeof(void *));

    if (!buffer) {
        return NULL;
    }

    buffer[0] = calloc(width * height, element_size);

    if (!buffer[0]) {
        free(buffer);
        return NULL;
    }

    for (int y = 1; y < height; y++) {
        buffer[y] = (char *)buffer[y - 1] + width * element_size;
    }

    memset(buffer[0], default_value, width * height * element_size);
    return buffer;
}

/**
 *	Copies the data from one screen into the bitmap of another,
 *	clipping to ensure that data is only copied in the smallest
 *	rectangle that fits on both screens.
 *
 *	Input:
 *		src - the address of a Screen from which data is to be copied.
 *		dest - the address of a Screen into which the data is to be copied.
 *
 *	Output: void.
 *
 *	Postcondition:
 *		(src is null OR dest is null OR src is dest ) AND nothing happens;
 *	OR:
 *		The contents of the source screen lying in the area that overlaps
 *		with the destination has been copied into dest at the corresponding
 *		position.
 */

void copy_screen(Screen * src, Screen * dest) {
    if (src == NULL || dest == NULL || src == dest) return;

    int clip_width = MIN(src->width, dest->width);
    int clip_height = MIN(src->height, dest->height);

    for (int y = 0; y < clip_height; y++) {
        memcpy(dest->pixels[y], src->pixels[y], clip_width);
        memcpy(dest->colours[y], src->colours[y], clip_width * sizeof(int));
    }
}

/*
**	This function is provided to support programmatic emulation
**	of a resized terminal window. It undoes the effects of
**	override_screen_size.
**	Subsequent calls to screen_width() and screen_height() will
**	return the width and height of the current terminal window,
**	respectively.
*/
void fit_screen_to_window(void) {
    if (zdk_suppress_output) {
        override_screen_size(80, 24);
    }
    else {
        override_screen_size(getmaxx(stdscr), getmaxy(stdscr));
    }
}

/**
 *	Releases all memory allocated to a given Screen.
 *
 *	Parameters:
 *		scr - a (possibly null) pointer to a Screen structure.
 *
 *	Notes: if scr is NULL no action is taken.
 */

void destroy_screen(Screen * scr) {
    if (scr) {
        if (scr->pixels) {
            if (scr->pixels[0]) {
                free(scr->pixels[0]);
            }

            free(scr->pixels);
        }

        if (scr->colours) {
            if (scr->colours[0]) {
                free(scr->colours[0]);
            }

            free(scr->colours);
        }

        free(scr);
    }
}

void auto_save_screen(bool save_if_true) {
    if (save_if_true && !zdk_save_stream) {
        char file_name[100];

        // A somewhat arbitrary upper limit on the number of save files.
        for (int i = 1; i < 1000000; i++) {
            sprintf(file_name, "zdk_screen.%d.txt", i);
            FILE * existing_file = fopen(file_name, "r");

            if (existing_file) {
                // File exists; leave it alone.
                fclose(existing_file);
            }
            else {
                // File does not exist; use this name.
                break;
            }
        }

        zdk_save_stream = fopen(file_name, "w");
    }
    else if (zdk_save_stream && !save_if_true) {
        fflush(zdk_save_stream);
        fclose(zdk_save_stream);
        zdk_save_stream = NULL;
    }
}
