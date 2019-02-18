#include "koules.h"

#include <stdio.h>  // sprintf()
#include <string.h> // strlen()

#ifndef EXIT_SUCCESS
    #define EXIT_SUCCESS  0
#endif
#ifndef EXIT_FAILURE
    #define EXIT_FAILURE 20
#endif

#define SENSITIVITY 2 // was 4

typedef struct
{   char  *text;
    void (*func)(void);
} KoulesMenu;

typedef struct
{   int    x, y, *number, max, min;
    int    line;
    void (*func)(void);
} Numbers;

EXPORT int                 ssound        = 1,
                           stars         = 0,
                           volume        = 1,
                           controller[MAXROCKETS] = { ABSRTKYBD, ABSLTKYBD, JOY1, JOY2, ABSMDKYBD };

IMPORT UBYTE               penter, pup, pdown, ph, pesc, pleft, pright;
IMPORT int                 difficulty,
                           gamemode,
                           gameplan,
                           startlevel,
                           maxlevel,
                           MouseButtons,
                           nrockets;
IMPORT struct KoulesObject object[MAXOBJECT];

MODULE Numbers    minim[2], maxim[2];
MODULE int        nmenu;
MODULE int        nnumbers;
MODULE KoulesMenu menu[20];
MODULE int        selected = 0;
MODULE char       playertext[9 + 1] = "1 PLAYER"; // must be enough for "# PLAYERS" (ie. 9 + 1)
MODULE char       leveltext[] = "LEVEL 000";
#define YPOSITION(i) (MAPHEIGHT/2+20-5*nmenu+10*i)
#define XPOSITION(i) (MAPWIDTH/2-4*strlen(menu[i].text))

MODULE void change_menu(void);
MODULE void change_mode(void);
MODULE void increase(int i);
MODULE void decrease(int i);
MODULE void control_change(void);
MODULE void start(void);
MODULE void playerchange(void);
MODULE void levelchange(void);
MODULE void to_main_menu(void);
MODULE void veryhard(void);
MODULE void     hard(void);
MODULE void   medium(void);
MODULE void     easy(void);
MODULE void veryeasy(void);
MODULE void deathmatch(void);
MODULE void cooperative(void);
MODULE void change_obtiznost(void);
MODULE void quit(void);

MODULE void control_change(void)
{   int player;

    player = selected - 1;

    if (controller[player] == LASTCONTROLLER)
    {   controller[player] = 0;
    } else
    {   controller[player]++;
    }

    change_menu();
}

MODULE void start(void)
{   int i;

    for (i = 0; i < nrockets; i++)
    {   object[i].score = 0;
    }
    gamemode = GAME;
    gameplan_init();
    init_objects();

    for (i = 0; i < nrockets; i++)
    {   if (controller[i] == MOUSE)
        {   confine();
            break; // for speed
}   }   }

MODULE void playerchange(void)
{   playertext[0] = nrockets + '0';

    if (nrockets == 1)
    {   playertext[8] = EOS;
    } else
    {   playertext[8] = 'S';
}   }

MODULE void levelchange(void)
{   sprintf(leveltext, "LEVEL %03i", startlevel + 1);
}

MODULE void to_main_menu(void)
{   // save_rc();
    main_menu();
}

MODULE void change_menu(void)
{   PERSIST   char  s[2][MAXROCKETS][50];
    PERSIST   char* s1;
    TRANSIENT int   i;

    menu[0].text = "BACK TO MAIN MENU";
    menu[0].func = to_main_menu;

    nnumbers = 0;
    for (i = 0; i < nrockets; i++)
    {   if (controller[i] == RELLTKYBD)
        {   s1 = "RELATIVE LEFT KEYBOARD (W/A/D)";
        } elif (controller[i] == ABSLTKYBD)
        {   s1 = "ABSOLUTE LEFT KEYBOARD (W/A/S/D)";
        } elif (controller[i] == RELMDKYBD)
        {   s1 = "RELATIVE MIDDLE KEYBOARD (I/J/L)";
        } elif (controller[i] == ABSMDKYBD)
        {   s1 = "ABSOLUTE MIDDLE KEYBOARD (I/J/K/L)";
        } elif (controller[i] == RELRTKYBD)
        {   s1 = "RELATIVE RIGHT KEYBOARD (CURSOR KEYS)";
        } elif (controller[i] == ABSRTKYBD)
        {   s1 = "ABSOLUTE RIGHT KEYBOARD (CURSOR KEYS)";
        } elif (controller[i] == JOY1)
        {   s1 = "JOYSTICK 1";
        } elif (controller[i] == JOY2)
        {   s1 = "JOYSTICK 2";
        } elif (controller[i] == JOY3)
        {   s1 = "JOYSTICK 3";
        } elif (controller[i] == JOY4)
        {   s1 = "JOYSTICK 4";
        } elif (controller[i] == MOUSE)
        {   s1 = "MOUSE";
        }
        sprintf(s[0][i], "PLAYER %i: %s", i + 1, s1);
        menu[i + 1].text = s[0][i];
        menu[i + 1].func = control_change;
    }
    nmenu = nrockets + 1;
    if (selected >= nmenu)
    {   selected = 0;
}   }

EXPORT void setsound(void)
{   ssound = !ssound;

#ifdef AMIGA
    if (ssound)
    {   start_sounds();
    } else
    {   freefx();
    }
#endif
#ifdef WIN32
    if (!ssound)
    {   stop_sounds();
    }
#endif

    main_menu();
}

MODULE void setstars(void)
{   stars = !stars;
    main_menu();
}

MODULE void veryhard(void) { difficulty = 0; main_menu(); }
MODULE void     hard(void) { difficulty = 1; main_menu(); }
MODULE void   medium(void) { difficulty = 2; main_menu(); }
MODULE void     easy(void) { difficulty = 3; main_menu(); }
MODULE void veryeasy(void) { difficulty = 4; main_menu(); }

MODULE void  deathmatch(void) { gameplan = DEATHMATCH ; main_menu(); }
MODULE void cooperative(void) { gameplan = COOPERATIVE; main_menu(); }

MODULE void change_mode(void)
{   menu[0].text = "DEATH MATCH";
    menu[0].func = deathmatch;
    menu[1].text = "COOPERATIVE";
    menu[1].func = cooperative;
    nnumbers = 0;
    nmenu    = 2;
    if (gameplan == DEATHMATCH)
    {   selected = 0;
    } else
    {   selected = 1;
}   }

MODULE void change_obtiznost(void)
{   menu[0].text = "VERY HARD";
    menu[0].func = veryhard;
    menu[1].text = "HARD";
    menu[1].func = hard;
    menu[2].text = "MEDIUM";
    menu[2].func = medium;
    menu[3].text = "EASY";
    menu[3].func = easy;
    menu[4].text = "VERY EASY";
    menu[4].func = veryeasy;
    nnumbers = 0;
    nmenu    = 5;
    selected = difficulty;
}

/***************************************************************************/

EXPORT void main_menu(void)
{   nnumbers = 2;
    menu[0].text = "START GAME";
    menu[0].func = start;
    menu[1].text = playertext;
    menu[1].func = playerchange;
    minim[0].x = XPOSITION(1);
    minim[0].y = YPOSITION(1);
    minim[0].line = 1;
    minim[0].number = &nrockets;
    minim[0].max = 5;
    minim[0].min = 1;
    minim[0].func = playerchange;
    maxim[0].x = XPOSITION(1) + 8 * 2;
    maxim[0].y = YPOSITION(1);
    maxim[0].line = 1;
    maxim[0].number = &nrockets;
    maxim[0].max = 5;
    maxim[0].min = 1;
    maxim[0].func = playerchange;
    playerchange();
    levelchange();
    menu[2].text = leveltext;
    menu[2].func = playerchange;
    minim[1].x = XPOSITION (2) + 7 * 7;
    minim[1].y = YPOSITION (2);
    minim[1].line = 2;
    minim[1].number = &startlevel;
    minim[1].max = maxlevel;
    minim[1].min = 0;
    minim[1].func = levelchange;
    maxim[1].x = XPOSITION (2) + 11 * 7;
    maxim[1].y = YPOSITION (2);
    maxim[1].line = 2;
    maxim[1].number = &startlevel;
    maxim[1].max = maxlevel;
    maxim[1].min = 0;
    maxim[1].func = levelchange;
    nnumbers = 2;
    menu[3].text = "CONTROLS";
    menu[3].func = change_menu;
    menu[4].text = "GAME MODE";
    menu[4].func = change_mode;
    menu[5].text = "DIFFICULTY";
    menu[5].func = change_obtiznost;
    menu[6].text = ssound ? "SOUND ON" : "SOUND OFF";
    menu[6].func = setsound;
    menu[7].text = stars ? "STARFIELD ON" : "STARFIELD OFF";
    menu[7].func = setstars;
    nmenu = 9;
    menu[8].text = "QUIT";
    menu[8].func = quit;
    selected = 0;
}

EXPORT void init_menu(void)
{   playertext[0] = nrockets + '0';
    main_menu();
}

EXPORT void draw_menu(void)
{   int i;

    levelchange();

    DrawWhiteText(MAPWIDTH / 2 -                   15 * 4,   50, "* K O U L E S *");
    DrawWhiteText(MAPWIDTH / 2 -                   25 * 4,   70, "LINUX VERSION BY JAHUSOFT");
#ifdef WIN32
    DrawWhiteText(MAPWIDTH / 2 -                   31 * 4,   80, "WINDOWS PORT BY AMIGAN SOFTWARE");
#endif
#ifdef AMIGA
    DrawWhiteText(MAPWIDTH / 2 -                   30 * 4,   80, "AMIGA PORTS BY AMIGAN SOFTWARE");
#endif
    DrawWhiteText(MAPWIDTH / 2 - (strlen(VERSIONTEXT) * 4), 100, VERSIONTEXT);
    DrawWhiteText(MAPWIDTH / 2 -                   30 * 4,  120, "PRESS P FOR PAUSE / H FOR HELP");

    for (i = 0; i < nmenu; i++)
    {   if (i == selected)
        {   DrawRedText(  XPOSITION(i)    , YPOSITION(i)    , menu[i].text);
        } else
        {   DrawWhiteText(XPOSITION(i)    , YPOSITION(i)    , menu[i].text);
}   }   }

PERSIST int inctime,
            changed,
            waittime;

MODULE void increase(int i)
{   changed = 1;
    if (waittime)
    {   waittime--;
        return;
    }
    waittime = inctime;
    if (inctime > 1)
    {   inctime--;
    } else
    {   (*maxim[i].number) += 2;
    }
    (*maxim[i].number)++;
    if (*maxim[i].number > maxim[i].max)
    {   *maxim[i].number = maxim[i].max;
    }
    if (ssound && inctime != 1)
    {   play_sound(S_CREATOR2);
    }
    maxim[i].func();
}

MODULE void decrease(int i)
{   changed = 1;
    if (waittime)
    {   waittime--;
        return;
    }
    waittime = inctime;
    if (inctime > 1)
    {  inctime--;
    } else
    {  (*maxim[i].number) -= 2;
    }
    (*minim[i].number)--;
    if (*minim[i].number < minim[i].min)
    {   *minim[i].number = minim[i].min;
    }
    if (ssound && inctime != 1)
    {   play_sound(S_CREATOR2);
    }
    minim[i].func();
}

EXPORT void menu_keys(void)
{   PERSIST int enter,
                esc,
                mtime = SENSITIVITY;
    auto    int ent   = 0,
                i;

    if (maxim[1].max != maxlevel)
    {   main_menu();
    }
    changed = 0;

    if (pleft)
    {   if (selected == 6 || selected == 7)
        {   if (ssound)
            {   play_sound(S_CREATOR2);
            }
            menu[selected].func();
        } else
        {   for (i = 0; i < nnumbers; i++)
            {   if (maxim[i].line == selected)
                {   decrease(i);
    }   }   }   }

    if (pright)
    {   if (selected == 6 || selected == 7)
        {   if (ssound)
            {   play_sound(S_CREATOR2);
            }
            menu[selected].func();
        } else
        {   for (i = 0; i < nnumbers; i++)
            {   if (maxim[i].line == selected)
                {   increase(i);
    }   }   }   }

    if (pesc)
    {   if (!esc)
        {   if (nnumbers == 2)
            {   cleanexit(EXIT_SUCCESS);
            }
            to_main_menu();
        }
        esc = 1;
    } else esc = 0;

    if (pup && selected > 0)
    {   if (mtime == SENSITIVITY)
        {   selected--;
            mtime = 0;
        } else
        {   mtime++;
    }   }

    if (pdown && selected < nmenu - 1)
    {   if (mtime == SENSITIVITY)
        {   selected++;
            mtime = 0;
        } else
        {   mtime++;
    }   }

    if (!pup && !pdown)
    {   mtime = SENSITIVITY;
    }

    if (penter || MouseButtons)
    {   ent = 1;
    }

    if (!ent && enter)
    {   if (ssound)
        {   play_sound(S_CREATOR2);
        }
        menu[selected].func();
    }

    enter = ent;
    if (!changed)
    {   inctime = 7;
        waittime = 0;
}   }

MODULE void quit(void)
{   cleanexit(EXIT_SUCCESS);
}
