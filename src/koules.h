#define DECIMALVERSION "1.33"
#define INTEGERVERSION "1.33"
#define VERSION        "\0$VER: Koules " INTEGERVERSION " (11.11.2018)" // d.m.yyyy format
#define RELEASEDATE    "11-11-18" // dd-mm-yy format. Year *must* be only 2 digits!
#define VERSIONTEXT    "VERSION 1.4 RELEASE " DECIMALVERSION ": 11 NOV 2018"
#define COPYRIGHT      "© 2006-2018 James Jacobs of Amigan Software"
// VERSION needs a leading \0 for OS4 to correctly find it

#define TITLEBARTEXT   "Koules " DECIMALVERSION

#define INCL_DOS
#define INCL_GPI
#define INCL_WIN

#ifdef WIN32
    #include <windows.h>
    #define inline         __inline
    typedef unsigned char  UBYTE;
    typedef unsigned char* STRPTR;
    #define M_PI           3.14159265358979323846264338327950288
 // #define USECONSOLE
#endif
#ifdef AMIGA
    #include <exec/types.h>
    #define __inline
    #define MAX_PATH       512
    #ifdef __SASC
        #define M_PI       3.14159265358979323846264338327950288
    #endif
#endif
#ifdef __amigaos4__
    #define chip
#endif /* __amigaos4__ */

#define WINWIDTH   640
#define MAPWIDTH   640
#define GAMEWIDTH  640
#define WINHEIGHT  512
#define MAPHEIGHT  460
#define GAMEHEIGHT 460

#define S_START          0
#define S_END            1
#define S_COLIZE         2
#define S_DESTROY_BALL   3
#define S_DESTROY_ROCKET 4
#define S_CREATOR1       5
#define S_CREATOR2       6

#define RELLTKYBD        0
#define ABSLTKYBD        1
#define RELMDKYBD        2
#define ABSMDKYBD        3
#define RELRTKYBD        4
#define ABSRTKYBD        5
#define JOY1             6
#define JOY2             7
#define JOY3             8
#define JOY4             9
#define MOUSE           10
#define LASTCONTROLLER  MOUSE

#define TRANSIENT      auto
#define MODULE         static
#define EXPORT
#define IMPORT         extern
#define elif           else if
#define acase          break; case
#define adefault       break; default
#define EOS            '\0' // end of string
#define DISCARD        (void)
#define PERSIST        static
typedef signed char    SBYTE;
typedef signed short   SWORD;
typedef signed long    SLONG;
typedef unsigned char  FLAG;

EXPORT struct KoulesObject
{   int   type,
          thief,
          ctype,
          live,
          time,
          score,
          lineto,
          live1,    /* backup for rockets */
          radius;
    float x,
          y,
          fx,       /* forces */
          fy,
          rotation, /* for rockets */
          M,
          accel;
    char  letter;
};

// amiga|ibm.c
EXPORT void cleanexit(SBYTE rc);
EXPORT FLAG UpdateInput(void);
EXPORT void updatescreen(void);
EXPORT void checkpointer(void);
EXPORT void clearkybd(void);
EXPORT int  ReadJoystick(int joynum);
EXPORT void setvolume(void);
#if defined(WIN32) && defined(USECONSOLE)
    EXPORT void openconsole(void);
#endif
EXPORT void confine(void);
EXPORT void unconfine(void);
#ifdef AMIGA
    EXPORT void start_sounds(void);
    EXPORT void freefx(void);
#endif
EXPORT void stop_sounds(void);
EXPORT void checksound(void);
EXPORT BOOL IsPressed(int which);
EXPORT void play_sound(int which);
EXPORT void usleep(unsigned long s);
EXPORT void changepen(int towhat);

// framebuffer.c
EXPORT void create_bitmaps(void);
EXPORT void draw_logo(void);
EXPORT void make_logo(void);
EXPORT void draw_apple(int x, int y);
EXPORT void draw_ball(int x, int y);
EXPORT void draw_bball(int x, int y);
EXPORT void draw_eye(int x, int y, int which);
EXPORT void draw_hole(int x, int y, int which);
EXPORT void draw_inspector(int x, int y);
EXPORT void draw_lball(int x, int y, int which);
EXPORT void draw_lunatic(int x, int y);
EXPORT void draw_rocket(int x, int y, int which);
EXPORT void gl_write(int x, int y, char* s);
EXPORT void clearscreen(void);
EXPORT void filloutro(void);

// gameplan.c
EXPORT void gameplan_init(void);
EXPORT void init_objects(void);
EXPORT int find_position(float*, float*, CONST float);
EXPORT int create_letter(void);
EXPORT int allow_finder(void);
EXPORT void update_game(void);

// koules.c
EXPORT void clearpoints(void);
EXPORT void creator(CONST int type);
EXPORT float M(CONST int type);
EXPORT int radius(CONST int type);
EXPORT void creator(CONST int type);
EXPORT void statusarea(void);
EXPORT void game(void);
EXPORT void DrawBlackMaskedText(int x, int y, char* text);
EXPORT void DrawRedText(int x, int y, char* text);
EXPORT void DrawWhiteText(int x, int y, char* text);
EXPORT void DrawColouredText(int x, int y, char* text, int player);
EXPORT void draw_objects(void);

// menu.c
EXPORT void main_menu(void);
EXPORT void draw_menu(void);
EXPORT void init_menu(void);
EXPORT void menu_keys(void);
EXPORT void setsound(void);

/* do not confuse compiler when function is not required */
#if defined(__GNUC__) && !defined(ONLYANSI)
#define CONST const
    #ifndef INLINE
        #define INLINE inline
    #endif
#else
#define INLINE
#endif
#define DUMMY do { } while (0)

#define MENUTIME 5

#define RAD(n)  ((float)(n)/180.0*M_PI)
#define ROTSTEP RAD(10)

#define EYE_RADIUS        5
#define MOUSE_RADIUS      4
#define BALL_RADIUS       8
#define BBALL_RADIUS     16
#define APPLE_RADIUS     32
#define INSPECTOR_RADIUS 14
#define LUNATIC_RADIUS   EYE_RADIUS
#define HOLE_RADIUS      12
#define ROCKET_RADIUS    14
#define EYE_RADIUS1      10
#define SPRINGSIZE       (BBALL_RADIUS * 4)
#define SPRINGSTRENGTH   (BBALL_RADIUS / 2)

#define       EYE_DIAMETER (      EYE_RADIUS * 2)
#define      BALL_DIAMETER (     BALL_RADIUS * 2)
#define     BBALL_DIAMETER (    BBALL_RADIUS * 2)
#define     APPLE_DIAMETER (    APPLE_RADIUS * 2)
#define INSPECTOR_DIAMETER (INSPECTOR_RADIUS * 2)
#define   LUNATIC_DIAMETER (  LUNATIC_RADIUS * 2)
#define      HOLE_DIAMETER (     HOLE_RADIUS * 2)
#define    ROCKET_DIAMETER (   ROCKET_RADIUS * 2)

#define NTRACKS 4
#define ROCKET 1
#define BALL 2
#define LBALL 3
#define CREATOR 4
#define HOLE 5
#define BBALL 6
#define APPLE 7
#define INSPECTOR 8
#define EHOLE 9
#define LUNATIC 10

#define MAXOBJECT 255
#ifdef AMIGA
    #define MAXPOINT (2000)
#else
    #define MAXPOINT (4000)
#endif
#define MAXROCKETS 5

#define L_ACCEL 'A'
#define L_GUMM 'M'
#define L_THIEF 'T'
#define L_FINDER 'G'
#define L_TTOOL 'S'
#define A_ADD 0.13
#define M_ADD 0.8
#define NLETTERS 5

#define S_START 0
#define S_END 1
#define S_COLIZE 2
#define S_DESTROY_BALL 3
#define S_DESTROY_ROCKET 4
#define S_CREATOR1 5
#define S_CREATOR2 6

#define C_REMOTE    0
#define C_KEYBOARD  1
#define C_RKEYBOARD 2
#define C_JOYSTICK1 3
#define C_JOYSTICK2 4
#define C_MOUSE     5

#define DEATHMATCH  0
#define COOPERATIVE 1

#define NSAMPLES    7

#define MENU    1
#define KEYS    2
#define GAME    3
#define JOY     4
#define WAIT    5
#define PREGAME 6

#define BLACK           0
#define WHITE           1
#define GREY            2
#define RED             3
#define DARKPINK        4
#define DARKGREEN       5
#define DARKORANGE      6
#define DARKYELLOW      7
#define DARKBLUE        8
#define DARKPURPLE      9
#define DARKRED        10
#define DARKCYAN       11
#define BGPEN          12
#define LOGOYELLOW     13
#define LOGOCYAN       14
#define LOGOGREEN      15
#define LOGOBLUE       16
#define LOGOGREY       17
#define OFFSET_STARS   18
#define OFFSET_PURPLE  32
#define FLAT_PURPLE    39
#define OFFSET_RED     64
#define FLAT_RED       71
#define OFFSET_YELLOW  96
#define FLAT_YELLOW   103
#define OFFSET_GREEN  128
#define FLAT_GREEN    135
#define OFFSET_BLUE   160
#define FLAT_BLUE     163
#define OFFSET_GREY   192
#define FLAT_GREY     199
