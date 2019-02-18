#include "koules.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef AMIGA
    #include <devices/timer.h>
#endif

#define MIN(a,b) ((a)>(b)?(b):(a))

MODULE int             helpmode;
MODULE char            str[2];
MODULE struct
{   int x, y, xp, yp, time, color;
} point[MAXPOINT];
MODULE struct
{   int mask,
        mx, my;
} controls[MAXROCKETS];

IMPORT int             controller[MAXROCKETS],
                       inactive,
                       level;
#ifdef AMIGA
    IMPORT UBYTE       display[WINHEIGHT][WINWIDTH],
                       fgc,
                       pens[256];
#endif
#ifdef WIN32
    IMPORT ULONG       display[WINHEIGHT][WINWIDTH],
                       fgc;
    IMPORT const ULONG pens[256];
    IMPORT FLAG        iconified;
#endif

EXPORT struct KoulesObject object[MAXOBJECT];
EXPORT UBYTE           penter = 0, pfire = 0, pup = 0, pdown = 0, ph = 0, pesc = 0, pleft = 0, pright = 0,
                       rocketcolor[MAXROCKETS] = { OFFSET_YELLOW,
                                                   OFFSET_BLUE,
                                                   OFFSET_RED,
                                                   OFFSET_PURPLE,
                                                   OFFSET_GREEN,
                                                 };
EXPORT FLAG            paused         = FALSE;
EXPORT int             startlevel     =  0,
                       maxlevel       = 99, // means 100
                       maxpoint       = MAXPOINT,
                       dosprings      =  0,
                       jx[4], jy[4],
                       randsprings    =  0,
                       gameplan       = COOPERATIVE,
                       npoint         =  0,
                       gamemode       = MENU,
                       keys[3][4],
                       a_bballs,
                       a_rockets,
                       a_balls,
                       a_holes,
                       a_apples,
                       a_inspectors,
                       a_lunatics,
                       a_eholes,
                       nobjects       =  8,
                       nrockets       =  1,
                       difficulty     =  2,
                       MouseButtons   =  0,
                       MouseX,
                       MouseY;
EXPORT float           ROCKET_SPEED   = (float)  1.2,
                       BALL_SPEED     = (float)  1.2,
                       BBALL_SPEED    = (float)  1.2,
                       SLOWDOWN       = (float)  0.8,
                       GUMM           =         20,
                       BALLM          =          3,
                       LBALLM         =          3,
                       BBALLM         =          8,
                       APPLEM         =         34,
                       INSPECTORM     =          2,
                       LUNATICM       = (float)  3.14,
                       ROCKETM        =          4;

MODULE void accel(CONST int i, CONST double howmuch);
MODULE void addpoint(CONST int x, CONST int y, CONST int xp, CONST int yp, CONST int color, CONST int time);
MODULE void check_limit(void);
MODULE void collisions(void);
MODULE INLINE int color(CONST int type, CONST int i, CONST int letter);
MODULE void creator_rocket(CONST int i);
MODULE void creators_points(int radius, int x1, int y1, int color1);
MODULE void destroy(CONST int i);
MODULE void explosion(CONST int x, CONST int y, CONST int type, CONST int letter, CONST int n);
MODULE void help(int x, int y, int radius, char* text);
MODULE __inline void Line(int x1, int y1, int x2, int y2, int color);
MODULE void move_objects(void);
MODULE void normalize(float* x, float* y, float size);
MODULE void points(void);
MODULE void process_keys(void);
MODULE void rocket_destroyed(CONST int player);
MODULE void sprocess_keys(void);
MODULE void update_forces(void);
MODULE void update_values(void);

MODULE void addpoint(CONST int x, CONST int y, CONST int xp, CONST int yp, CONST int color, CONST int time)
{   point[npoint].x     = x;
    point[npoint].y     = y;
    point[npoint].xp    = xp;
    point[npoint].yp    = yp;
    point[npoint].time  = time;
    point[npoint].color = color;
    npoint++;
    if (npoint >= maxpoint)
    {   npoint = 0;
}   }

MODULE void points(void)
{   int i;

    for (i = 0; i < maxpoint; i++)
    {   if (point[i].time > 0)
        {   point[i].time--;

            point[i].x += point[i].xp;
            point[i].y += point[i].yp;
            if
            (   point[i].x >= 0 && (point[i].x >> 8) < MAPWIDTH
             && point[i].y >= 0 && (point[i].y >> 8) < MAPHEIGHT
            )
            {   display[point[i].y >> 8][point[i].x >> 8] = pens[point[i].color];
            } else
            {   point[i].time = 0;
}   }   }   }

EXPORT int radius(CONST int type)
{   switch (type)
    {
    case EHOLE:
    case HOLE:
        return HOLE_RADIUS;
    case ROCKET:
        return ROCKET_RADIUS;
    case BALL:
    case LBALL:
        return BALL_RADIUS;
    case BBALL:
        return BBALL_RADIUS;
    case APPLE:
        return APPLE_RADIUS;
    case INSPECTOR:
        return INSPECTOR_RADIUS;
    case LUNATIC:
        return LUNATIC_RADIUS;
    }
    return 0;
}

MODULE INLINE int color(CONST int type, CONST int i, CONST int letter)
{   switch (type)
    {
    case EHOLE:
    case BBALL:
        return OFFSET_GREEN;
    case HOLE:
    case APPLE:
    case BALL:
        return OFFSET_RED;
    case ROCKET:
        return rocketcolor[i];
    case LBALL:
        switch (letter)
        {
        case L_ACCEL:
            return OFFSET_GREEN;
        case L_GUMM:
            return OFFSET_BLUE;
        case L_THIEF:
            return OFFSET_GREY;
        case L_FINDER:
        case L_TTOOL:
            return OFFSET_YELLOW;
        }
    case INSPECTOR:
        return OFFSET_BLUE;
    case LUNATIC:
        return OFFSET_YELLOW;
    }
    return 0;
}

EXPORT float M(CONST int type)
{   switch (type)
    {
    case APPLE:
        return APPLEM;
    case INSPECTOR:
        return INSPECTORM;
    case LUNATIC:
        return LUNATICM;
    case HOLE:
    case EHOLE:
        return BBALLM;
    case ROCKET:
        return ROCKETM;
    case BALL:
    case LBALL:
        return BALLM;
    case BBALL:
        return BBALLM;
    }
    return 0;
}

EXPORT int find_position(float* x, float* y, CONST float radius)
{   int   x1, y1, i, y2 = 0;
    float xp, yp;

rerand:
    x1 = rand() % (GAMEWIDTH  - 60) + 30;
    y1 = rand() % (GAMEHEIGHT - 60) + 30;

    for (i = 0; i < nobjects; i++)
    {   xp = x1 - object[i].x;
        yp = y1 - object[i].y;
        if
        (   (xp * xp) + (yp * yp)
         <  (radius + object[i].radius) * (radius + object[i].radius)
        )
        {   y2++;
            if (y2 > 10000)
                return 0;
            goto rerand;
    }   }

    *x = (float) x1;
    *y = (float) y1;
    return 1;
}

MODULE void normalize(float* x, float* y, float size)
{   float length = (float) (sqrt(((*x) * (*x)) + ((*y) * (*y))));

    if (length == 0)
    {   length = 1;
    }
    *x *= size / length;
    *y *= size / length;
}

MODULE void move_objects(void)
{   int i;

    for (i = 0; i < nobjects; i++)
    {   if (object[i].type == CREATOR)
        {   object[i].time--;
            if (object[i].time <= 0)
            {   play_sound(S_CREATOR2);
                object[i].live = object[i].live1;
                object[i].type = object[i].ctype;
                if (object[i].type == ROCKET)
                {   object[i].time = 200;
                }
                object[i].radius = radius(object[i].ctype);
                object[i].M = M(object[i].ctype);
        }   }
        elif (object[i].live)
        {   object[i].x += (float) (object[i].fx * (GAMEWIDTH / 640.0 + 1) / 2);
            object[i].y += (float) (object[i].fy * (GAMEWIDTH / 640.0 + 1) / 2);
}   }   }

MODULE void help(int x, int y, int radius, char* text)
{   int x1 = x + radius + 2,
        y1 = y - 4,
        x2 = x1 + strlen(text) * 8,
        y2 = y1 + 8;

    if (helpmode && x1 >= 0 && x2 < GAMEWIDTH && y1 >= 0 && y2 < GAMEHEIGHT)
    {   DrawWhiteText(x1, y1, text);
}   }

EXPORT void draw_objects(void)
{   int i;

    clearscreen();
    points();
    help(5, 11, 0, "HELP - PRESS 'H' TO DISABLE");

    for (i = 0; i < nobjects; i++)
    {   if
        (   object[i].live
         && object[i].lineto != -1
         && object[object[i].lineto].live
        )
        {   Line
            (   (int) object[i].x,
                (int) object[i].y,
                (int) object[object[i].lineto].x,
                (int) object[object[i].lineto].y,
                WHITE // white
            );
            help
            (   (int) ((object[i].x + object[object[i].lineto].x) / 2),
                (int) ((object[i].y + object[object[i].lineto].y) / 2),
                2,
                "Spit"
            );
    }   }

    for (i = 0; i < nobjects; i++)
    {   if (object[i].live)
        {   switch (object[i].type)
            {
            case BALL:
                draw_ball
                (   (int) (object[i].x - BALL_RADIUS),
                    (int) (object[i].y - BALL_RADIUS)
                );
                help((int) object[i].x, (int) object[i].y, object[i].radius, "Koules");
            acase LBALL:
                switch (object[i].letter)
                {
                case L_ACCEL:
                    draw_lball
                    (   (int) (object[i].x - BALL_RADIUS),
                        (int) (object[i].y - BALL_RADIUS),
                        0
                    );
                    help((int) object[i].x, (int) object[i].y, object[i].radius, "Acceleration");
                acase L_GUMM:
                    draw_lball
                    (   (int) (object[i].x - BALL_RADIUS),
                        (int) (object[i].y - BALL_RADIUS),
                        1
                    );
                    help((int) object[i].x, (int) object[i].y, object[i].radius, "Weight");
                acase L_THIEF:
                    draw_lball
                    (   (int) (object[i].x - BALL_RADIUS),
                        (int) (object[i].y - BALL_RADIUS),
                        2
                    );
                    help((int) object[i].x, (int) object[i].y, object[i].radius, "Thief");
                acase L_FINDER:
                    draw_lball
                    (   (int) (object[i].x - BALL_RADIUS),
                        (int) (object[i].y - BALL_RADIUS),
                        3
                    );
                    help((int) object[i].x, (int) object[i].y, object[i].radius, "Goodie");
                acase L_TTOOL:
                    draw_lball
                    (   (int) (object[i].x - BALL_RADIUS),
                        (int) (object[i].y - BALL_RADIUS),
                        4
                    );
                    help((int) object[i].x, (int) object[i].y, object[i].radius, "Thief toolkit");
                }
                str[0] = object[i].letter;
                DrawBlackMaskedText ((int) object[i].x - 4, (int) object[i].y - 3, str);
            acase HOLE:
                draw_hole
                (   (int) (object[i].x - HOLE_RADIUS),
                    (int) (object[i].y - HOLE_RADIUS),
                    0
                );
                help((int) object[i].x, (int) object[i].y, object[i].radius, "Black hole");
            acase EHOLE:
                draw_hole
                (   (int) (object[i].x - HOLE_RADIUS),
                    (int) (object[i].y - HOLE_RADIUS),
                    1
                );
                help((int) object[i].x, (int) object[i].y, object[i].radius, "Magnetic hole");
            acase BBALL:
                draw_bball
                (   (int) (object[i].x - BBALL_RADIUS),
                    (int) (object[i].y - BBALL_RADIUS)
                );
                help((int) object[i].x, (int) object[i].y, object[i].radius, "BBALL!");
            acase INSPECTOR:
                draw_inspector
                (   (int) (object[i].x - INSPECTOR_RADIUS),
                    (int) (object[i].y - INSPECTOR_RADIUS)
                );
                help((int) object[i].x, (int) object[i].y, object[i].radius, "Inspector");
            acase LUNATIC:
                draw_lunatic
                (   (int) (object[i].x - LUNATIC_RADIUS),
                    (int) (object[i].y - LUNATIC_RADIUS)
                );
                help((int) object[i].x, (int) object[i].y, object[i].radius, "Lunatic");
            acase APPLE:
                draw_apple
                (   (int) (object[i].x - APPLE_RADIUS),
                    (int) (object[i].y - APPLE_RADIUS)
                );
                draw_eye
                (   (int) (object[i].x - EYE_RADIUS),
                    (int) (object[i].y + APPLE_RADIUS - 15),
                    0
                );
                help((int) object[i].x, (int) object[i].y, object[i].radius, "APPLEPOLISHER");
            acase ROCKET:
                {   int x1, y1;

                    help((int) object[i].x, (int) object[i].y, object[i].radius, "Player");
                    draw_rocket
                    (   (int) (object[i].x - ROCKET_RADIUS),
                        (int) (object[i].y - ROCKET_RADIUS),
                        i
                    );

                    if (!object[i].thief)
                    {   // right eye
                        x1 = (int) (object[i].x + sin(object[i].rotation - RAD(30)) * EYE_RADIUS1 - EYE_RADIUS);
                        y1 = (int) (object[i].y + cos(object[i].rotation - RAD(30)) * EYE_RADIUS1 - EYE_RADIUS);
                        draw_eye
                        (   x1,
                            y1,
                            i
                        );

                        // left eye
                        x1 = (int) (object[i].x + sin(object[i].rotation + RAD(30)) * EYE_RADIUS1 - EYE_RADIUS);
                        y1 = (int) (object[i].y + cos(object[i].rotation + RAD(30)) * EYE_RADIUS1 - EYE_RADIUS);
                        draw_eye
                        (   x1,
                            y1,
                            i
                        );
                    } else
                    {   x1 = (int) (object[i].x + sin(object[i].rotation - RAD(30)) * EYE_RADIUS1 - BALL_RADIUS);
                        y1 = (int) (object[i].y + cos(object[i].rotation - RAD(30)) * EYE_RADIUS1 - BALL_RADIUS);
                        draw_lball
                        (   x1,
                            y1,
                            2
                        );

                        x1 = (int) (object[i].x + sin(object[i].rotation + RAD(30)) * EYE_RADIUS1 - BALL_RADIUS);
                        y1 = (int) (object[i].y + cos(object[i].rotation + RAD(30)) * EYE_RADIUS1 - BALL_RADIUS);
                        draw_lball
                        (   x1,
                            y1,
                            2
                        );
    }   }   }   }   }

    if (gamemode == MENU)
    {   draw_menu();
    }
    if (paused)
    {   DrawWhiteText((MAPWIDTH / 2) - (4 * 6), 140, "PAUSED");
    }
    statusarea();
    updatescreen();
}

MODULE void explosion(CONST int x, CONST int y, CONST int type, CONST int letter, CONST int n)
{   float i;
    int   speed,
          color1,
          radius1 = radius(type);

    for (i = 0; i < RAD(360); i += (float) (RAD(360.0) / radius1 / radius1 / M_PI))
    {   speed = rand() % 3096 + 10;
        color1 = color(type, n, letter) + (rand() % 16);
        addpoint
        (   x * 256,
            y * 256,
            (int) (sin(i) * (speed)),
            (int) (cos(i) * (speed)),
            color1,
            rand() % 100 + 10
        );
}   }

MODULE void rocket_destroyed(CONST int player)
{   int i,
        nalive   = 0,
        igagnant = 0;

    if (gamemode == GAME)
    {   switch (gameplan)
        {
        case DEATHMATCH:
            if (nrockets == 1)
            {   return;
            }
            for (i = 0; i < nrockets; i++)
            {   if (object[i].type == ROCKET && object[i].live && i != player)
                {   object[i].score += 100;
                    nalive++;
                    igagnant = i;
            }   }
            if (nalive == 1)    /* winner bonus */
            {   object[igagnant].score += 50;
}   }   }   }

MODULE void destroy(CONST int i)
{   int y;

    if (object[i].x - object[i].radius < 0)
        object[i].x = (float) (object[i].radius + 1), object[i].fx *= -1;
    if (object[i].y - object[i].radius < 0)
        object[i].y = (float) (object[i].radius + 1), object[i].fy *= -1;
    if (object[i].x + object[i].radius > GAMEWIDTH)
        object[i].x = (float) (GAMEWIDTH - object[i].radius - 1), object[i].fx *= -1;
    if (object[i].y + object[i].radius > GAMEHEIGHT)
       object[i].y = (float) (GAMEHEIGHT - object[i].radius - 1), object[i].fy *= -1;

    switch (object[i].type)
    {
    case LBALL:
        play_sound(S_DESTROY_BALL);
        object[i].live = 0, explosion((int) object[i].x, (int) object[i].y, object[i].type, object[i].letter, i);
        if (object[i].letter == L_THIEF && allow_finder())
        {   object[i].live = 1;
            object[i].letter = L_FINDER;
        } /* else
            if (object[i].letter == L_FINDER)
            {   object[i].live = 1;
                object[i].letter = L_THIEF;
            } */
    acase APPLE:
        play_sound(S_DESTROY_ROCKET);
        object[i].live = 0, explosion((int) object[i].x, (int) object[i].y, object[i].type, object[i].letter, i);
    acase BALL:
    case EHOLE:
    case BBALL:
    case INSPECTOR:
    case LUNATIC:
        play_sound(S_DESTROY_BALL);
        if ((y = create_letter()) != 0)
        {   object[i].type = LBALL;
            object[i].M = LBALLM;
            switch (y)
            {
            case 1:
                object[i].letter = L_ACCEL;
            acase 2:
                object[i].letter = L_GUMM;
            acase 3:
                object[i].letter = L_THIEF;
            acase 4:
                object[i].letter = L_FINDER;
            acase 5:
                object[i].letter = L_TTOOL;
        }   }
        else
        {   object[i].live = 0;
            explosion((int) object[i].x, (int) object[i].y, object[i].type, object[i].letter, i);
        }
    acase ROCKET:
        play_sound(S_DESTROY_ROCKET);
        if (gamemode == GAME)
        {   object[i].live1--;
            object[i].live--;
        }
        explosion((int) object[i].x, (int) object[i].y, object[i].type, object[i].letter, i);
        rocket_destroyed(i);
        if (object[i].live)
        {   /* object[i].x = rand() % (GAMEWIDTH  - 60) + 30;
               object[i].y = rand() % (GAMEHEIGHT - 60) + 30; */
            object[i].fx = 0;
            object[i].fy = 0;
            object[i].rotation = 0;
            object[i].type = ROCKET;
            object[i].accel = ROCKET_SPEED;
            creator_rocket(i);
}   }   }

MODULE void check_limit(void)
{   int i;

    for (i = 0; i < nobjects; i++)
    {   if (object[i].live)
        {   if
            (   object[i].x - object[i].radius <= 0
             || object[i].x + object[i].radius >= GAMEWIDTH
             || object[i].y - object[i].radius <= 0
             || object[i].y + object[i].radius >= GAMEHEIGHT
            )
            {   destroy(i);
}   }   }   }

// count number of creatures
MODULE void update_values(void)
{   int i;

    a_holes      =
    a_rockets    =
    a_balls      =
    a_bballs     =
    a_apples     =
    a_eholes     =
    a_inspectors =
    a_lunatics   = 0;

    for (i = 0; i < nobjects; i++)
    {   if (object[i].live)
        {   switch (object[i].type)
            {
            case HOLE:
                a_holes++;
            acase EHOLE:
                a_eholes++;
            acase ROCKET:
                a_rockets++;
            acase LBALL:
            case BALL:
                a_balls++;
            acase BBALL:
                a_bballs++;
            acase APPLE:
                a_apples++;
            acase INSPECTOR:
                a_inspectors++;
            acase LUNATIC:
                a_lunatics++;
            break;
        }   }

        if (object[i].type == CREATOR)
        {   switch (object[i].ctype)
            {
            case BBALL:
                a_bballs++;
            acase HOLE:
                a_holes++;
            acase EHOLE:
                a_eholes++;
            acase ROCKET:
                a_rockets++;
            acase LBALL:
            case BALL:
                a_balls++;
            acase APPLE:
                a_apples++;
            acase INSPECTOR:
                a_inspectors++;
            acase LUNATIC:
                a_lunatics++;
            break;
}   }   }   }

// accelerate rocket
MODULE void accel(CONST int i, CONST double howmuch)
{   int   y;
    float p;

    /* howmuch is between 0 and 1, everything else is cheating */

    object[i].time = 0;
    object[i].fx += (float) (howmuch * sin(object[i].rotation) * object[i].accel);
    object[i].fy += (float) (howmuch * cos(object[i].rotation) * object[i].accel);

    for (y = 0; y < 5; y++)
    {   p = (float) (RAD(rand() % 45 - 22));
        addpoint
        (   (int) (object[i].x * 256),
            (int) (object[i].y * 256),
            (int) ((object[i].fx - howmuch * sin(object[i].rotation + p) * object[i].accel * 10) * (rand() % 512)),
            (int) ((object[i].fy - howmuch * cos(object[i].rotation + p) * object[i].accel * 10) * (rand() % 512)),
            rocketcolor[i] + (rand() % 16),
            10
        );
}   }

MODULE void sprocess_keys(void)
{   PERSIST int    i;         // all PERSISTent for speed
    PERSIST double dx, dy, a;
#ifdef ANALOG
    PERSIST double x, y; // PERSISTent for speed
#endif

    if (gamemode != GAME)
    {   return;
    }

    for (i = 0; i < MAXROCKETS; i++)
    {   if (object[i].live && object[i].type == ROCKET)
        {   switch (controller[i])
            {
            case RELLTKYBD:
            case RELMDKYBD:
            case RELRTKYBD:
                if (controls[i].mask & 1)
                    object[i].rotation += (float) ROTSTEP;
                if (controls[i].mask & 2)
                    object[i].rotation -= (float) ROTSTEP;
                if (controls[i].mask & 4)
                    accel(i, 1.0);
            acase MOUSE:
                dx = object[i].x - controls[i].mx;
                dy = object[i].y - controls[i].my;
                if (dx == 0)
                    dx = 0.001;
                a = atan(fabs(dy) / fabs(dx));
	            if (dx < 0 && dy >= 0)
                    object[i].rotation = (float) (a + RAD(90));
                elif (dx < 0 && dy < 0)
                    object[i].rotation = (float) (RAD(90) - a);
                elif (dx >= 0 && dy < 0)
                    object[i].rotation = (float) (a + RAD(270));
                elif (dx >= 0 && dy >= 0)
                    object[i].rotation = (float) (RAD(270) - a);
                if (controls[i].mask)
                    accel(i, 1.0);
            break;
#ifdef ANALOG
            case JOY1:
            case JOY2:
            case JOY3:
            case JOY4:
                if   (controller[i] == JOY1) { x = (double) jx[0]; y = (double) jy[0]; }
                elif (controller[i] == JOY2) { x = (double) jx[1]; y = (double) jy[1]; }
                elif (controller[i] == JOY3) { x = (double) jx[2]; y = (double) jy[2]; }
                elif (controller[i] == JOY4) { x = (double) jx[3]; y = (double) jy[3]; }
                // else assert(0);

                a = atan(fabs(y)/fabs(x));
                if (x == 0 && y == 0)
                {   ; // important!
                } elif (x < 0 && y >= 0)
                {   object[i].rotation = (float) (a + RAD(90));
                } elif (x < 0 && y < 0)
                {   object[i].rotation = (float) (RAD(90) - a);
                } elif (x >= 0 && y < 0)
                {   object[i].rotation = (float) (a + RAD(270));
                } elif (x >= 0 && y >= 0)
                {   object[i].rotation = (float) (RAD(270) - a);
                }

                // Measure the deflection (a is betw. 0 and 1)
  	            a = hypot(x * (1.0 / 32767.0), y * (1.0 / 32767.0));
           	    // I must make sure, that I am not cheating :-)
		        // "a" can't be bigger than one
	            if ((a > 1.0) || (controls[i].mask != 0)) a = 1.0;
	            if (a > 0.1) accel(i, a);
	        break;
#endif
            case ABSLTKYBD:
            case ABSMDKYBD:
            case ABSRTKYBD:
#ifndef ANALOG
            case JOY1:
            case JOY2:
            case JOY3:
            case JOY4:
#endif
                switch (controls[i].mask)
                {
                case 1:
                    object[i].rotation = (float) RAD(-135);
                    accel(i, 1.0);
                acase 2:
                    object[i].rotation = (float) RAD(135);
                    accel(i, 1.0);
                acase 3:
                    object[i].rotation = (float) RAD(45);
                    accel(i, 1.0);
                acase 4:
                    object[i].rotation = (float) RAD(-45);
                    accel(i, 1.0);
                acase 5:
                    object[i].rotation = (float) RAD(-90);
                    accel(i, 1.0);
                acase 6:
                    object[i].rotation = (float) RAD(90);
                    accel(i, 1.0);
                acase 7:
                    object[i].rotation = (float) RAD(180);
                    accel(i, 1.0);
                acase 8:
                    object[i].rotation = (float) RAD(0);
                    accel(i, 1.0);
}   }   }   }   }

MODULE void process_keys(void)
{   TRANSIENT int i,
                  s /*  = 0 */ ;
    PERSIST   int lasth = 0;

#ifdef WIN32
    while (iconified)
    {   Sleep(0);
        UpdateInput();
    }
#endif

    UpdateInput();
    if (paused)
    {   return;
    }

    if (ph && !lasth)
    {   helpmode ^= 1;
    }
    lasth = ph;

    checksound();

    switch (gamemode)
    {
    case MENU:
        menu_keys();
    acase GAME:
        if (pesc)
        {   gamemode = MENU;
            unconfine();
            while (pesc)
            {   UpdateInput();
        }   }
        for (i = 0; i < nrockets; i++)
        {   if (object[i].type != ROCKET)
			{    continue;
			}

            s = 0;
            switch (controller[i])
            {
            case RELLTKYBD:
                if (IsPressed(keys[0][2]))
                    s |= 1;
                if (IsPressed(keys[0][3]))
                    s |= 2;
                if (IsPressed(keys[0][0]))
                    s |= 4;
            acase RELMDKYBD:
                if (IsPressed(keys[1][2]))
                    s |= 1;
                if (IsPressed(keys[1][3]))
                    s |= 2;
                if (IsPressed(keys[1][0]))
                    s |= 4;
            acase RELRTKYBD:
                if (IsPressed(keys[2][2]))
                    s |= 1;
                if (IsPressed(keys[2][3]))
                    s |= 2;
                if (IsPressed(keys[2][0]))
                    s |= 4;
            acase ABSLTKYBD:
                if (IsPressed (keys[0][2]) && IsPressed (keys[0][0]))
                    s = 1;
                elif (IsPressed (keys[0][3]) && IsPressed (keys[0][0]))
                    s = 2;
                elif (IsPressed (keys[0][1]) && IsPressed (keys[0][3]))
                    s = 3;
                elif (IsPressed (keys[0][1]) && IsPressed (keys[0][2]))
                    s = 4;
                elif (IsPressed (keys[0][2]))
                    s = 5;
                elif (IsPressed (keys[0][3]))
                    s = 6;
                elif (IsPressed (keys[0][0]))
                    s = 7;
                elif (IsPressed (keys[0][1]))
                    s = 8;
            acase ABSMDKYBD:
                if (IsPressed (keys[1][2]) && IsPressed (keys[1][0]))
                    s = 1;
                elif (IsPressed (keys[1][3]) && IsPressed (keys[1][0]))
                    s = 2;
                elif (IsPressed (keys[1][1]) && IsPressed (keys[1][3]))
                    s = 3;
                elif (IsPressed (keys[1][1]) && IsPressed (keys[1][2]))
                    s = 4;
                elif (IsPressed (keys[1][2]))
                    s = 5;
                elif (IsPressed (keys[1][3]))
                    s = 6;
                elif (IsPressed (keys[1][0]))
                    s = 7;
                elif (IsPressed (keys[1][1]))
                    s = 8;
            acase ABSRTKYBD:
                if (IsPressed (keys[2][2]) && IsPressed (keys[2][0]))
                    s = 1;
                elif (IsPressed (keys[2][3]) && IsPressed (keys[2][0]))
                    s = 2;
                elif (IsPressed (keys[2][1]) && IsPressed (keys[2][3]))
                    s = 3;
                elif (IsPressed (keys[2][1]) && IsPressed (keys[2][2]))
                    s = 4;
                elif (IsPressed (keys[2][2]))
                    s = 5;
                elif (IsPressed (keys[2][3]))
                    s = 6;
                elif (IsPressed (keys[2][0]))
                    s = 7;
                elif (IsPressed (keys[2][1]))
                    s = 8;
            acase JOY1:
                s = ReadJoystick(0);
            acase JOY2:
                s = ReadJoystick(1);
            acase JOY3:
                s = ReadJoystick(2);
            acase JOY4:
                s = ReadJoystick(3);
            acase MOUSE:
                controls[i].mx   = MouseX;
                controls[i].my   = MouseY;
                s                = MouseButtons;
            }
            controls[i].mask = s;
}   }   }

MODULE void creators_points(int radius, int x1, int y1, int color1)
{   int    z, x, y, x2, y2;
    double r;
    int    time = 50;
    int    midX, midY, r2, r1;

    midX = GAMEWIDTH / 2;
    midY = GAMEHEIGHT / 2;
    r2 = r1 = MIN(midX, midY);
    r2 *= r2;

    z = (int) (radius * radius * M_PI);
    while (z--)
    {   do
        {   x = rand() % GAMEWIDTH;
            y = rand() % GAMEHEIGHT;
        } while (((x - midX) * (x - midX) + (y - midY) * (y - midY)) > r2);
        r = sqrt((double) ((x - midX) * (x - midX) + (y - midY) * (y - midY)));
        r = (r * radius / r1) / r * 0.9;
        x2 = (int) (x1 + (x - midX) * r);
        y2 = (int) (y1 + (y - midY) * r);

        addpoint
        (   x * 256,
            y * 256,
            (x2 - x) * 256 / (time),
            (y2 - y) * 256 / (time),
            color1 + (rand() % 16),
            time
        );
}   }

EXPORT void creator(CONST int type)
{   int i,
        color1 = color(type, 0, 0);

    for
    (   i = nrockets;
        i < nobjects && (object[i].live || object[i].type == CREATOR);
        i++
    );
    {   if (i >= MAXOBJECT)
        {   return;
    }   }
    if (!find_position(&object[i].x, &object[i].y, (float) radius(type)))
    {   return;
    }
    if (i >= nobjects)
    {   nobjects = i + 1;
    }
    object[i].live     = 0;
    object[i].live1    = 1;
    object[i].lineto   = -1;
    object[i].ctype    = type;
    object[i].fx       = 0.0;
    object[i].fy       = 0.0;
    object[i].time     = 50;
    object[i].rotation = 0;
    object[i].type     = CREATOR;
    object[i].M        = M(type);
    object[i].radius   = radius(type);
    object[i].accel    = ROCKET_SPEED;
    object[i].letter   = ' ';
    creators_points(object[i].radius, (int) object[i].x, (int) object[i].y, color1);
    play_sound(S_CREATOR1);
}

MODULE void creator_rocket(CONST int i)
{   int type   = ROCKET,
        color1 = color(ROCKET, i, 0);

    if (!find_position(&object[i].x, &object[i].y, (float) radius(type)))
    {   return;
    }
    object[i].live1    = object[i].live;
    object[i].live     = 0;
    object[i].thief    = 0;
    object[i].ctype    = type;
    object[i].lineto   = -1;
    object[i].fx       = 0.0;
    object[i].fy       = 0.0;
    object[i].time     = 50;
    object[i].rotation = 0;
    object[i].type     = CREATOR;
    object[i].M        = ROCKETM;
    object[i].radius   = ROCKET_RADIUS;
    object[i].accel    = ROCKET_SPEED;
    object[i].letter   = ' ';
    creators_points(ROCKET_RADIUS, (int) object[i].x, (int) object[i].y, color1);
}

MODULE void update_forces(void)
{   int   i,
          r,
          frocket /* = 0 */ ;
    float d,
          xp,
          yp;

    for (i = 0; i < nobjects; i++)
    {   if (object[i].live)
        {   if (object[i].lineto != -1)
            {   if (!object[object[i].lineto].live)
                {   object[i].lineto = -1;
                } elif (object[i].lineto == i)
                {   object[i].lineto = -1;
                } else
                {   int force;

                    xp = object[i].x - object[object[i].lineto].x;
                    yp = object[i].y - object[object[i].lineto].y;
                    force = (int) (sqrt(xp * xp + yp * yp));
                    if (force >= 2 * SPRINGSIZE || gameplan == COOPERATIVE)
                    {   force = force - SPRINGSIZE;
                        if (force < 0)
                        {   force *= 3;
                        }
                        force /= SPRINGSTRENGTH;
                        normalize(&xp, &yp, (float) (force * BALL_SPEED / object[i].M));
                        object[i].fx -= xp;
                        object[i].fy -= yp;
                        normalize(&xp, &yp, (float) (force * BALL_SPEED / object[object[i].lineto].M));
                        object[object[i].lineto].fx += xp;
                        object[object[i].lineto].fy += yp;
            }   }   }
            if (object[i].type == ROCKET && object[i].time)
            {   object[i].time--;
            }
            if (object[i].type == ROCKET && !object[i].time)
            {   // d = 640 * 640;
                // frocket = -1;
                for (r = 0; r < nobjects; r++)
                {   if (object[r].live && !object[r].time && object[r].type == EHOLE)
                    {   int   distance;
                        float gravity;

                        xp = object[r].x - object[i].x;
                        yp = object[r].y - object[i].y;
                        distance = (int) (sqrt(xp * xp + yp * yp));
                        gravity = BALL_SPEED * (gameplan == COOPERATIVE ? 200 : 50) / distance;
                        if (gravity > BALL_SPEED * 4 / 5)
                        {   gravity = BALL_SPEED * 4 / 5;
                        }
                        normalize(&xp, &yp, gravity);
                        object[i].fx += xp;
                        object[i].fy += yp;
            }   }   }

            if (object[i].type == BALL || object[i].type == LBALL || object[i].type == BBALL || object[i].type == LUNATIC)
            {   frocket = -1;
                d = (float) (640 * 640);
                for (r = 0; r < nrockets; r++)
                {   if (object[r].live && !object[r].time)
                    {   xp = object[r].x - object[i].x;
                        yp = object[r].y - object[i].y;
                        if (xp * xp + yp * yp < d)
                        {   d = xp * xp + yp * yp;
                            frocket = r;
                }   }   }
                if (frocket != -1)
                {   xp = object[frocket].x - object[i].x;
                    yp = object[frocket].y - object[i].y;
                } else
                {   xp = GAMEWIDTH  / 2 - object[i].x;
                    yp = GAMEHEIGHT / 2 - object[i].y;
                }
                if (object[i].type == LUNATIC && !rand() % 4)
                {   xp = (float)  rand();
                    yp = (float) (rand() + 1);
                }
                switch (object[i].type)
                {
                case BBALL:
                    normalize(&xp, &yp, BBALL_SPEED);
                acase BALL:
                case LUNATIC:
                case LBALL:
                    normalize(&xp, &yp, BALL_SPEED);
                }
                object[i].fx += xp;
                object[i].fy += yp;
            }
            object[i].fx *= SLOWDOWN,
            object[i].fy *= SLOWDOWN;
}   }   }

MODULE void collisions(void)
{   TRANSIENT int   i,
                    y,
                    colize = 0;
    TRANSIENT float distance,
                    xp,
                    yp,
                    gummfactor;
    PERSIST   int   ctime  = 0;

    for (i = 0; i < nobjects; i++)
    {   if (object[i].live)
        {   for (y = i + 1; y < nobjects; y++)
            {   if (object[y].live)
                {   xp = object[y].x - object[i].x; // how much to the east  the target is compared to source
                    yp = object[y].y - object[i].y; // how much to the south the target is compared to source
                    // StormC doesn't calculate xp and yp correctly!

                    distance = (float) (sqrt((xp * xp) + (yp * yp)));
                    if
                    (   distance
                     <  (float) (object[y].radius + object[i].radius)
                    )
                    {   colize = 1;
                        if (object[i].type == HOLE || object[i].type == EHOLE)
                        {   if (object[y].type != APPLE)
                                destroy(y);
                            if (object[i].type == EHOLE)
                                destroy(i);
                            continue;
                        }
                        if (object[y].type == HOLE || object[y].type == EHOLE)
                        {   if (object[i].type != APPLE)
                                destroy(i);
                            if (object[y].type == EHOLE)
                                destroy(y);
                            continue;
                        }
                        if (object[i].type == ROCKET)
                        {   if (object[y].thief == 1 && object[i].thief == 1)
                            {   float tmp;

                                tmp = object[i].M;
                                object[i].M = object[y].M;
                                object[y].M = tmp;
                                object[i].thief = 0;
                                object[y].thief = 0;
                            }
                            if (object[y].type == BBALL && object[i].thief == 1)
                            {   object[i].M += object[y].M - M(BALL);
                                object[i].thief = 0;
                                object[y].M = M(BALL);
                            } elif (object[y].type == ROCKET && object[i].thief == 1)
                            {   object[i].M += object[y].M - M(ROCKET);
                                object[i].accel += object[y].accel - ROCKET_SPEED;
                                object[i].thief = 0;
                                object[y].M = M(object[i].type);
                                object[y].accel = (float) (ROCKET_SPEED - A_ADD);
                            }
                            if (object[i].type == ROCKET && object[y].thief == 1)
                            {   object[y].M += object[i].M - M(ROCKET);
                                object[y].accel += object[i].accel - ROCKET_SPEED;
                                object[y].thief = 0;
                                object[i].M = M(object[y].type);
                                object[i].accel = (float) (ROCKET_SPEED - A_ADD);
                            }
                            if (gamemode == GAME && gameplan == COOPERATIVE)
                                object[i].score++;
                            if (object[y].letter == L_ACCEL)
                            {   object[i].accel += (float) A_ADD;
                                if (gamemode == GAME) object[i].score += 10;
                            }
                            if (object[y].letter == L_GUMM)
                            {   object[i].M += (float) M_ADD;
                                if (gamemode == GAME) object[i].score += 10;
                            }
                            if (object[y].letter == L_THIEF)
                            {   object[i].M = M(object[i].type);
                                object[i].accel = (float) (ROCKET_SPEED - A_ADD);
                                if (gamemode == GAME) object[i].score -= 30;
                            }
                            if (object[y].letter == L_FINDER)
                            {   object[i].accel += (float) (A_ADD * (rand() % 5));
                                object[i].M += (float) (M_ADD * (rand() % 10));
                                if (gamemode == GAME) object[i].score += 30;
                            }
                            if (object[y].letter == L_TTOOL)
                            {   object[i].thief = 1;
                                if (gamemode == GAME) object[i].score += 30;
                            }

                            object[y].letter = ' ';
                            if (object[y].type == LBALL)
                                object[y].type = BALL;
                            if (object[y].type == BALL && dosprings && !(rand() % randsprings))
                                object[y].lineto = i;

                            if
                            (   gameplan == DEATHMATCH
                             && object[y].type == ROCKET
                             && dosprings
                             && !(rand() % (2 * randsprings))
                            )
                            {   object[y].lineto = i;
                        }   }
                        if (object[y].type == LUNATIC)
                        {   gummfactor = -ROCKETM / LUNATICM;
                        } elif (object[i].type == LUNATIC)
                        {   gummfactor = -LUNATICM / ROCKETM;
                        } else
                        {   gummfactor = object[i].M / object[y].M;
                        }
                        normalize(&xp, &yp, (float) (gummfactor * GUMM));
                        object[y].fx += xp;
                        object[y].fy += yp;
                        normalize(&xp, &yp, (float) (1 / gummfactor * GUMM));
                        object[i].fx -= xp;
                        object[i].fy -= yp;
                        if (object[i].type == ROCKET && object[i].time)
                        {   object[i].fx = 0;
                            object[i].fy = 0;
                        }
                        if (object[y].type == ROCKET && object[y].time)
                        {   object[y].fx = 0;
                            object[y].fy = 0;
                        }
                        if (object[y].type == INSPECTOR && object[i].type == ROCKET)
                        {   object[y].fx = 0;
                            object[y].fy = 0;
                            object[i].fx *= -2;
                            object[i].fy *= -2;
    }   }   }   }   }   }

    if (colize && !ctime)
    {   play_sound(S_COLIZE);
        ctime = 4;
        updatescreen();
    }
    if (ctime)
    {   ctime--;
}   }

#ifdef AMIGA
    #define tv_usec tv_micro
#endif

EXPORT void game(void)
{   init_menu();

    for (;;)
    {   process_keys();
        if (!paused)
        {   sprocess_keys();
            update_values(); // count number of creatures
            update_game();
            update_forces();
            collisions();
            move_objects();
            check_limit();
            draw_objects();
        }
        usleep(40000);
}   }

EXPORT void clearpoints(void)
{   int i;

    for (i = 0; i < maxpoint; i++)
    {   point[i].time = 0;
}   }

MODULE __inline void Line(int x1, int y1, int x2, int y2, int color)
{   int    dx,      // deltas
           dy,
           dx2,     // scaled deltas
           dy2,
           ix,      // increase rate on the x axis
           iy,      // increase rate on the y axis
           err,     // the error term
           i;       // looping variable

#ifdef AMIGA
    UBYTE* ptr_vid = &display[y1][x1]; // identify the first pixel
#endif
#ifdef WIN32
    ULONG* ptr_vid = &display[y1][x1]; // identify the first pixel
#endif

    // difference between starting and ending points
    dx = x2 - x1;
    dy = y2 - y1;

    // calculate direction of the vector and store in ix and iy
    if (dx >= 0)
    {   ix =  1;
    } elif (dx < 0)
    {   dx = -dx;
        ix = -1;
    } else
    {   ix = 0;
    }

    if (dy >= 0)
    {   iy =  WINWIDTH;
    } elif (dy < 0)
    {   dy = -dy;
        iy = -WINWIDTH;
    } else
    {   iy = 0;
    }

    // scale deltas and store in dx2 and dy2
    dx2 = dx * 2;
    dy2 = dy * 2;

    if (dx > dy) // dx is the major axis
    {   // initialize the error term
        err = dy2 - dx;

        for (i = 0; i <= dx; i++)
        {   *ptr_vid = pens[color];
            if (err >= 0)
            {   err -= dx2;
                ptr_vid += iy;
            }
            err += dy2;
            ptr_vid += ix;
    }   }
    else // dy is the major axis
    {   // initialize the error term
        err = dx2 - dy;

        for (i = 0; i <= dy; i++)
        {   *ptr_vid = pens[color];
            if (err >= 0)
            {   err -= dy2;
                ptr_vid += ix;
            }
            err += dx2;
            ptr_vid += iy;
}   }   }

EXPORT void statusarea(void)
{   char s[10 + 1];
    int  i,
         remainder;

    if (gameplan == COOPERATIVE && gamemode == GAME)
    {   sprintf(s, "Level: %3i", level + 1);
        DrawWhiteText(50, MAPHEIGHT + 22, s);
    }
    DrawWhiteText(180, MAPHEIGHT +  7, "Lives:");
    DrawWhiteText(180, MAPHEIGHT + 17, "Score:");
    DrawWhiteText(180, MAPHEIGHT + 27, "Accel.:");
    DrawWhiteText(180, MAPHEIGHT + 37, "Mass:");
    for (i = 0; i < 5; i++)
    {   sprintf(s, "%7i", nrockets > i ? object[i].live1 : 0);
        DrawColouredText(244 + (i * 64), MAPHEIGHT +  7, s, i);
        sprintf(s, "%7i", object[i].score);
        DrawColouredText(244 + (i * 64), MAPHEIGHT + 17, s, i);
        remainder =  (int) (object[i].accel * 10.0);
        remainder %= 10;
        sprintf(s, "%5d.%1d", (int) object[i].accel, remainder);
        DrawColouredText(244 + (i * 64), MAPHEIGHT + 27, s, i);
        remainder =  (int) (object[i].accel * 10.0);
        remainder %= 10;
        sprintf(s, "%5d.%1d", (int) object[i].M, remainder);
        DrawColouredText(244 + (i * 64), MAPHEIGHT + 37, s, i);
}   }

EXPORT void DrawWhiteText(int x, int y, char* text)
{   fgc = pens[BLACK];
    gl_write(x + 1, y + 1, text);
    fgc = pens[WHITE];
    gl_write(x    , y    , text);
}
EXPORT void DrawRedText(int x, int y, char* text)
{   fgc = pens[BLACK];
    gl_write(x + 1, y + 1, text);
    fgc = pens[RED];
    gl_write(x, y, text);
}
EXPORT void DrawColouredText(int x, int y, char* text, int player)
{   PERSIST const int solidbody[MAXROCKETS] = { FLAT_YELLOW, // $C0C000
                                                FLAT_BLUE  , // $4040E0
                                                FLAT_RED   , // $C00000
                                                FLAT_PURPLE, // $C000C0
                                                FLAT_GREEN , // $00C000
                                              };

    fgc = pens[BLACK];
    gl_write(x + 1, y + 1, text);
    fgc = pens[solidbody[player]];
    gl_write(x    , y    , text);
}
EXPORT void DrawBlackMaskedText(int x, int y, char* text)
{   fgc = pens[BLACK];
    gl_write(x, y, text);
}
