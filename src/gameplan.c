#include "koules.h"
#include "text.h"

#include <stdio.h>  // sprintf()
#include <stdlib.h> // rand()
#include <string.h> // strlen()
#include <math.h>

#define HOLELEVEL 5
#define BBALLLEVEL (nrockets==1?12:10)
#define EHOLELEVEL 20
#define SPRINGLEVEL 30
#define THIEFLEVEL 40
#define FINDERLEVEL 50
#define TTOOLLEVEL 60
#define INSPECTORLEVEL 70
#define BBBALLLEVEL 90
#define LUNATICLEVEL 80

EXPORT int                 level = 0;

IMPORT struct KoulesObject object[MAXOBJECT];

IMPORT float    ROCKET_SPEED;
IMPORT float    BALL_SPEED;
IMPORT float    BBALL_SPEED;
IMPORT float    SLOWDOWN;
IMPORT float    GUMM;

IMPORT float    BALLM;
IMPORT float    LBALLM;
IMPORT float    BBALLM;
IMPORT float    APPLEM;
IMPORT float    INSPECTORM;
IMPORT float    LUNATICM;
IMPORT float    ROCKETM;

IMPORT int      dosprings;
IMPORT int      difficulty;
IMPORT int      randsprings;
IMPORT int      nobjects;
IMPORT int      nrockets;
IMPORT int      gameplan;
IMPORT char     control[MAXROCKETS];
IMPORT int      startlevel;
IMPORT int      mouseplayer;
IMPORT int      gamemode;
IMPORT int      a_bballs, a_rockets, a_balls, a_holes, a_apples, a_inspectors,
                a_lunatics, a_eholes;
IMPORT int      keys[3][4],
                maxlevel;
IMPORT UBYTE    pesc;

MODULE int      gtime;
MODULE int      ktime = 0;
MODULE int      kmode = 0;
MODULE int      secondpart;

MODULE void outro(CONST int size, char* text[]);

EXPORT void gameplan_init(void)
{   level = startlevel;
}

/*
 * level creation routines
 */

EXPORT void init_objects(void)
{   int  i,
         nholes;
    char s[20];

    switch (difficulty)
    {
    case 0:
        ROCKET_SPEED = (float) 0.8;
        BALL_SPEED   = (float) 1.2;
        BBALL_SPEED  = (float) 1.2;
        SLOWDOWN     = (float) 0.9;
        GUMM         =        20;

        BALLM = 3;
        LBALLM = 3;
        BBALLM = 8;
        APPLEM = 40;
        ROCKETM = 2;
    acase 1:
        ROCKET_SPEED = (float) 1.0;
        BALL_SPEED   = (float) 1.2;
        BBALL_SPEED  = (float) 1.2;
        SLOWDOWN     = (float) 0.9;
        GUMM         =        20;

        BALLM = 3;
        LBALLM = 3;
        BBALLM = 8;
        APPLEM = 40;
        ROCKETM = 4;
    acase 2:
        ROCKET_SPEED = (float) 1.2;
        BALL_SPEED   = (float) 1.2;
        BBALL_SPEED  = (float) 1.2;
        SLOWDOWN     = (float) 0.8;
        GUMM         =        20;

        APPLEM = 34;
        BALLM = 3;
        LBALLM = 3;
        BBALLM = 8;
        ROCKETM = 4;
    acase 3:
        ROCKET_SPEED = (float) 2.0;
        BALL_SPEED   = (float) 1.2;
        BBALL_SPEED  = (float) 1.2;
        SLOWDOWN     = (float) 0.8;
        GUMM         =        20;

        BALLM = 3;
        LBALLM = 3;
        APPLEM = 24;
        BBALLM = 8;
        ROCKETM = 5;
    acase 4:
        ROCKET_SPEED = (float) 2.0;
        BALL_SPEED   = (float) 1.2;
        BBALL_SPEED  = (float) 1.2;
        SLOWDOWN     = (float) 0.8;
        GUMM         =        15;

        BALLM = 3;
        LBALLM = 3;
        APPLEM = 24;
        BBALLM = 8;
        ROCKETM = 7;
    }

    clearpoints();

    dosprings = 0;
    if (maxlevel < level)
    {   maxlevel = level;
    }
    if (gamemode != GAME)
    {   nobjects = 0;
        return;
    }

    switch (gameplan)
    {
    case DEATHMATCH:
        dosprings = 1;        /* enable spring in death match */
        randsprings = 40;
        if (nrockets != 1)
        {   nobjects = nrockets * 3 * GAMEWIDTH / 640 * GAMEHEIGHT / 460 + 3;
            for (i = 0; i < nobjects; i++)
            {   object[i].live = 0;
            }
            for (i = 0; i < nobjects; i++)
            {   object[i].live = 1;
                object[i].lineto = -1;
                object[i].live1 = 1;
                object[i].thief = 0;
                object[i].time = (i < nrockets ? 99 : 0);
                object[i].fx = 0;
                object[i].fy = 0;
                object[i].rotation = 0;
                object[i].type = (i < nrockets ? ROCKET : BALL);
                if (!find_position(&object[i].x, &object[i].y, (float) radius(object[i].type)))
                {   return;
                }
                object[i].M = M(object[i].type);
                object[i].radius = radius(object[i].type);
                object[i].accel = ROCKET_SPEED;
                object[i].letter = ' ';
                secondpart = 0;
            }
            nholes = nrockets / 3 + 1;
            for (i = 0; i < nholes; i++)
            {   creator(HOLE);
            }
            creator(INSPECTOR);
            creator(EHOLE);
        } else
        {   nobjects = 2;
            for (i = 0; i < nobjects; i++)
                object[i].live = 0;
            for (i = 0; i < nobjects; i++)
            {   object[i].live = 1;
                object[i].lineto = -1;
                if (i != 0)
                    object[i].lineto = 0;
                object[i].live1 = 1;
                object[i].thief = 0;
                object[i].time = (i < nrockets ? 99 : 0);
                object[i].fx = 0;
                object[i].fy = 0;
                object[i].rotation = 0;
                object[i].type = (i < nrockets ? ROCKET : LUNATIC);
                if (!find_position(&object[i].x, &object[i].y, (float) radius(object[i].type)))
                {   return;
                }
                object[i].M = M(object[i].type);
                object[i].radius = radius(object[i].type);
                object[i].accel = ROCKET_SPEED;
                object[i].letter = ' ';
                secondpart = 0;
        }   }
    acase COOPERATIVE:
        if (level != 99)
        {   int nbballs;

            if (level > SPRINGLEVEL)
                dosprings = 1;
            randsprings = 40 - level / 3;
            nobjects = (int) (3 + sqrt((double) (level * ((nrockets + 1) / 2) + 2 * nrockets)));
            nobjects = (int) (nobjects * (GAMEWIDTH / 640 * GAMEHEIGHT / 460 + 2) / 3);
            if (nobjects > 30)
                nobjects = 30;
            for (i = 0; i < nobjects; i++)
                object[i].live = 0;
            nbballs = nrockets + level / BBBALLLEVEL;
            gtime = 100 + 1000 / (level + 1);
            for (i = 0; i < nobjects; i++)
            {   object[i].live = (i < nrockets ? 5 : 1);
                object[i].live1 = (i < nrockets ? 5 : 1);
                object[i].lineto = -1;
                object[i].thief = 0;
                object[i].time = (i < nrockets ? 100 : 0);
                object[i].fx = 0;
                object[i].fy = 0;
                object[i].rotation = 0;
                object[i].type = (i < nrockets ? ROCKET : (i < nbballs ? BBALL : BALL));
                object[i].M = M(object[i].type);
                if (i < nrockets)
                {   if (level < 5)
                        object[i].M *= (float) (1.0 + (5.0 - level) /  15.0);
                    if (level < 25)
                        object[i].M *= (float) (1.0 +        level  / 120.0);
                }
                object[i].radius = radius (object[i].type);
                object[i].accel = ROCKET_SPEED;
                if (!find_position(&object[i].x, &object[i].y, (float) radius(object[i].type)))
                {   return;
                }
                object[i].letter = ' ';
        }   }
        else
        {   nobjects = nrockets + 10;
            for (i = 0; i < nobjects; i++)
            {   object[i].live = 0;
            }
            object[nrockets].type = APPLE;
            object[nrockets].M = APPLEM;
            object[nrockets].lineto = -1;
            object[nrockets].thief = 0;
            object[nrockets].radius = APPLE_RADIUS;
            object[nrockets].live = (i < nrockets ? 5 : 1);
            object[nrockets].fx = 0;
            object[nrockets].fy = 0;
            object[nrockets].x = (float) (GAMEWIDTH  / 2);
            object[nrockets].y = (float) (GAMEHEIGHT / 2);
            for (i = 0; i < nrockets; i++)
            {   object[i].live = 5;
                object[i].live1 = 5;
                object[i].time = 100;
                object[i].thief = 0;
                object[i].lineto = -1;
                object[i].fx = 0;
                object[i].fy = 0;
                object[i].rotation = 0;
                object[i].type = ROCKET;
                object[i].accel = ROCKET_SPEED;
                object[i].M = M(object[i].type);
                object[i].radius = radius (object[i].type);
                object[i].letter = ' ';
                object[i].x = (float) (GAMEWIDTH  / 2 + sin(i * RAD(360) / nrockets) * GAMEWIDTH  / 3);
                object[i].y = (float) (GAMEHEIGHT / 2 + cos(i * RAD(360) / nrockets) * GAMEHEIGHT / 3);
            }
            for (i = nrockets + 1; i < nobjects; i++)
            {   object[i].live = 1;
                object[i].live1 = 1;
                object[i].lineto = -1;
                object[i].time = 0;
                object[i].thief = 0;
                object[i].fx = 0;
                object[i].fy = 0;
                object[i].rotation = 0;
                object[i].type = BALL;
                object[i].accel = BALL_SPEED;
                object[i].M = M(object[i].type);
                object[i].radius = radius (object[i].type);
                object[i].letter = ' ';
                if (!find_position(&object[i].x, &object[i].y, (float) radius(object[i].type)))
                {   return;
    }   }   }   }
    ktime = 0;

    if (gamemode == GAME)
    {   if (level == 99) // 99 just finished, 100 about to start
        {   play_sound(S_END);
        } else
        {   play_sound(S_START);
        }
        if (gameplan == COOPERATIVE)
        {   switch (level)
            {
            case 0:
                outro(INTROSIZE,     introtext    );
            acase HOLELEVEL + 1:
                outro(HOLESIZE,      holetext     );
            acase INSPECTORLEVEL:
                outro(INSPECTORSIZE, inspectortext);
            acase BBBALLLEVEL:
                outro(BBBALLSIZE,    bbballtext   );
            acase EHOLELEVEL:
                outro(MAGSIZE,       magholetext  );
            acase SPRINGLEVEL + 1:
                outro(SPRINGTSIZE,   springtext   );
            acase THIEFLEVEL:
                outro(THIEFSIZE,     thieftext    );
            acase FINDERLEVEL:
                outro(FINDERSIZE,    findertext   );
            acase TTOOLLEVEL:
                outro(TTOOLSIZE,     ttooltext    );
            acase LUNATICLEVEL:
                outro(LUNATICSIZE,   lunatictext  );
            acase 99:
                outro(TEXTSIZE1,     text1        );
            adefault:
                if (level == BBALLLEVEL + 1)
                {   outro(BBALLSIZE, bballtext    );
        }   }   }
        if (pesc)
        {   gamemode = MENU;
            unconfine();
            pesc = 0;
            /* If the user takes their finger off before any repeat messages are sent,
               they will be at the main menu...otherwise they will exit to the OS. */
            return;
        }
        clearscreen();
        if (gameplan == COOPERATIVE)
            sprintf(s, "SECTOR %i", level + 1);
        else
            sprintf(s, "GET READY");
        DrawWhiteText(MAPWIDTH / 2 - 32, MAPHEIGHT / 2 - 4, s);
        statusarea();
        updatescreen();
        usleep(0); // needed (at least on IBM-PC)
        usleep(1000000);
        UpdateInput();
}   }

EXPORT int allow_finder(void)
{   if (level > FINDERLEVEL || gameplan == DEATHMATCH)
    {   return 1;
    }
    return 0;
}

EXPORT int create_letter(void)
{   int tirage,
        chance,
        letter,
        i;

    if (gameplan == COOPERATIVE)
    {   i = rand() % 4;
        if (i < 3)
            return i;
        if (level > THIEFLEVEL && rand() % 400 < level - THIEFLEVEL + 40)
            return 3;
        if (level > TTOOLLEVEL && rand() % 600 < level - TTOOLLEVEL + 40)
            return 5;
        if (level > FINDERLEVEL && gtime < 0 && rand() % 700 < level - FINDERLEVEL + 40)
            return 4;
        return 0;
    }
    chance = rand() % 100;
    if (chance < 80)
    {   /* create letter : = 1 rand / 2 */
        tirage = rand() % 100;
        if (tirage < 30)
            letter = 1;
        elif (tirage < 60)
            letter = 2;
        elif (tirage < 70)
            letter = 5;
        elif (tirage < 80)
            letter = 3;
        else
            letter = 4;
        return letter;
    } else
    {   return 0;
}   }

EXPORT void update_game(void)
{   int i;

    if (gamemode == GAME)
    {   if (ktime)
        {   ktime--;
            if (ktime == 0)
            {   switch (kmode)
                {
                case 1:
                    init_objects();
                acase 2:
                    startlevel = 0;
                    level = 1;
                    gamemode = MENU;
                    unconfine();
                    outro(TEXTSIZE2, text2);
                acase 3:
                    if (a_balls || a_bballs || a_inspectors || a_lunatics)
                    {   break;
                    }
                    for (i = 0; i < nrockets; i++)
                    {   if (object[i].type == ROCKET && object[i].live)
                        {   object[i].score += object[i].live * 20;
                    }   }
                    level++;
                    init_objects();
                acase 4:
                    gamemode = MENU;
                    unconfine();
        }   }   }
        else
        {   switch (gameplan)
            {
            case DEATHMATCH:
                if (nrockets == 1)
                {   if (!(rand() % 60))
                    {   creator(HOLE);
                    }
                    if (a_rockets == 0)
                    {   ktime = 50;
                        kmode = 1;
                        object[0].score -= 100;
                    }
                    if (a_lunatics == 0)
                    {   ktime = 50;
                        kmode = 1;
                        object[0].score += 100;
                    }
                    break;
                }
                if (a_balls == 0)
                {   secondpart = 1;
                }
                if (a_lunatics < nrockets && !(rand() % 150))
                {   creator(LUNATIC);
                }
                if (secondpart)
                {   if (!(rand() % 100))
                    {   creator(BBALL);
                    }
                    if (!(rand() %  60))
                    {   creator(HOLE);
                    }
                    if (!(rand() % 400))
                    {   creator(BBALL);
                    }
                    if (!(rand() % 400))
                    {   creator(INSPECTOR);
                    }
                    if (!(rand() % 600))
                    {   creator(EHOLE);
                }   }
                if (a_rockets <= 1)
                {   ktime = 50;
                    kmode = 1;
                }
            acase COOPERATIVE:
                if (level == 99)
                {   if (!a_apples)
                        ktime = 50, kmode = 2;
                    if (a_balls < 15)
                        if (!(rand() % 40))
                            creator(BALL);
                 /* if (a_lunatics < 3)
                        if (!(rand() % 90))
                            creator(LUNATIC); */
                    if (a_bballs < 3)
                        if (!(rand() % 3000))
                            creator(BBALL);
                } else
                {   gtime--;
                    if (a_balls == 0 && a_bballs == 0 && a_inspectors == 0 && a_lunatics == 0)
                    {   ktime = 50, kmode = 3;
                    }
                    if (a_balls < 4 * level && gtime < 0)
                        if (!(rand() % ((nrockets == 1 ? 200 : 150) + (110 - level))))
                            creator(BALL);
                    if (a_lunatics < (level - LUNATICLEVEL) && a_lunatics < 3 && gtime < 0)
                        if (!(rand() % ((nrockets == 1 ? 800 : 450) + (110 - level))))
                            creator(LUNATIC);
                    if (a_holes < 4 * (level - HOLELEVEL) && gtime < 0)
                        if (!(rand() % (412 + 512 / level)))
                            creator(HOLE);
                    if (a_bballs < 4 * (level - BBALLLEVEL) && gtime < 0)
                        if (!(rand() % ((nrockets == 1 ? 700 : 500) + 1 * (110 - level) / 3 + 2024 / level)))
                            creator (BBALL);
                    if (a_inspectors < level / (INSPECTORLEVEL) && gtime < 0)
                        if (!(rand() % (1500 + 10 * (110 - level))))
                            creator (INSPECTOR);
                    if (a_eholes < level / EHOLELEVEL + 1 && gtime < 0 && level >= EHOLELEVEL)
                        if (!(rand() % (500 + 1000 / level)))
                            creator (EHOLE);
                }
                if (a_rockets == 0)
                {   ktime = 50;
                    kmode = 4;
    }   }   }   }
    else
    {   if (a_balls    < 5 && !(rand() %  (50)))
            creator(BALL);
        if (a_lunatics < 5 && !(rand() %  (50)))
            creator(LUNATIC);
        if (a_bballs   < 4 && !(rand() % (200)))
            creator(BBALL);
}   }

MODULE void outro(CONST int size, char* text[])
{   int i;

    filloutro();

    for (i = 0; i < size; i++)
    {   DrawWhiteText
        (   (MAPWIDTH  / 2) - (strlen(text[i]) * 4),
            (MAPHEIGHT / 2) - (size * 5) + (i * 10),
            text[i]
        );
    }

    updatescreen();
    usleep(250000);

    clearkybd();

    while (!UpdateInput());

#ifdef AMIGA
    usleep(0);
#endif
}
