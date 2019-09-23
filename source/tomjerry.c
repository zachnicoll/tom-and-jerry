#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

#define DELAY 10
#define HEIGHT (double)screen_height()
#define WIDTH (double)screen_width()
#define MINSPEED 0.1
#define WALL '*'

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

bool game_over, level_over, pause;

int setup_players, total_levels;
int cheese, cheese_collected, traps, trap_supply, fireworks, current_level = 1, current_player;
int cheese_positions[5][2], trap_positions[5][2], door_position[2];
double pause_start, pause_end, pause_time, game_time, cheese_time, trap_time, firework_time, STARTTIME;
struct player
{
    int points, lives, level_points;
    double initx, inity, xpos, ypos, speed, direction;
    char symbol;
} jerry, tom, firework;

/////////////////////////////////////////////////////
////////////////FUNC DECLARATIONS////////////////////

/* For exaplanations of each functions use and functionality, see this section.*/

/*///////////////*/
/* Drawing Funcs */

// Draw the room from a specified FILE pointer.
void draw_room(FILE *stream);

// Draws status bar, displaying score, lives, current player and more.
void draw_hud();

// Draws the game over screen and waits for either Q or R to (Q)uit the game or (R)estart the level.
void draw_game_over(char key);

// Draws Tom and Jerry at their current rounded x and y positions.
void draw_players();

// Draws cheese, traps, fireworks, and the door.
void draw_objects();

// Executes all drawing functions. Takes a char pointer with the name of the current room's .txt file. This is converted into a FILE* and parsed to draw_room().
void draw_all(char *current_room);

/* Drawing Funcs */
/*///////////////*/

/*//////////////*/
/*Gameplay Funcs*/

// After any point is scored, check_win is called to see if 5 cheese have been collected by Jerry, or Tom has scored 5 points. If so, spawn the Door.
void check_win();

// Checks is the firework has collided with Tom and handles point calculations. If it hasn't then go to firework_homing to move closer to Tom.
void update_firework();

// After checking collisions in update_firework(), firework_homing is called to advance the firework's position closer to Tom.
void firework_homing();

// Check the value of key_pressed; if it is a directional value (WASD) then move the current player.
// If it is an action value, then shoot a firework, place a trap etc.
void update_movement(int key_pressed, struct player *plyr);

// Check both cheese and trap collisions at once and perform actions based on the current player.
void check_cheese_trap_collisions();

// Check the associated collisions with Jerry as a player. i.e. if he collides with cheese, add 1 to his points.
void check_jerry_collisions();

// Check the associated collisions with Tom as a player. i.e. if he collides with a firework, take 1 away from Tom's lives.
void check_tom_collisions();

// Update the movement of the current player, as well as check for associated collisions. Parse in the key pressed, and a pointer to the current player.
void update_player(int key_pressed, struct player *plyr);

// Updates Tom's random movement by calling move_random();
void update_tom();

// Move the input player (via reference) randomly around the screen,
// changing speed and direction every time it collides with a wall.
void move_random(struct player *plyr);

// Move the player automatically with a dx and dy, checking for wall collisions.
void move_auto_player(struct player *plyr, double dx, double dy);

// Calculate the required x and y movements for Tom to reach Jerry, and parse them into move_auto_player. This function controls Toms "seeking" behaviour.
void update_tom_advanced();

// Check every cheese's position relative to Jerry. If the cheese is less than 10 units away, then chase_cheese() on that cheese's index in cheese_positions.
void seek_cheese();

// Tell automated Jerry to chase a specified cheese and index i in the cheese_positions array.
void chase_cheese(int i);

// If Tom is within 5 units of Jerry, Jerry will run in the opposite direction.
void escape_tom(double x, double y, double d);

// Update automated Jerry and handle his firewokr shooting, cheese seeking, and Tom avoidance.
void update_jerry();

// Update the "enemy" player i.e. the current automated player.
void update_enemy();

// Checks collisions relative to current player's x and y displacement, with specified symbol.
int check_collision(struct player plyr, char symbol, double xd, double yd);

// Handle loss of life events.
void lose_life();

// Place the cheese and traps based on 2 and 3 second intervals respectively. Also only places a trap automatically when Tom is NOT the player.
void place_cheese_traps();

// Place a trap at Tom's x and y position.
void place_trap();

// Place a cheese at either a random position on the screen (auto_place == 'A') or at Tom's position (auto_place == 'M').
void place_cheese(char auto_place);

// Handle the end of the level, either by death or reaching the door.
// Condition 'Q' goes to game over screen.
// Condition 'N' goes to the next level.
void level_end(char condition);

// Performs required pause time calculations.
void paused();

/*Gameplay Funcs*/
/*//////////////*/

/*Main funcs*/
/*//////////*/

// Reset all variable back to initial values, including game time, points, lives etc.
void setup();

// Calls all necessary functions for the game's loop, including draw_all, update_player etc.
// Additonally takes a char* to the current room's .txt file and parses it into draw_all().
void loop(char *current_room);

/*Main Funcs*/
/*//////////*/

////////////////FUNC DECLARATIONS////////////////////
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
/////////////////DRAWING EVENTS//////////////////////

void draw_room(FILE *stream)
{

    while (!feof(stream))
    {
        char command;
        double x1, y1, x2, y2;

        int arg_count = fscanf(stream, "%c %lf %lf %lf %lf", &command, &x1, &y1, &x2, &y2);

        if (arg_count == 3 && setup_players < 2)
        {
            if (command == 'T')
            {
                tom.initx = round(x1 * (WIDTH - 1));
                if (round(y1 * (HEIGHT) + 5) > HEIGHT)
                {
                    tom.inity = round(y1 * (HEIGHT)-1);
                }
                else
                {
                    tom.inity = round(y1 * (HEIGHT) + 5);
                }
                tom.xpos = tom.initx;
                tom.ypos = tom.inity;
                tom.symbol = 'T';
                draw_char(tom.xpos, tom.ypos, tom.symbol);
            }
            else if (command == 'J')
            {
                jerry.initx = round(x1 * (WIDTH - 1));
                if (round(y1 * (HEIGHT) + 5) > HEIGHT)
                {
                    jerry.inity = round(y1 * (HEIGHT)-1);
                }
                else
                {
                    jerry.inity = round(y1 * (HEIGHT) + 5);
                }
                jerry.xpos = jerry.initx;
                jerry.ypos = jerry.inity;
                jerry.symbol = 'J';
                draw_char(jerry.xpos, jerry.ypos, jerry.symbol);
            }
            setup_players++;
        }
        else if (arg_count == 5)
        {
            if (command == 'W')
            {
                draw_line(round(x1 * WIDTH), round(y1 * HEIGHT + 4), round(x2 * WIDTH), round(y2 * HEIGHT + 4), WALL);
            }
        }
    }
}

void draw_hud()
{
    char str_buffer[50];

    draw_string(0, 0, "Student Number: n10214453");

    if (current_player == 'J')
    {
        sprintf(str_buffer, "Score: %d", jerry.points);
    }
    else
    {
        sprintf(str_buffer, "Score: %d", tom.points);
    }
    draw_string(10 + WIDTH / 5, 0, str_buffer);

    sprintf(str_buffer, "Lives: %d", current_player == 'J' ? jerry.lives : tom.lives);
    draw_string(10 + 2 * WIDTH / 5, 0, str_buffer);

    sprintf(str_buffer, "Player: %c", current_player);
    draw_string(10 + 3 * WIDTH / 5, 0, str_buffer);

    int i_minutes = floor(game_time / 60);
    double fl_minutes = game_time / 60;
    double fraction = fl_minutes - floor(fl_minutes);
    int seconds = 60 * fraction;

    draw_formatted(10 + 4 * WIDTH / 5, 0, "Time: %02d:%02d", i_minutes, seconds);

    sprintf(str_buffer, "Cheese: %d", cheese);
    draw_string(0, 3, str_buffer);

    sprintf(str_buffer, "Traps: %d", traps);
    draw_string(10 + WIDTH / 5, 3, str_buffer);

    sprintf(str_buffer, "Fireworks: %d", fireworks);
    draw_string(10 + 2 * WIDTH / 5, 3, str_buffer);

    sprintf(str_buffer, "Level: %d", current_level);
    draw_string(10 + 3 * WIDTH / 5, 3, str_buffer);

    draw_line(0, 4, WIDTH, 4, '-');
}

void draw_game_over(char key)
{
    clear_screen();
    draw_string(WIDTH / 2 - strlen("---------GAME OVER---------") / 2, HEIGHT / 2, "---------GAME OVER---------");
    draw_string(WIDTH / 2 - strlen("Press Q to Quit, or R to Restart.") / 2, HEIGHT / 2 + 5, "Press Q to Quit, or R to Restart.");
    if (key == 'q')
    {
        game_over = true;
    }
    else if (key == 'r')
    {
        setup();
        current_level = 1;
        jerry.xpos = jerry.initx;
        jerry.ypos = jerry.inity;
        tom.xpos = tom.initx;
        tom.ypos = tom.inity;
    }

    show_screen();
}

void draw_players()
{
    draw_char(round(jerry.xpos), round(jerry.ypos), jerry.symbol);
    draw_char(round(tom.xpos), round(tom.ypos), tom.symbol);
}

void draw_objects()
{
    for (int i = 0; i < 5; i++)
    {
        draw_char(cheese_positions[i][0], cheese_positions[i][1], '>');
        draw_char(trap_positions[i][0], trap_positions[i][1], '#');
    }

    draw_char(door_position[0], door_position[1], 'X');
    draw_char(round(firework.xpos), round(firework.ypos), '~');
}

void draw_all(char *current_room)
{
    clear_screen();

    FILE *stream = fopen(current_room, "r");
    if (stream != NULL)
    {
        draw_room(stream);
        fclose(stream);
    }

    draw_hud();
    draw_players();
    draw_objects();

    show_screen();
}

/////////////////DRAWING EVENTS//////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
/////////////////GAMEPLAY FUNCTIONS/////////////////

void check_win()
{
    if (door_position[0] == -1 && (cheese_collected == 5 || tom.level_points >= 5))
    {
        int x, y;

        do
        {
            x = round(((double)rand() / (double)RAND_MAX) * (WIDTH - 1));
            y = round(((double)rand() / (double)RAND_MAX) * (HEIGHT - 4)) + 4;
        } while (scrape_char(x, y) != ' ');

        door_position[0] = x;
        door_position[1] = y;
    }
}

void firework_homing()
{
    double t1 = tom.xpos - firework.xpos;
    double t2 = tom.ypos - firework.ypos;
    double d = sqrt(t1 * t1 + t2 * t2);

    double dx = t1 * (0.200 / d);
    double dy = t2 * (0.200 / d);

    if (firework.xpos + dx < WIDTH - 1 && firework.xpos + dx > 1 && firework.ypos + dy < HEIGHT - 1 && firework.ypos + dy > 5)
    {
        if (!check_collision(firework, WALL, dx, 0) && !check_collision(firework, WALL, 0, dy))
        {
            firework.xpos += dx;
            firework.ypos += dy;
        }
        else
        {
            firework.xpos = -1;
            firework.ypos = -1;
            fireworks--;
        }
    }
    else
    {
        firework.xpos = -1;
        firework.ypos = -1;
        fireworks--;
    }
}

void update_firework()
{
    if (round(firework.xpos) == round(tom.xpos) && round(firework.ypos) == round(tom.ypos))
    {
        tom.xpos = tom.initx;
        tom.ypos = tom.inity;

        firework.xpos = -1;
        firework.ypos = -1;
        fireworks--;

        if (current_player == 'J')
        {
            jerry.points++;
        }
    }
    else if (firework.xpos != -1 && fireworks > 0 && !pause)
    {
        firework_homing();
    }
}

void update_movement(int key_pressed, struct player *plyr)
{
    plyr->xpos = round(plyr->xpos);
    plyr->ypos = round(plyr->ypos);
    if (key_pressed == 'w' && plyr->ypos > 5 && !check_collision(*plyr, WALL, 0, -1))
    {
        plyr->ypos -= 1;
    }
    else if (key_pressed == 'a' && plyr->xpos > 0 && !check_collision(*plyr, WALL, -1, 0))
    {
        plyr->xpos -= 1;
    }
    else if (key_pressed == 's' && plyr->ypos < HEIGHT - 1 && !check_collision(*plyr, WALL, 0, 1))
    {
        plyr->ypos += 1;
    }
    else if (key_pressed == 'd' && plyr->xpos < WIDTH - 1 && !check_collision(*plyr, WALL, 1, 0))
    {
        plyr->xpos += 1;
    }
    else if (key_pressed == 'p')
    {
        paused();
    }
    else if (key_pressed == 'f' && firework.xpos == -1 && plyr->symbol == 'J' && current_level > 1)
    {
        firework.xpos = jerry.xpos;
        firework.ypos = jerry.ypos;
        fireworks++;
    }
    else if (key_pressed == 'z' && current_level > 1)
    {
        current_player = current_player == 'J' ? 'T' : 'J';
    }
    else if (key_pressed == 'm' && current_player == 'T' && traps < 5)
    {
        place_trap();
    }
    else if (key_pressed == 'c' && current_player == 'T' && cheese < 5)
    {
        place_cheese('M');
    }
}

void check_cheese_trap_collisions()
{
    for (int i = 0; i < 5; i++)
    {
        if (current_player == 'J')
        {
            if (round(jerry.xpos) == cheese_positions[i][0] && round(jerry.ypos) == cheese_positions[i][1])
            {
                jerry.points++;
                cheese_collected++;
                cheese_positions[i][0] = -1;
                cheese_positions[i][1] = -1;
                cheese--;

                check_win();
            }

            if (round(jerry.xpos) == trap_positions[i][0] && round(jerry.ypos) == trap_positions[i][1])
            {
                trap_positions[i][0] = -1;
                trap_positions[i][1] = -1;
                traps--;
                lose_life();
            }
        }
        else
        {
            if (round(jerry.xpos) == cheese_positions[i][0] && round(jerry.ypos) == cheese_positions[i][1])
            {
                cheese_positions[i][0] = -1;
                cheese_positions[i][1] = -1;
                cheese--;
            }

            if (round(jerry.xpos) == trap_positions[i][0] && round(jerry.ypos) == trap_positions[i][1])
            {
                trap_positions[i][0] = -1;
                trap_positions[i][1] = -1;
                traps--;
                tom.points++;
                tom.level_points++;

                jerry.xpos = jerry.initx;
                jerry.ypos = jerry.inity;

                check_win();
            }
        }
    }
}

void check_jerry_collisions()
{
    if (check_collision(jerry, 'X', 0, 0))
    {
        level_end('N');
    }

    if (check_collision(jerry, 'T', 0, 0))
    {
        lose_life();
    }

    check_cheese_trap_collisions();
}

void check_tom_collisions()
{
    if (check_collision(tom, 'J', 0, 0))
    {
        tom.points += 5;
        tom.level_points += 5;
        check_win();
        jerry.xpos = jerry.initx;
        jerry.ypos = jerry.inity;
        tom.xpos = tom.initx;
        tom.ypos = tom.inity;
    }

    if (check_collision(tom, 'X', 0, 0))
    {
        level_end('N');
    }

    if (check_collision(tom, '~', 0, 0))
    {
        firework.xpos = -1;
        firework.ypos = -1;
        fireworks--;
        lose_life();
    }
}

void update_player(int key_pressed, struct player *plyr)
{
    update_movement(key_pressed, plyr);

    if (plyr->symbol == 'J')
    {
        check_jerry_collisions();
    }
    else
    {
        check_tom_collisions();
    }
}

void update_tom()
{
    struct player *plyr = &tom;
    move_random(plyr);
}

void move_random(struct player *plyr)
{
    double dx = cos(plyr->direction) * plyr->speed;
    double dy = sin(plyr->direction) * plyr->speed;

    if ((plyr->xpos + dx > WIDTH - 1) || (plyr->xpos + dx < 0) || (plyr->ypos + dy > HEIGHT - 1) || (plyr->ypos + dy < 5) || check_collision(*plyr, WALL, dx, dy) || check_collision(*plyr, WALL, dx, 0) || check_collision(*plyr, WALL, 0, dy))
    {
        plyr->speed = (double)rand() / (double)RAND_MAX * MINSPEED + MINSPEED;
        plyr->direction = ((double)rand() / (double)RAND_MAX) * M_PI * 2;
    }
    else
    {
        if ((plyr->xpos + dx < WIDTH - 1) && (plyr->xpos + dx > 0))
        {
            plyr->xpos += dx;
        }

        if (plyr->ypos + dy < HEIGHT - 1 && plyr->ypos + dy > 5)
        {
            plyr->ypos += dy;
        }
    }
}

void move_auto_player(struct player *plyr, double dx, double dy)
{
    if (!check_collision(*plyr, WALL, dx, 0) && !check_collision(*plyr, WALL, 0, dy) && (round(plyr->xpos) + dx < WIDTH - 1) && (round(plyr->xpos) + dx > 0) && (round(plyr->ypos) + dy < HEIGHT - 1) && (round(plyr->ypos) + dy > 5))
    {
        plyr->xpos += dx;
        plyr->ypos += dy;
    }
    else if (!check_collision(*plyr, WALL, dx, 0) && (round(plyr->xpos) + dx < WIDTH - 1) && (round(plyr->xpos) + dx > 0))
    {
        plyr->xpos += dx;
    }
    else if (!check_collision(*plyr, WALL, 0, dy) && (round(plyr->ypos) + dy < HEIGHT - 1) && (round(plyr->ypos) + dy > 5))
    {
        plyr->ypos += dy;
    }
}

void update_tom_advanced()
{
    double t1 = jerry.xpos - tom.xpos;
    double t2 = jerry.ypos - tom.ypos;
    double d = sqrt(t1 * t1 + t2 * t2);

    double dx = t1 * (0.08 / d);
    double dy = t2 * (0.08 / d);
    struct player *plyr = &tom;
    move_auto_player(plyr, dx, dy);
}

void chase_cheese(int i)
{
    double x, y, d, dx, dy;

    x = cheese_positions[i][0] - round(jerry.xpos);
    y = cheese_positions[i][1] - round(jerry.ypos);
    d = sqrt(x * x + y * y);

    dx = x * (0.1 / d);
    dy = y * (0.1 / d);

    struct player *plyr = &jerry;
    move_auto_player(plyr, dx, dy);

    if (round(jerry.xpos) == cheese_positions[i][0] && round(jerry.ypos) == cheese_positions[i][1])
    {
        cheese_positions[i][0] = -1;
        cheese_positions[i][1] = -1;
        cheese--;
    }
}

void seek_cheese()
{
    double x_to_cheese, y_to_cheese, d_to_cheese, nearest = WIDTH;
    int cheese_index = -1;
    for (int i = 0; i < 5; i++)
    {
        if (cheese_positions[i][0] != -1)
        {
            x_to_cheese = cheese_positions[i][0] - jerry.xpos;
            y_to_cheese = cheese_positions[i][1] - jerry.ypos;
            d_to_cheese = sqrt(x_to_cheese * x_to_cheese + y_to_cheese * y_to_cheese);

            if (d_to_cheese < nearest)
            {
                nearest = d_to_cheese;
                cheese_index = i;
            }
        }
    }

    if (cheese_index != -1 && nearest <= 10)
    {
        chase_cheese(cheese_index);
    }
    else
    {
        struct player *plyr = &jerry;
        move_random(plyr);
    }
}

void escape_tom(double x, double y, double d)
{
    double dx, dy;

    dx = -x * (0.1 / d);
    dy = -y * (0.1 / d);

    struct player *plyr = &jerry;
    move_auto_player(plyr, dx, dy);
}

void update_jerry()
{
    double current_time = round(get_current_time());
    double x_to_tom = tom.xpos - jerry.xpos;
    double y_to_tom = tom.ypos - jerry.ypos;
    double d_to_tom = sqrt(x_to_tom * x_to_tom + y_to_tom * y_to_tom);

    if (d_to_tom > 5)
    {
        seek_cheese();
    }
    else
    {
        escape_tom(x_to_tom, y_to_tom, d_to_tom);
    }

    if (current_time - firework_time == 5 && !pause)
    {
        firework.xpos = jerry.xpos;
        firework.ypos = jerry.ypos;
        fireworks++;
        firework_time = round(get_current_time());
    }
    else if (pause || current_time - firework_time > 5)
    {
        firework_time = round(get_current_time());
    }

    check_cheese_trap_collisions();
}

void update_enemy()
{
    if (!pause)
    {
        if (current_player == 'J')
        {
            if (current_level == 1)
            {
                update_tom();
            }
            else
            {
                update_tom_advanced();
            }
        }
        else
        {
            update_jerry();
        }
    }
}

int check_collision(struct player plyr, char symbol, double xd, double yd)
{
    bool is_colliding = false;

    if (xd > 0)
    {
        xd = 1;
    }
    else if (xd < 0)
    {
        xd = -1;
    }

    if (yd > 0)
    {
        yd = 1;
    }
    else if (yd < 0)
    {
        yd = -1;
    }

    if (scrape_char(round(plyr.xpos) + xd, round(plyr.ypos) + yd) == symbol)
    {
        is_colliding = true;
    }

    return is_colliding;
}

void paused()
{
    pause = !pause;
    if (pause)
    {
        pause_start = get_current_time();
        pause_end = 0;
    }
    else
    {
        pause_end = get_current_time();
        pause_time += pause_end - pause_start;
    }
}

void lose_life()
{
    jerry.xpos = jerry.initx;
    jerry.ypos = jerry.inity;
    tom.xpos = tom.initx;
    tom.ypos = tom.inity;
    if (current_player == 'J')
    {
        jerry.lives--;
    }
    else
    {
        tom.lives--;
    }

    if (jerry.lives == 0 || tom.lives == 0)
    {
        level_end('Q');
    }
}

void place_cheese_traps()
{
    int current_time = round(get_current_time());

    if (cheese < 5 && current_time - cheese_time == 2 && !pause)
    {
        place_cheese('A');
    }
    else if (cheese == 5 || pause)
    {
        cheese_time = round(get_current_time());
    }

    if (trap_supply > 0 && current_time - trap_time == 3 && !pause && current_player != 'T')
    {
        place_trap();
    }
    else if (traps == 5 || pause)
    {
        trap_time = round(get_current_time());
    }
}

void place_trap()
{
    for (int i = 0; i < 5; i++)
    {
        if (trap_positions[i][0] == -1)
        {
            trap_positions[i][0] = round(tom.xpos);
            trap_positions[i][1] = round(tom.ypos);
            traps++;
            break;
        }
    }
    trap_time = round(get_current_time());
}

void place_cheese(char auto_place)
{
    int x, y;
    if (auto_place == 'A')
    {
        x = round(((double)rand() / (double)RAND_MAX) * (WIDTH - 1));
        y = round(((double)rand() / (double)RAND_MAX) * (HEIGHT - 4)) + 4;

        if (scrape_char(x, y) == ' ')
        {
            for (int i = 0; i < 5; i++)
            {
                if (cheese_positions[i][0] == -1)
                {
                    cheese_positions[i][0] = x;
                    cheese_positions[i][1] = y;
                    cheese++;
                    break;
                }
            }
        }
    }
    else
    {
        x = tom.xpos;
        y = tom.ypos;
        for (int i = 0; i < 5; i++)
        {
            if (cheese_positions[i][0] == -1)
            {
                cheese_positions[i][0] = x;
                cheese_positions[i][1] = y;
                cheese++;
                break;
            }
        }
    }
    cheese_time = round(get_current_time());
}

void level_end(char condition)
{
    if (condition == 'Q')
    {
        level_over = true;
        jerry.points = 0;
        tom.points = 0;
        jerry.lives = 5;
        tom.lives = 5;
    }
    else if (condition == 'N')
    {
        current_level++;
        setup();
    }
}
/////////////////GAMEPLAY FUNCTIONS/////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
//////////////////MAIN FUNCTIONS///////////////////

void loop(char *current_room)
{
    int key = get_char();

    if (current_level > total_levels)
    {
        level_end('Q');
    }

    if (!level_over)
    {
        double current_time = get_current_time();
        if (!pause)
        {
            game_time = current_time - STARTTIME - pause_time;
        }
        struct player *plyrPntr = current_player == 'J' ? &jerry : &tom;
        update_firework();
        update_enemy();
        place_cheese_traps();
        draw_all(current_room);
        update_player(key, plyrPntr);
    }
    else
    {
        draw_game_over(key);
    }
}

void setup()
{
    srand(get_current_time());
    STARTTIME = get_current_time();

    game_over = false;
    level_over = false;
    pause = false;

    current_player = 'J';
    setup_players = 0;

    jerry.speed = (double)rand() / (double)RAND_MAX * MINSPEED + MINSPEED;
    jerry.direction = ((double)rand() / (double)RAND_MAX) * M_PI * 2;

    tom.speed = (double)rand() / (double)RAND_MAX * MINSPEED + MINSPEED;
    tom.direction = ((double)rand() / (double)RAND_MAX) * M_PI * 2;
    tom.level_points = 0;

    firework.xpos = -1;
    firework.ypos = -1;

    cheese = 0;
    cheese_collected = 0;
    traps = 0;
    trap_supply = 5;
    fireworks = 0;

    int reset_objs[5][2] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}};
    memcpy(cheese_positions, reset_objs, sizeof(reset_objs));
    memcpy(trap_positions, reset_objs, sizeof(reset_objs));

    int reset_door[2] = {-1, 1};
    memcpy(door_position, reset_door, sizeof(reset_door));

    pause_time = 0;
    cheese_time = round(STARTTIME);
    trap_time = round(STARTTIME);
    firework_time = round(STARTTIME);
}

int main(int argc, char *argv[])
{
    setup_screen();
    setup();
    jerry.points = 0;
    tom.points = 0;
    jerry.lives = 5;
    tom.lives = 5;
    total_levels = argc - 1;

    while (game_over == false)
    {
        loop(argv[current_level]);
        timer_pause(DELAY);
    }

    return 0;
}

//////////////////MAIN FUNCTIONS///////////////////
////////////////////////////////////////////////////