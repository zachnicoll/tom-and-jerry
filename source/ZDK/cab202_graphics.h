/*
*    Graphics.h
*
*    Base-level curses-based graphics library for CAB202, 2016 semester 2.
*
*    Authors:
*         Lawrence Buckingham
*        Jenna Riseley
*        Ben Talbot.
*
*    $Revision:Sat Feb 23 00:47:31 EAST 2019$
*/

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

/*
 *  Screen structure contains the off-screen drawing area in which each
 *  frame of the view is constructed before being flushed to the display.
 *
 *  Members:
 *      width - A non-negative integer which specifies the width of the
 *              screen. When the screen is first initialised, this is
 *              made equal to the width of the terminal window at that time.
 *              Subsequently, it may be changed by override_screen_size or
 *              by fit_screen_to_window.
 *
 *      height - A non-negative integer which specifies the height of the
 *              screen. When the screen is first initialised, this is
 *              made equal to the height of the terminal window at that time.
 *              Subsequently, it may be changed by override_screen_size or
 *              by fit_screen_to_window.
 *
 *      pixels - A two-dimensional (ragged) array of characters containing the
 *              "pixel" data of the display. To access the character at
 *              location (x,y) of Screen * s, use:
 *                               s->pixels[y][x]
 *
 *      colours - A two-dimensional (ragged) array of int containing the
 *              colour data of the display. To access the colour at
 *              location (x,y) of Screen * s, use:
 *                               s->colours[y][x]
 */
typedef struct Screen {
    int width;
    int height;
    char ** pixels;
    int ** colours;
} Screen;

/**
 *    The active screen to which data is added by drawing commands.
 *    The contents of this screen will be rendered into the display
 *    when show_screen() is called.
 */
Screen * zdk_screen;

/**
 *    A backing screen which contains a copy data previously displayed by
 *    show_screen().
 */
Screen * zdk_prev_screen;

/**
 *    Set up the terminal display for curses-based graphics:
 *    .    Echo is disabled.
 *    .    Keystrokes are reported immediately rather than waiting for Enter.
 *    .    Mouse interactions are turned on.
 *    .    The numeric keypad and arrow keys are enabled.
 */
void setup_screen(void);

/**
 *    Erases the zdk_screen buffer by filling it with space characters
 *    (ASCII code 32). The current (foreground,background) colour pair
 *    is applied to every pixel location.
 *
 *    Notes:
 *    .    This does not cause the screen to immediately appear "blank".
 *    .    To do that, call clear_screen() and then call show_screen().
 */
void clear_screen(void);

/**
 *    Transfers the contents of the zdk_screen buffer to the curses display
 *    window. The operation is optimised to the extent that only characters which
 *    have changed since the last call to show_screen are emitted.
 *
 *    The display is double-buffered, so after this, the contents of the
 *    zdk_screen are copied to the zdk_prev_screen.
 */
void show_screen(void);

/**
 *    Draws the specified symbol at the prescribed (x,y) location in the terminal
 *    window. The rendered character is added to the zdk_screen buffer, but
 *    remains unseen until the next invocation of show_screen(). The character is
 *    drawn with the current (foreground,background) colour pair.
 *
 *    Input:
 *        x - The horizontal offset of the location at which the character
 *            is to be drawn, measuring from left to right, with 0 corresponding
 *            to the left-most visible character position in the terminal window.
 *
 *        y - The vertical offset of the location at which the character
 *            is to be drawn, measuring from top to bottom, with 0 corresponding
 *            to the upper-most visible character position in the terminal window.
 *
 *        value - The character code (ASCII) of the symbol to be displayed.
 *
 *    Output: void.
 */
void draw_char(int x, int y, char value);

/**
 *    Draws the specified string, starting at the prescribed (x,y) location in the
 *    terminal window. The rendered text is added to the zdk_screen buffer, but
 *    remains unseen until the next invocation of show_screen(). The text is
 *    drawn with the current (foreground,background) colour pair.
 *
 *    Input:
 *        x - The horizontal offset of the location at which the first character
 *            is to be drawn, measuring from left to right, with 0 corresponding
 *            to the left-most visible character position in the terminal window.
 *
 *        y - The vertical offset of the location at which the first character
 *            is to be drawn, measuring from top to bottom, with 0 corresponding
 *            to the upper-most visible character position in the terminal window.
 *
 *        value - The character code (ASCII) of the symbol to be displayed.
 *
 *    Output: void.
 */
void draw_string(int x, int y, char * text);

/**
 *    Draws an integer value, starting at the prescribed (x,y) location in the
 *    terminal window. The rendered text is added to the zdk_screen buffer, but
 *    remains unseen until the next invocation of show_screen(). The value is
 *    drawn with the current (foreground,background) colour pair.
 *
 *    Input:
 *        x - The horizontal offset of the location at which the first character
 *            is to be drawn, measuring from left to right, with 0 corresponding
 *            to the left-most visible character position in the terminal window.
 *
 *        y - The vertical offset of the location at which the first character
 *            is to be drawn, measuring from top to bottom, with 0 corresponding
 *            to the upper-most visible character position in the terminal window.
 *
 *        value - The numeric value to be displayed.
 *
 *    Output: void.
 *
 *    Notes:
 *    (1)    This function is logically equivalent to draw_formatted(x, y, "%d", value).
 *    (2) Use draw_formatted() to achieve advanced effects such as justification and padding.
 */
void draw_int(int x, int y, int value);

/**
 *    Draws a floating point value, starting at the prescribed (x,y) location in the
 *    terminal window. The rendered text is added to the zdk_screen buffer, but
 *    remains unseen until the next invocation of show_screen(). The value is
 *    drawn with the current (foreground,background) colour pair.
 *
 *    Input:
 *        x - The horizontal offset of the location at which the first character
 *            is to be drawn, measuring from left to right, with 0 corresponding
 *            to the left-most visible character position in the terminal window.
 *
 *        y - The vertical offset of the location at which the first character
 *            is to be drawn, measuring from top to bottom, with 0 corresponding
 *            to the upper-most visible character position in the terminal window.
 *
 *        value - The numeric value to be displayed.
 *
 *    Output: void.
 *
 *    Notes:
 *    (1) This function is logically equivalent to
 *        draw_formatted(x, y, "%f", value).
 *
 *    (2) Use draw_formatted() to achieve advanced effects such as justification,
 *        padding, or hexadecimal representation.
 */
void draw_double(int x, int y, double value);

/**
 *    Draws formatted text, starting at the specified location. The rendered text
 *    is added to the zdk_screen buffer, but remains unseen until the next
 *    invocation of show_screen().
 *
 *    Input:
 *        x - The horizontal offset at which the first character is to be
 *            drawn. See draw_string() for further interpretation of this value.
 *
 *        y - The vertical offset at which the first character is to be drawn.
 *            See draw_string() for further interpretation of this value.
 *
 *        format - A string which is structured according to the requirements
 *            of sprintf() which will be used in conjunction with any subsequent
 *            arguments to generate the text for display.
 *
 *        ... - An optional sequence of additional arguments.
 *
 *    Output: void.
 *
 *    Notes:    Total length of generated text must be less than 1000 characters.
 *
 *    See also: Standard library function, sprintf().
 */
void draw_formatted(int x, int y, const char * format, ...);

/**
 *    Draws a line segment from (x1,y1) to (x2,y2) using the specified character.
 *
 *    Input:
 *        (x1,y1) - The offset coordinates of one endpoint of the line. These
 *                  arguments are interpreted in the same manner as (x,y) of
 *                  draw_char().
 *
 *        (x2,y2) - The offset coordinates of the other endpoint. These
 *                  arguments are interpreted in the same manner as (x,y) of
 *                  draw_char().
 *
 *        value - The symbol that is to be used to construct the line segment.
 *
 *    Output: void.
 */
void draw_line(int x1, int y1, int x2, int y2, char value);

/*
**    Draws the specified symbol at the prescribed (x,y) location in the terminal
**    window. The rendered character is added to the zdk_screen buffer, but
**    remains unseen until the next invocation of show_screen(). The character is
**    drawn with the current (foreground,background) colour pair.
**
**    Input:
**        x - The horizontal offset of the location at which the character
**            is to be drawn, measuring from left to right, with 0 corresponding
**            to the left-most visible character position in the terminal window.
**
**        y - The vertical offset of the location at which the character
**            is to be drawn, measuring from top to bottom, with 0 corresponding
**            to the upper-most visible character position in the terminal window.
**
**        colour - The colour (0..15) of the pixel to be displayed.
**
**    Output:
**        A solid-colour pixel has been added to the display at the designated (x,y) position.
**
**    Side effects:
**        No other side-effects occur. In particular, previous colour settings are preserved.
**
**    Notes:
**        This function produces the same visual result as:
**            set_colours( colour | INVERSE, colour );
**            draw_char( x, y, ' ' );
*/
void draw_pixel(int x, int y, int colour);

/**
 *    Draws a solid-colour line segment from (x1,y1) to (x2,y2).
 *
 *    Input:
 *        (x1,y1) - The offset coordinates of one endpoint of the line. These
 *                  arguments are interpreted in the same manner as (x,y) of
 *                  draw_char().
 *
 *        (x2,y2) - The offset coordinates of the other endpoint. These
 *                  arguments are interpreted in the same manner as (x,y) of
 *                  draw_char().
 *
 *        colour - The colour (0..15) that is to be used to construct the line segment.
 *
 *    Output: void.
 */
void draw_solid_line(int x1, int y1, int x2, int y2, int colour);

/**
 *    Gets the dimensions of the screen buffer.
 *
 *    Input:
 *        width - The address of a variable of type int in which the width
 *                of the logical screen will be stored.
 *
 *        height - The address of a variable of type int in which the height
 *                 of the logical screen will be stored.
 *
 *    Output: void.
 *
 *    Notes: The following blocks of code are logically equivalent.
 *        (a)     int w, h;
 *                get_screen_size( &w, &h );
 *
 *        (b)     int w = zdk_screen->width;
 *                int h = zdk_screen->height;
 */
void get_screen_size(int * width, int * height);

/**
 *    Returns the width of the zdk_screen buffer.
 *
 *    Input: void.
 *
 *    Output: The designated value.
 *
 *    Notes: The following blocks of code are logically equivalent.
 *        (a)        int w = screen_width();
 *
 *        (b)        int w = zdk_screen->width;
 */
int screen_width(void);

/**
 *    Returns the current height of the zdk_screen buffer.
 *
 *    Input: void.
 *
 *    Output: The designated value.
 *
 *    Notes: The following blocks of code are logically equivalent.
 *        (a)        int h = screen_height();
 *
 *        (b)        int h = zdk_screen->height;
 */
int screen_height(void);

/**
 *    Waits for and returns the next character from the standard input stream.
 *
 *    Input: void.
 *
 *    Output: See get_char() for a full description of the returned characters.
 */
int wait_char(void);

/**
 *    Immediately returns the next character from the standard input stream
 *    if one is available, or ERR if none is present.
 *
 *    If the save-screen state is true (see auto_save_screen) a record of the
 *    character returned is emitted to the current save-screen file.
 *
 *    Input: void.
 *
 *    Output: If the current input stream contains a keystroke, the character
 *            code of that key is returned. For regular characters, this will
 *            be the ASCII code of the key. Extended key codes are reported for
 *            additional characters, such as function keys and arrow keys. In
 *            addition, special codes are returned to signal screen resize and
 *            mouse events.
 *
 *            The return value is KEY_MOUSE if a mouse event is available. In
 *            that eventuality, you can use get_mouse_buttons, get_mouse_x, and
 *            get_mouse_y to find the state of the mouse.
 *
 *    See also: <curses.h> for a full list of extended key codes.
 *
 *    Notes:    (Advanced) If the zdk_input_stream is non-null, that stream will be used as a
 *            source rather than the standard input stream.
 */

int get_char(void);

/*
** Returns the latest reported horizontal location of the mouse.
*/
int get_mouse_x();

/*
** Returns the latest reported vertical location of the mouse.
*/
int get_mouse_y();

/*
**    Returns the latest reported button state of the mouse.
**
**    Mouse button modifiers are:
**
**    BUTTON1_RELEASED, BUTTON1_PRESSED, BUTTON1_CLICKED, BUTTON1_DOUBLE_CLICKED
**    BUTTON1_TRIPLE_CLICKED
**
**    BUTTON2_RELEASED, BUTTON2_PRESSED, BUTTON2_CLICKED, BUTTON2_DOUBLE_CLICKED
**    BUTTON2_TRIPLE_CLICKED
**
**    BUTTON3_RELEASED, BUTTON3_PRESSED, BUTTON3_CLICKED, BUTTON3_DOUBLE_CLICKED
**    BUTTON3_TRIPLE_CLICKED
**
**    BUTTON4_RELEASED, BUTTON4_PRESSED, BUTTON4_CLICKED, BUTTON4_DOUBLE_CLICKED
**    BUTTON4_TRIPLE_CLICKED
**
**    BUTTON5_RELEASED, BUTTON5_PRESSED, BUTTON5_CLICKED, BUTTON5_DOUBLE_CLICKED
**    BUTTON5_TRIPLE_CLICKED
**
**    BUTTON_CTRL, BUTTON_SHIFT, BUTTON_ALT, REPORT_MOUSE_POSITION
**
*/
unsigned long get_mouse_buttons();

/*
**    Saves a screen shot to a file having the designated name. Upon
**    successful execution, the contents of designated file has been
**    replaced by the contents of the zdk_screen buffer.
**
**    Input:
**        file_name - a string holding the name of the file.
**
**    Output:
**        void.
**
**    Notes:
**        This DOES NOT save colour information.
*/
void save_screen(const char * file_name);

/*
**    Automatically save a screen shot each time show_screen is called
**    if this is non-zero.
**
**    Input:
**        save_if_true - a boolean value which becomes the new save-screen state.
**
**    Notes:
**        (1)    Whenever the save-screen state switches from false to true, a new file
**            with a name of the form "zdk_screen.N.txt" is created. Here N is
**            a sequential number starting from 1. This file will accumulate a copy
**            of the program interaction until the save-screen state becomes false.
**
**        (2) The saved data file can be displayed with the CAB202 Movie Player web
**            application.
*/
void auto_save_screen(bool save_if_true);

/**
 *    Reallocates zdk_screen and zdk_prev_screen buffers to the designated
 *    dimensions.
 *
 *    Input:
 *        width - A strictly positive integer which specifies the required
 *                buffer width.
 *
 *        height - A strictly positive integer which specifies the required
 *                buffer height.
 *
 *    Output: void.
 *
 *    Notes:  This function is provided to support automated testing
 *            of code that adapts to a changing screen size.
 */
void override_screen_size(int width, int height);

/**
 *    Resets the zdk_screen and zdk_prev_screen buffers to match the
 *    dimensions of the terminal window.
 *
 *    Input: void.
 *
 *    Output: void.
 *
 *    Notes:
 *        You can use get_screen_size() to find the dimensions of the terminal
 *        window. Compare the current dimensions with the previous dimensions. If
 *        width or height has changed, _then_ invoke this function. You will
 *        probably have to recalculate the entire view to ensure objects are not
 *        lost outside the visible display area.
 */
void fit_screen_to_window(void);

/*
**    A list of known colours.
*/
#define BLACK         (0)
#define RED            (1)
#define GREEN         (2)
#define YELLOW        (3)
#define BLUE        (4)
#define MAGENTA        (5)
#define CYAN        (6)
#define WHITE        (7)

#define NUM_COLOURS    (8)

#define BRIGHT (8)
#define INVERSE (16)
#define TRANSPARENT (32)

#define BRIGHT_BLACK    (BRIGHT | BLACK)
#define BRIGHT_RED        (BRIGHT | RED)
#define BRIGHT_GREEN     (BRIGHT | GREEN)
#define BRIGHT_YELLOW    (BRIGHT | YELLOW)
#define BRIGHT_BLUE        (BRIGHT | BLUE)
#define BRIGHT_MAGENTA    (BRIGHT | MAGENTA)
#define BRIGHT_CYAN        (BRIGHT | CYAN)
#define BRIGHT_WHITE    (BRIGHT | WHITE)

/*
**    Sets the background colour for future drawing operations.
**
**    Input:
**        colour - The new background colour, which must be between 0 and
**            (NUM_COLOURS-1), inclusive. That is, you cannot explicitly set
**            a BRIGHT background.
**
**    Output:
**        void.
**
**    Side effects:
**        The new background colour has been saved for subsequent use. This
**        affects draw_char, draw_line, draw_int, draw_double, draw_string and
**        draw_formatted.
**
**    Notes:
**    .    If the supplied colour is out of bounds, the program will terminate
**        abruptly with an assertion.
*/
void set_background(int colour);

/*
**    Sets the foreground colour for future drawing operations.
**
**    Input:
**        colour - The new foreground colour, which must be between 0 and
**            (NUM_COLOURS-1) | BRIGHT | INVERSE | TRANSPARENT, inclusive.
**
**            The TRANSPARENT colour is used when rendering a sprite. It may be removed.
**
**    Output:
**        void.
**
**    Side effects:
**        The new foreground colour has been saved for subsequent use. This
**        affects drawing operations.
**
**    Notes:
**    .    If the supplied colour is out of bounds, the program will terminate
**        abruptly with an assertion.
**
**    See also:
**        draw_char, draw_line, draw_int, draw_double, draw_string,
**        draw_formatted.
*/
void set_foreground(int colour);

/*
**    Gets the background colour that will be used for future drawing operations.
**
**    Input:
**        void.
**
**    Output:
**        A value between 0 and (NUM_COLOURS-1), inclusive.
**
**    Side effects:
**        None.
*/
int get_background(void);

/*
**    Gets the foreground colour that will be used for future drawing operations.
**
**    Input:
**        void.
**
**    Output:
**        A value between 0 and (NUM_COLOURS-1) | BRIGHT | INVERSE | TRANSPARENT, inclusive.
*/
int get_foreground(void);

/*
**    Sets both foreground and background colour for subsequent drawing operations.
**
**    Input:
**        foreground - the colour index for text foreground. This may be OR'd with
**            BRIGHT, INVERSE, or TRANSPARENT modifiers if desired.
**
**        background - the colour for the text background.
**
**    Returns:
**        void.
**
**    Side effects:
**        The designated colours are saved for later use, superceding any previous
**        colour pair.
**
**    See also:
**        set_foreground, set_background, get_colours.
*/
void set_colours(int foreground, int background);

/*
**    Gets both foreground and background colour. If any colour modifiers (BRIGHT,
**    INVERSE, or TRANSPARENT) are saved, they will be returned as part of the
**    value refered to by foreground.
**
**    Input:
**        foreground, background - the addresses of a pair of variables in which the
**            saved colour pair will be passed back to caller.
**
**    Returns:
**        void.
**
**    Side effects:
**        None.
**
**    See also:
**        get_foreground, get_background, set_colours.
*/
void get_colours(int *foreground, int *background);

/*
**    Retrieves a character value out of the most recently displayed screen buffer.
**
**    Input:
**        x    The horisontal location from which to read.
**        y    The vertical location from which to read.
**
**    Output:
**        The char code of the symbol that is currently visible at (x,y).
**
**    Side effects:
**        None.
**
**    Notes:
**        Screen-scraping is sometimes convenient or necessary, but it is also a
**        very poor practice. Gnerally, do not store important data in the
**        display! However, for those moments when it _is_ the right thing to do,
**        here is a way to do it.
*/
char scrape_char(int x, int y);

// ------------------------------------------------------------------
//    Advanced facilities to support automated testing.
// ------------------------------------------------------------------

/**
 *    Override standard output stream.
 *
 *    An output stream which, if not NULL, will receive a copy of all key
 *    events detected by get_char/wait_char plus copies of all displayed
 *    screens.
 *
 *    Notes:
 *    (1)    To enable screen capture automatically, use the
 *        auto_save_screen function rather than attempting to manipulate
 *        this object.
 *    (2)    If you specify an exotic stream such as a memory stream you will
 *        probably have to disable curses functionality.
 */
FILE * zdk_save_stream;

/**
 *    Override standard input stream.
 *
 *    An input stream which, if not NULL, will replace the standard
 *    input used by get_char/wait_char as the source of keyboard events.
 *
 *    Note: A similar effect can be achieved by using command-line
 *    redirection to pipe input from a text file. You may find it
 *    easier to use that rather than attempting to work with this interface.
 */
FILE * zdk_input_stream;

/**
 *    Override: disable all curses functionality
 *
 *    A flag which, if true, blocks curses functionality. Useful mainly in a unit test scenario.
 *
 *
 *    If you _do_ decide to play with this, the safest path is to set it true
 *    before calling setup_screen(), and don't change it back to false
 *    until after calling cleanup_screen() at the end of the program run.
 */
bool zdk_suppress_output;

/**
 *    Disable ncurses and restore the terminal to its normal operational state.
 *
 *    IF YOU ARE NOT A ZDK EXPERT, YOU PROBABLY SHOULD NOT EVER CALL THIS FUNCTION!
 *
 *    Notes:
 *    .    Under normal operating circumstances, this function will be called
 *        automatically as part of a clean program termination sequence.
 *    .    Therefore, you will rarely call this function explicitly.
 *
 *    Side effects:
 *        After calling this function, graphics WILL break. Don't call this function.
 */
void cleanup_screen(void);

#endif /* GRAPHICS_H_ */
