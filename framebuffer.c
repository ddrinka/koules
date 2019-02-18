#include "koules.h"

#include <stdlib.h> // abs(), rand()
#include <math.h>

#ifdef AMIGA
    #define NOTHING 0 // TRANSPARENT is already used, under Windows at least
#endif
#ifdef WIN32
    #define NOTHING 0xFFFFFFFF
#endif

EXPORT UBYTE amigan[16][16 + 1] =
{ "................",
  "..........WW....",
  "..........WR....",
  ".........WRA....",
  ".........WRA....",
  "........WR.AA...",
  "........WR.AA...",
  ".......WR..AA...",
  ".BB....WWWWWW...",
  ".BB...YYW...AA..",
  "..CC..YY.W..AA..",
  "...GGYY...W.AA..",
  "...GGYY....W.AA.",
  "....YY......WAA.",
  "....YY.......AA.",
  "................",
};

IMPORT int   gamemode,
             gameplan,
             level,
             stars;
#ifdef AMIGA
IMPORT UBYTE starfield[MAPHEIGHT][WINWIDTH];
#endif
#ifdef WIN32
IMPORT ULONG starfield[MAPHEIGHT][WINWIDTH];
#endif

/* Be sure not to use any high-ASCII characters, as they will lead to accesses of
nonexistent array elements. */
MODULE const unsigned char thefont[1024] =
{   0,0,0,0,0,0,0,0,                     //   0
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,                     //  10
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,                     //  20
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,                     //  30
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,                     //  32 ' '
    56,56,56,56,56,0,56,56,              //  33 '!'
    108,108,0,0,0,0,0,0,                 //  34 '"'
    0,108,254,254,108,254,254,108,       //  35 '#'
    48,126,224,124,14,254,252,48,        //  36 '$'
    98,230,204,24,48,102,206,140,        //  37 '%'
    120,220,252,120,250,222,252,118,     //  38 '&'
    28,28,56,0,0,0,0,0,                  //  39 '''
    14,28,28,28,28,28,28,14,             //  40 '('
    112,56,56,56,56,56,56,112,           //  41 ')'
    0,0x5A,0x3C,0xE7,0xE7,0x3C,0x5A,0,   //  42 '*'
/*
.#.##.#.
..####..
###..###
###..###
..####..
.#.##.#.
*/
    0,0,28,28,127,127,28,28,             //  43 '+'
    0,0,0,0,0,28,28,56,                  //  44 ','
    0,0,0,0,124,124,0,0,                 //  45 '-'
    0,0,0,0,0,0,56,56,                   //  46 '.'
    28,28,56,56,112,112,224,224,         //  47 '/'
    124,254,238,238,238,254,254,124,     //  48 '0'
    56,120,248,56,56,254,254,254,        //  49 '1'
    252,254,14,60,112,254,254,254,       //  50 '2'
    252,254,14,60,14,254,254,252,        //  51 '3'
    238,238,238,254,254,14,14,14,        //  52 '4'
    254,254,224,252,14,254,254,252,      //  53 '5'
    124,252,224,252,238,254,254,124,     //  54 '6'
    252,254,14,14,28,28,56,56,           //  55 '7'
    124,254,238,124,238,254,254,124,     //  56 '8'
    124,254,238,126,14,254,254,252,      //  57 '9'
    0,0,28,28,0,28,28,28,                //  58 ':'
    0,0,28,28,0,28,28,56,                //  59 ';'
    6,14,28,56,56,28,14,6,               //  60 '<'
    0,0,124,124,0,124,124,124,           //  61 '='
    112,56,28,14,14,28,56,112,           //  62 '>'
    124,254,206,28,56,0,56,56,           //  63 '?'
    124,198,190,182,190,182,200,126,     //  64 '@'
    124,254,238,254,238,238,238,238,     //  65 'A'
    252,254,206,252,206,254,254,252,     //  66 'B'
    124,254,238,224,238,254,254,124,     //  67 'C'
    252,254,238,238,238,254,254,252,     //  68 'D'
    254,254,224,248,224,254,254,254,     //  69 'E'
    126,254,224,248,224,224,224,224,     //  70 'F'
    126,254,224,238,238,254,254,124,     //  71 'G'
    238,238,238,254,238,238,238,238,     //  72 'H'
    254,254,56,56,56,254,254,254,        //  73 'I'
    254,254,14,14,238,254,254,124,       //  74 'J'
    238,238,252,248,252,238,238,238,     //  75 'K'
    224,224,224,224,224,254,254,126,     //  76 'L'
    130,198,238,254,254,238,238,238,     //  77 'M'
    206,238,254,254,254,254,238,230,     //  78 'N'
    124,254,238,238,238,254,254,124,     //  79 'O'
    252,254,238,238,252,224,224,224,     //  80 'P'
    124,254,238,238,254,254,252,118,     //  81 'Q'
    252,254,238,238,252,238,238,238,     //  82 'R'
    126,254,224,124,14,254,254,252,      //  83 'S'
    254,254,56,56,56,56,56,56,           //  84 'T'
    238,238,238,238,238,254,254,124,     //  85 'U'
    238,238,238,238,238,238,124,56,      //  86 'V'
    238,238,238,254,254,238,198,130,     //  87 'W'
    238,238,124,56,124,238,238,238,      //  88 'X'
    238,238,124,124,56,56,112,112,       //  89 'Y'
    254,254,28,56,112,254,254,254,       //  90 'Z'
    124,124,112,112,112,124,124,124,     //  91 '['
    112,112,56,56,28,28,14,14,           //  92 '\'
    124,124,28,28,28,124,124,124,        //  93 ']'
    56,124,238,198,0,0,0,0,              //  94 '^'
    0,0,0,0,0,254,254,254,               //  95 '_'
    56,56,28,0,0,0,0,0,                  //  96 '`'
    0,124,254,238,254,238,238,238,       //  97 'a'
    0,252,254,206,252,206,254,252,       //  98 'b'
    0,124,254,238,224,238,254,124,       //  99 'c'
    0,252,254,238,238,238,254,252,       // 100 'd'
    0,254,254,224,248,224,254,254,       // 101 'e'
    0,126,254,224,248,224,224,224,       // 102 'f'
    0,126,254,224,238,238,254,124,       // 103 'g'
    0,238,238,238,254,238,238,238,       // 104 'h'
    0,254,254,56,56,56,254,254,          // 105 'i'
    0,254,254,14,14,238,254,124,         // 106 'j'
    0,238,238,252,248,252,238,238,       // 107 'k'
    0,224,224,224,224,224,254,126,       // 108 'l'
    0,130,198,238,254,254,238,238,       // 109 'm'
    0,206,238,254,254,254,238,230,       // 110 'n'
    0,124,254,238,238,238,254,124,       // 111 'o'
    0,252,254,238,238,252,224,224,       // 112 'p'
    0,124,254,238,238,254,252,118,       // 113 'q'
    0,252,254,238,238,252,238,238,       // 114 'r'
    0,126,254,224,124,14,254,252,        // 115 's'
    0,254,254,56,56,56,56,56,            // 116 't'
    0,238,238,238,238,238,254,124,       // 117 'u'
    0,238,238,238,238,238,124,56,        // 118 'v'
    0,238,238,238,254,238,198,130,       // 119 'w'
    0,238,238,124,56,124,238,238,        // 120 'x'
    0,238,238,124,124,56,56,112,         // 121 'y'
    0,254,254,28,56,112,254,254,         // 122 'z'
    60,124,112,112,112,124,124,60,       // 123 '{'
    56,56,56,0,56,56,56,56,              // 124 '|'
    120,124,28,28,28,124,124,120,        // 125 '}'
    236,254,118,0,0,0,0,0,               // 126 '~'
    0,16,56,124,254,254,254,254          // 127 triangle
};

IMPORT       UBYTE rocketcolor[MAXROCKETS];
#ifdef AMIGA
IMPORT       UBYTE display[WINHEIGHT][WINWIDTH],
                   fgc,
                   pens[256];
#endif
#ifdef WIN32
IMPORT       ULONG display[WINHEIGHT][WINWIDTH],
                   fgc;
IMPORT const ULONG pens[256];
#endif

#ifdef AMIGA
MODULE UBYTE      hole_bitmap[2]         [     HOLE_DIAMETER][     HOLE_DIAMETER],
                   eye_bitmap[MAXROCKETS][      EYE_DIAMETER][      EYE_DIAMETER],
                  ball_bitmap            [     BALL_DIAMETER][     BALL_DIAMETER],
                 bball_bitmap            [    BBALL_DIAMETER][    BBALL_DIAMETER],
             inspector_bitmap            [INSPECTOR_DIAMETER][INSPECTOR_DIAMETER],
                 lball_bitmap[NLETTERS]  [     BALL_DIAMETER][     BALL_DIAMETER],
                rocket_bitmap[MAXROCKETS][   ROCKET_DIAMETER][   ROCKET_DIAMETER],
                 apple_bitmap            [    APPLE_DIAMETER][    APPLE_DIAMETER],
               lunatic_bitmap            [  LUNATIC_DIAMETER][  LUNATIC_DIAMETER];
#endif
#ifdef WIN32
MODULE ULONG      hole_bitmap[2]         [     HOLE_DIAMETER][     HOLE_DIAMETER],
                   eye_bitmap[MAXROCKETS][      EYE_DIAMETER][      EYE_DIAMETER],
                  ball_bitmap            [     BALL_DIAMETER][     BALL_DIAMETER],
                 bball_bitmap            [    BBALL_DIAMETER][    BBALL_DIAMETER],
             inspector_bitmap            [INSPECTOR_DIAMETER][INSPECTOR_DIAMETER],
                 lball_bitmap[NLETTERS]  [     BALL_DIAMETER][     BALL_DIAMETER],
                rocket_bitmap[MAXROCKETS][   ROCKET_DIAMETER][   ROCKET_DIAMETER],
                 apple_bitmap            [    APPLE_DIAMETER][    APPLE_DIAMETER],
               lunatic_bitmap            [  LUNATIC_DIAMETER][  LUNATIC_DIAMETER];
#endif

EXPORT void create_bitmaps(void)
{   int   x, y, r, po, radius;
#ifdef AMIGA
    UBYTE color = 0; // initialized to avoid a spurious SAS/C warning
#endif
#ifdef WIN32
    UBYTE red, green, blue;
#endif

    for (x = 0; x < HOLE_DIAMETER; x++)
    {   for (y = 0; y < HOLE_DIAMETER; y++)
        {   radius = HOLE_RADIUS / 2 + (int) (atan(fabs(x - HOLE_RADIUS + 0.5) / fabs(y - HOLE_RADIUS + 0.5)) * HOLE_RADIUS / 2) % (HOLE_RADIUS / 2);
            if
            (   (x - HOLE_RADIUS)
              * (x - HOLE_RADIUS)
              + (y - HOLE_RADIUS)
              * (y - HOLE_RADIUS)
              < radius * radius
            )
            {   r = (x - HOLE_RADIUS)
                  * (x - HOLE_RADIUS)
                  + (y - HOLE_RADIUS)
                  * (y - HOLE_RADIUS);
#ifdef AMIGA
                r = r * 24 / (radius * radius);
                if (r > 23)
                {   r = 23;
                }
                hole_bitmap[0][x][y] = (UBYTE) (OFFSET_RED   + r + 1);
                hole_bitmap[1][x][y] = (UBYTE) (OFFSET_GREEN + r + 1);
#endif
#ifdef WIN32
                r = 8 + (r * 192 / (radius * radius));
                if (r > 0xF8) { red   = 0; } else { red   = 0xF8 - r; } if (r > 0xB0 / 3) { green = blue = 0; } else { green = blue = 0xB0 - (r * 3); } // red
                hole_bitmap[0][x][y] = (red << 16) | (green << 8) | blue;
                if (r > 0xF8) { green = 0; } else { green = 0xF8 - r; } if (r > 0xE0 / 4) { red   = blue = 0; } else { red   = blue = 0xE0 - (r * 4); } // green
                hole_bitmap[1][x][y] = (red << 16) | (green << 8) | blue;
#endif
            } else
            {   hole_bitmap[0][x][y] =
                hole_bitmap[1][x][y] = NOTHING;
    }   }   }

    for (po = 0; po < MAXROCKETS; po++)
    {
#ifdef AMIGA
        color = (UBYTE) (32 + (32 * po));
#endif
        for (y = 0; y < EYE_DIAMETER; y++)
        {   for (x = 0; x < EYE_DIAMETER; x++)
            {   if
                (   (x - EYE_RADIUS) * (x - EYE_RADIUS) + (y - EYE_RADIUS) * (y - EYE_RADIUS)
                  < (EYE_RADIUS - 0.5) * (EYE_RADIUS - 0.5)
                )
                {   r = (x - 3 * EYE_RADIUS / 4)
                      * (x - 3 * EYE_RADIUS / 4)
                      + (y -     EYE_RADIUS / 4)
                      * (y -     EYE_RADIUS / 4);
#ifdef AMIGA
                    r = (int) (r * 32 / (1.5 * EYE_RADIUS) / (1.5 * EYE_RADIUS));
                    if (r > 31)
                    {   r = 31;
                    }
                    eye_bitmap[po][x][y] = color + r;
#endif
#ifdef WIN32
                    r = (int) (r * 256 / (1.5 * EYE_RADIUS) / (1.5 * EYE_RADIUS));
                    switch (po)
                    {
                    case  0: if (r > 0xF8) { red =         blue  = 0; } else { red =         blue = 0xF8 - r; } if (r > 0xE0 / 4) {       green =        0; } else {       green =        0xE0 - (r * 4); } // purple
                    acase 1: if (r > 0xF8) { red =                 0; } else { red =                0xF8 - r; } if (r > 0xB0 / 3) {       green = blue = 0; } else {       green = blue = 0xB0 - (r * 3); } // red
                    acase 2: if (r > 0xF8) { red = green =         0; } else { red = green =        0xF8 - r; } if (r > 0xE0 / 4) {               blue = 0; } else {               blue = 0xE0 - (r * 4); } // yellow
                    acase 3: if (r > 0xF8) {       green =         0; } else {       green =        0xF8 - r; } if (r > 0xE0 / 4) { red =         blue = 0; } else { red =         blue = 0xE0 - (r * 4); } // green
                    acase 4: if (r > 0xF8) {               blue  = 0; } else {               blue = 0xF8 - r; } if (r > 0x70 / 2) { red = green =        0; } else { red = green =        0x70 - (r * 2); } // blue
                    }
                    eye_bitmap[po][x][y] = (red << 16) | (green << 8) | blue;
#endif
                } else
                {   eye_bitmap[po][x][y] = NOTHING;
        }   }   }

#ifdef AMIGA
        color = rocketcolor[po];
#endif
        for (y = 0; y < ROCKET_DIAMETER; y++)
        {   for (x = 0; x < ROCKET_DIAMETER; x++)
            {   if
                (   (x - ROCKET_RADIUS) * (x - ROCKET_RADIUS) + (y - ROCKET_RADIUS) * (y - ROCKET_RADIUS)
                  < (ROCKET_RADIUS - 0.5) * (ROCKET_RADIUS - 0.5)
                )
                {   r = (x - 3 * ROCKET_RADIUS / 4)
                      * (x - 3 * ROCKET_RADIUS / 4)
                      + (y -     ROCKET_RADIUS / 4)
                      * (y -     ROCKET_RADIUS / 4);
#ifdef AMIGA
                    r = (int) (r * 32 / (1.5 * ROCKET_RADIUS) / (1.5 * ROCKET_RADIUS));
                    if (r > 31)
                    {   r = 31;
                    }
                    rocket_bitmap[po][x][y] = color + r;
#endif
#ifdef WIN32
                    r = (int) (r * 256 / (1.5 * ROCKET_RADIUS) / (1.5 * ROCKET_RADIUS));
                    switch (po)
                    {
                    case  0: if (r > 0xF8) { red = green =         0; } else { red = green =        0xF8 - r; } if (r > 0xE0 / 4) {               blue = 0; } else {               blue = 0xE0 - (r * 4); } // yellow
                    acase 1: if (r > 0xF8) {               blue  = 0; } else {               blue = 0xF8 - r; } if (r > 0x70 / 2) { red = green =        0; } else { red = green =        0x70 - (r * 2); } // blue
                    acase 2: if (r > 0xF8) { red =                 0; } else { red =                0xF8 - r; } if (r > 0xB0 / 3) {       green = blue = 0; } else {       green = blue = 0xB0 - (r * 3); } // red
                    acase 3: if (r > 0xF8) { red =         blue  = 0; } else { red =         blue = 0xF8 - r; } if (r > 0xE0 / 4) {       green =        0; } else {       green =        0xE0 - (r * 4); } // purple
                    acase 4: if (r > 0xF8) {       green =         0; } else {       green =        0xF8 - r; } if (r > 0xE0 / 4) { red =         blue = 0; } else { red =         blue = 0xE0 - (r * 4); } // green
                    }
                    rocket_bitmap[po][x][y] = (red << 16) | (green << 8) | blue;
#endif
                } else
                {   rocket_bitmap[po][x][y] = NOTHING;
    }   }   }   }

    for (y = 0; y < BALL_DIAMETER; y++)
    {   for (x = 0; x < BALL_DIAMETER; x++)
        {   if
            (   (x - BALL_RADIUS) * (x - BALL_RADIUS) + (y - BALL_RADIUS) * (y - BALL_RADIUS)
              < (BALL_RADIUS - 0.5) * (BALL_RADIUS - 0.5)
            )
            {   r = (x - 3 * BALL_RADIUS / 4)
                  * (x - 3 * BALL_RADIUS / 4)
                  + (y -     BALL_RADIUS / 4)
                  * (y -     BALL_RADIUS / 4);

#ifdef AMIGA
                r = (int) (r * 32 / (1.5 * BALL_RADIUS) / (1.5 * BALL_RADIUS));
                if (r > 31)
                {   r = 31;
                }
                ball_bitmap[x][y] = OFFSET_RED + r;
#endif
#ifdef WIN32
                r = (int) (r * 256 / (1.5 * BALL_RADIUS) / (1.5 * BALL_RADIUS));
                if (r > 0xF8)
                {   red = 0;
                } else
                {   red = 0xF8 - r;
                }
                if (r > 0xB0 / 3)
                {   green = blue = 0;
                } else
                {   green = blue = 0xB0 - (r * 3);
                }
                ball_bitmap[x][y] = (red << 16) | (green << 8) | blue;
#endif
            } else
            {   ball_bitmap[x][y] = NOTHING;
    }   }   }

    for (y = 0; y < BBALL_DIAMETER; y++)
    {   for (x = 0; x < BBALL_DIAMETER; x++)
        {   if
            (   (x - BBALL_RADIUS) * (x - BBALL_RADIUS) + (y - BBALL_RADIUS) * (y - BBALL_RADIUS)
              < (BBALL_RADIUS - 0.5) * (BBALL_RADIUS - 0.5)
            )
            {   r = (x - 3 * BBALL_RADIUS / 4)
                  * (x - 3 * BBALL_RADIUS / 4)
                  + (y -     BBALL_RADIUS / 4)
                  * (y -     BBALL_RADIUS / 4);
#ifdef AMIGA
                r = (int) (r * 32 / (1.5 * BBALL_RADIUS) / (1.5 * BBALL_RADIUS));
                if (r > 31)
                {   r = 31;
                }
                bball_bitmap[x][y] = OFFSET_GREEN + r;
#endif
#ifdef WIN32
                r = (int) (r * 256 / (1.5 * BBALL_RADIUS) / (1.5 * BBALL_RADIUS));
                green = (r > 0xF8) ? 0 : (0xF8 - r); red = blue = (r > 0xE0 / 4) ? 0 : (0xE0 - (r * 4));
                bball_bitmap[x][y] = (red << 16) | (green << 8) | blue;
#endif
            } else
            {   bball_bitmap[x][y] = NOTHING;
    }   }   }

    for (y = 0; y < INSPECTOR_DIAMETER; y++)
    {   for (x = 0; x < INSPECTOR_DIAMETER; x++)
        {   if
            (   (x - INSPECTOR_RADIUS) * (x - INSPECTOR_RADIUS) + (y - INSPECTOR_RADIUS) * (y - INSPECTOR_RADIUS)
              < (INSPECTOR_RADIUS - 0.5) * (INSPECTOR_RADIUS - 0.5)
            )
            {   r = (x - 3 * INSPECTOR_RADIUS / 4)
                  * (x - 3 * INSPECTOR_RADIUS / 4)
                  + (y -     INSPECTOR_RADIUS / 4)
                  * (y -     INSPECTOR_RADIUS / 4);
#ifdef AMIGA
                r = (int) (r * 32 / (1.5 * INSPECTOR_RADIUS) / (1.5 * INSPECTOR_RADIUS));
                if (r > 31)
                {   r = 31;
                }
                inspector_bitmap[x][y] = OFFSET_BLUE + r;
#endif
#ifdef WIN32
                r = (int) (r * 256 / (1.5 * INSPECTOR_RADIUS) / (1.5 * INSPECTOR_RADIUS));
                blue = (r > 0xF8) ? 0 : (0xF8 - r); red = green = (r > 0x70 / 2) ? 0 : (0x70 - (r * 2));
                inspector_bitmap[x][y] = (red << 16) | (green << 8) | blue;
#endif
            } else
            {   inspector_bitmap[x][y] = NOTHING;
    }   }   }

    for (po = 0; po < NLETTERS; po++)
    {
#ifdef AMIGA
        switch (po)
        {
        case  0: color = OFFSET_GREEN;
        acase 1: color = OFFSET_BLUE;
        acase 2: color = OFFSET_GREY;
        acase 3: color = OFFSET_YELLOW + 31; // ie. 127
        acase 4: color = OFFSET_PURPLE;
        }
#endif
        for (y = 0; y < BALL_DIAMETER; y++)
        {   for (x = 0; x < BALL_DIAMETER; x++)
            {   if
                (   (x - BALL_RADIUS) * (x - BALL_RADIUS) + (y - BALL_RADIUS) * (y - BALL_RADIUS)
                  < (BALL_RADIUS - 0.5) * (BALL_RADIUS - 0.5)
                )
                {   r = (x - 3 * BALL_RADIUS / 4)
                      * (x - 3 * BALL_RADIUS / 4)
                      + (y -     BALL_RADIUS / 4)
                      * (y -     BALL_RADIUS / 4);
#ifdef AMIGA
                    r = (int) (r * 32 / (1.5 * BALL_RADIUS) / (1.5 * BALL_RADIUS));
                    if (r > 31)
                    {   r = 31;
                    }
                    lball_bitmap[po][x][y] = ((po == 3) ? (color - r) : (color + r));
#endif
#ifdef WIN32
                    r = (int) (r * 256 / (1.5 * BALL_RADIUS) / (1.5 * BALL_RADIUS));
                    switch (po)
                    {
                    case  0: green = (r > 0xF8) ? 0 : (0xF8 - r); red = blue  = (r > 0xE0 / 4) ? 0 : (0xE0 - (r * 4));
                    acase 1: blue  = (r > 0xF8) ? 0 : (0xF8 - r); red = green = (r > 0x70 / 2) ? 0 : (0x70 - (r * 2));
                    acase 2: red   = green = blue = (r > 0xF8) ? 0xF8 : r;
                    acase 3: if (r > 0xF8) { red = green = 0xF8; } else { red = green =        r; } if (r < 0xE0 / 4) { blue  = 0; } else { blue  =         r * 4 ; } // yellow
                    acase 4: if (r > 0xF8) { red = blue  =    0; } else { red = blue  = 0xF8 - r; } if (r > 0xE0 / 4) { green = 0; } else { green = 0xE0 - (r * 4); } // purple
                    }
                    lball_bitmap[po][x][y] = (red << 16) | (green << 8) | blue;
#endif
                } else
                {   lball_bitmap[po][x][y] = NOTHING;
    }   }   }   }

    for (y = 0; y < APPLE_DIAMETER; y++)
    {   for (x = 0; x < APPLE_DIAMETER; x++)
        {   radius = APPLE_RADIUS * (abs(x - APPLE_RADIUS) / 2 + 50) / 60;
            if (radius > APPLE_RADIUS)
            {   radius = APPLE_RADIUS;
            }
            if
            (   (x - APPLE_RADIUS) * (x - APPLE_RADIUS) + (y - APPLE_RADIUS) * (y - APPLE_RADIUS)
              < radius * radius
            )
            {   r = (x - 3 * APPLE_RADIUS / 4)
                  * (x - 3 * APPLE_RADIUS / 4)
                  + (y -     APPLE_RADIUS / 4)
                  * (y -     APPLE_RADIUS / 4);
#ifdef AMIGA
                r = (int) (3 + r * 22 / (1.5 * radius) / (1.5 * radius));
                if (r > 31)
                {   r = 31;
                }
                apple_bitmap[x][y] = OFFSET_RED + r;
#endif
#ifdef WIN32
                r = (int) (24 + r * 176 / (1.5 * radius) / (1.5 * radius));
                if (r > 0xF8)
                {   red = 0;
                } else
                {   red = 0xF8 - r;
                }
                if (r > 0xB0 / 3)
                {   green = blue = 0;
                } else
                {   green = blue = 0xB0 - (r * 3);
                }
                apple_bitmap[x][y] = (red << 16) | (green << 8) | blue;
#endif
            } else
            {   apple_bitmap[x][y] = NOTHING;
    }   }   }

    for (y = 0; y < LUNATIC_DIAMETER; y++)
    {   for (x = 0; x < LUNATIC_DIAMETER; x++)
        {   if
            (   (x - LUNATIC_RADIUS) * (x - LUNATIC_RADIUS) + (y - LUNATIC_RADIUS) * (y - LUNATIC_RADIUS)
              < (LUNATIC_RADIUS - 0.5) * (LUNATIC_RADIUS - 0.5)
            )
            {   r = (x - 3 * LUNATIC_RADIUS / 4)
                  * (x - 3 * LUNATIC_RADIUS / 4)
                  + (y -     LUNATIC_RADIUS / 4)
                  * (y -     LUNATIC_RADIUS / 4);
#ifdef AMIGA
                r = (int) (r * 32 / (1.5 * LUNATIC_RADIUS) / (1.5 * LUNATIC_RADIUS));
                if (r > 31)
                {   r = 31;
                }
                lunatic_bitmap[x][y] = OFFSET_GREY + r;
#endif
#ifdef WIN32
                r = (int) (r * 256 / (1.5 * LUNATIC_RADIUS) / (1.5 * LUNATIC_RADIUS));
                red = green = blue = (r > 0xF8) ? 0xF8 : r;
                lunatic_bitmap[x][y] = (red << 16) | (green << 8) | blue;
#endif
            } else
            {   lunatic_bitmap[x][y] = NOTHING;
}   }   }   }

EXPORT void make_logo(void)
{   int x, y;

    for (y = 0; y < 16; y++)
    {   for (x = 0; x < 16; x++)
        {   switch (amigan[y][x])
            {
            case  'W': amigan[y][x] = WHITE;
            acase 'A': amigan[y][x] = LOGOGREY;
            acase 'B': amigan[y][x] = LOGOBLUE;
            acase 'Y': amigan[y][x] = LOGOYELLOW;
            acase 'C': amigan[y][x] = LOGOCYAN;
            acase 'G': amigan[y][x] = LOGOGREEN;
            acase 'R': amigan[y][x] = RED;
            acase '.': amigan[y][x] = BLACK;
}   }   }   }

EXPORT void draw_eye(int x, int y, int which)
{   int xx, yy;

    for (xx = 0; xx < EYE_DIAMETER; xx++)
    {   for (yy = 0; yy < EYE_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && eye_bitmap[which][xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[eye_bitmap[which][xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      eye_bitmap[which][xx][yy] ;
#endif
}   }   }   }

EXPORT void draw_ball(int x, int y)
{   int xx, yy;

    for (xx = 0; xx < BALL_DIAMETER; xx++)
    {   for (yy = 0; yy < BALL_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && ball_bitmap[xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[ball_bitmap[xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      ball_bitmap[xx][yy] ;
#endif
}   }   }   }

EXPORT void draw_lball(int x, int y, int which)
{   int xx, yy;

    for (xx = 0; xx < BALL_DIAMETER; xx++)
    {   for (yy = 0; yy < BALL_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && lball_bitmap[which][xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[lball_bitmap[which][xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      lball_bitmap[which][xx][yy] ;
#endif
}   }   }   }

EXPORT void draw_hole(int x, int y, int which)
{   int xx, yy;

    for (xx = 0; xx < HOLE_DIAMETER; xx++)
    {   for (yy = 0; yy < HOLE_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && hole_bitmap[which][xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[hole_bitmap[which][xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      hole_bitmap[which][xx][yy] ;
#endif
}   }   }   }

EXPORT void draw_bball(int x, int y)
{   int xx, yy;

    for (xx = 0; xx < BBALL_DIAMETER; xx++)
    {   for (yy = 0; yy < BBALL_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && bball_bitmap[xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[bball_bitmap[xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      bball_bitmap[xx][yy] ;
#endif
}   }   }   }

EXPORT void draw_inspector(int x, int y)
{   int xx, yy;

    for (xx = 0; xx < INSPECTOR_DIAMETER; xx++)
    {   for (yy = 0; yy < INSPECTOR_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && inspector_bitmap[xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[inspector_bitmap[xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      inspector_bitmap[xx][yy] ;
#endif
}   }   }   }

EXPORT void draw_rocket(int x, int y, int which)
{   int xx, yy;

    for (xx = 0; xx < ROCKET_DIAMETER; xx++)
    {   for (yy = 0; yy < ROCKET_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && rocket_bitmap[which][xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[rocket_bitmap[which][xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      rocket_bitmap[which][xx][yy] ;
#endif
}   }   }   }

EXPORT void draw_lunatic(int x, int y)
{   int xx, yy;

    for (xx = 0; xx < LUNATIC_DIAMETER; xx++)
    {   for (yy = 0; yy < LUNATIC_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && lunatic_bitmap[xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[lunatic_bitmap[xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      lunatic_bitmap[xx][yy] ;
#endif
}   }   }   }

EXPORT void draw_apple(int x, int y)
{   int xx, yy;

    for (xx = 0; xx < APPLE_DIAMETER; xx++)
    {   for (yy = 0; yy < APPLE_DIAMETER; yy++)
        {   if
            (   x + xx >= 0
             && x + xx <  MAPWIDTH
             && y + yy >= 0
             && y + yy <  MAPHEIGHT
             && apple_bitmap[xx][yy] != NOTHING
            )
            {
#ifdef AMIGA
                display[y + yy][x + xx] = pens[apple_bitmap[xx][yy]];
#endif
#ifdef WIN32
                display[y + yy][x + xx] =      apple_bitmap[xx][yy] ;
#endif
}   }   }   }

EXPORT void gl_write(int x, int y, char* s)
{   int xx, yy;

    while (*s)
    {   for (xx = 0; xx < 8; xx++)
        {   for (yy = 0; yy < 8; yy++)
            {   if
                (   x + xx >= 0
                 && x + xx <  WINWIDTH
                 && y + yy >= 0
                 && y + yy <  WINHEIGHT
                )
                {   if (thefont[((*s) * 8) + yy] & (128 >> xx))
                    {   display[y + yy][x + xx] = fgc;
        }   }   }   }
        s++;
        x += 8;
}   }

EXPORT void clearscreen(void)
{   int    x, y;
    ULONG  bkgrnd;
#ifdef AMIGA
    ULONG  longpen;
    ULONG* longrow;
#endif

    if (stars)
    {   for (y = 0; y < MAPHEIGHT; y++)
        {   for (x = 0; x < WINWIDTH; x++)
            {   display[y][x] = starfield[y][x];
    }   }   }
    else
    {   if (gamemode == MENU)
        {   bkgrnd = DARKBLUE;
        } elif (gameplan == COOPERATIVE)
        {   if (level == 99)
            {   bkgrnd = DARKPURPLE;
            } else
            {   switch (level % 4)
                {
                case  0:
                    bkgrnd = DARKPINK;
                acase 1:
                    bkgrnd = DARKGREEN;
                acase 2:
                    bkgrnd = DARKORANGE;
                adefault: // ie. 3
                    bkgrnd = DARKYELLOW;
        }   }   }
        else
        {   // assert(gameplan == DEATHMATCH);
            bkgrnd = DARKRED;
        }
#ifdef AMIGA
        changepen(bkgrnd);
        longpen = (pens[BGPEN] << 24)
                | (pens[BGPEN] << 16)
                | (pens[BGPEN] <<  8)
                |  pens[BGPEN];
        for (y = 0; y < MAPHEIGHT; y++)
        {   longrow = (ULONG*) display[y];
            for (x = 0; x < WINWIDTH / 4; x++)
            {   *(longrow + x) = longpen;
        }   }
#endif
#ifdef WIN32
        for (y = 0; y < MAPHEIGHT; y++)
        {   for (x = 0; x < WINWIDTH; x++)
            {   display[y][x] = pens[bkgrnd];
        }   }
#endif
    }

    for (x = 0; x <= 588; x++)
    {   for (y = MAPHEIGHT + 2; y < WINHEIGHT; y++)
        {   display[y][x] = pens[GREY];
}   }   }

EXPORT void draw_logo(void)
{   int x, y;

    for (x = 0; x < WINWIDTH; x++)
    {   display[MAPHEIGHT    ][x] = pens[WHITE];
        display[MAPHEIGHT + 1][x] = pens[BLACK];
    }
    for (y = MAPHEIGHT + 1; y < WINHEIGHT; y++)
    {   display[y][589] = pens[WHITE];
    }
    for (y = 0; y < 50; y++)
    {   for (x = 0; x < 50; x++)
        {   display[MAPHEIGHT +  2 + y][590 + x] = pens[BLACK];
    }   }
    for (y = 1; y < 15; y++)
    {   for (x = 1; x < 15; x++)
        {   display[MAPHEIGHT + 16 + y][607 + x] = pens[amigan[y][x]];
}   }   }

EXPORT void filloutro(void)
{   int x, y;

#ifdef AMIGA
    changepen(DARKCYAN);
#endif

    if (stars)
    {   for (y = 0; y < MAPHEIGHT; y++)
        {   for (x = 0; x < WINWIDTH; x++)
            {   display[y][x] = starfield[y][x];
    }   }   }
    else
    {   for (y = 0; y < MAPHEIGHT; y++)
        {   for (x = 0; x < WINWIDTH; x++)
            {
#ifdef AMIGA
                display[y][x] = pens[BGPEN];
#else
                display[y][x] = pens[DARKCYAN];
#endif
}   }   }   }
