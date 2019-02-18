#include "koules.h"
#define PENS            224
#include "system.h"

#include "resource.h"
#include <commctrl.h>

#include <stdio.h>
#include <time.h>

#include <fcntl.h>    // for _O_TEXT
#include <io.h>       // for _open_osfhandle()

#define SCAN_W           17
#define SCAN_I           23
#define SCAN_P           25
#define SCAN_LCTRL       29
#define SCAN_A           30
#define SCAN_S           31
#define SCAN_D           32
#define SCAN_H           35
#define SCAN_J           36
#define SCAN_K           37
#define SCAN_L           38
#define SCAN_LSHIFT      42
#define SCAN_X           45
#define SCAN_RSHIFT      54
#define SCAN_F12         88
#define SCAN_UP         328
#define SCAN_DOWN       336
#define SCAN_LEFT       331
#define SCAN_RIGHT      333
#define SCAN_ENTER      284 // numeric ENTER
#define SCAN_RCTRL      285
#define SCAN_ESCAPE       1
#define SCAN_RETURN      28
#define SCAN_PAUSE       69

#define CONFIGLENGTH     14

#define SAMPLES           7
#define CHANNELS         24 // this how many simultaneous sound effects we
                            // support; if we exceed this, the oldest
                            // sound is aborted to make room for the
                            // newest sound.

#define USR_NOTIFYICON (WM_USER + 1)
#define IDK_F12           0

EXPORT FLAG        iconified           = FALSE;
EXPORT int         inactive            = FALSE;
EXPORT ULONG       display[WINHEIGHT][WINWIDTH],
                   fgc,
                   starfield[MAPHEIGHT][WINWIDTH];
EXPORT const char  g_szClassName[]     = "Koules";

IMPORT struct KoulesObject object[MAXOBJECT];
IMPORT FLAG                paused;
IMPORT int                 controller[MAXROCKETS],
                           difficulty,
                           gamemode,
                           gameplan,
                           jx[4], jy[4],
                           startlevel,
                           level,
                           MouseButtons,
                           MouseX,
                           MouseY,
                           nrockets,
                           keys[3][4],
                           ssound,
                           stars,
                           volume;
IMPORT UBYTE               penter, pesc, pup, pdown, pleft, pright, ph,
                           amigan[16][16 + 1];

MODULE FLAG        ctrl                = FALSE,
                   lame                = FALSE;
MODULE int         joys                = 0,
                   showpointer         = TRUE,
                   titleheight,
                   writemode;
MODULE ULONG       SampleLength[SAMPLES];
MODULE UBYTE       pfires[4],
                   KeyMatrix[64];
MODULE char        ProgDir[MAX_PATH + 1];
MODULE HICON       BigIconPtr          = NULL,
                   SmallIconPtr        = NULL;
MODULE HINSTANCE   InstancePtr         = NULL;
MODULE HMENU       TrayMenuPtr         = NULL;
MODULE HWAVEOUT    hChannel[CHANNELS];
MODULE HWND        WindowPtr;
MODULE WAVEHDR     ChannelHdr[CHANNELS];

MODULE UBYTE       bitmapbuffer[sizeof(BITMAPINFO) + 16];
MODULE BITMAPINFO* BitmapHeaderPtr;
MODULE BOOL        ChannelOpened[CHANNELS],
                   ChannelPrepared[CHANNELS];
MODULE UBYTE*      SampleBuffer[SAMPLES];

#define MENUBITMAPS 1
MODULE struct
{   const DWORD   menuitem,
                  resource;
// uninitialized
          HBITMAP bitmap;
} mib[MENUBITMAPS] =
{ { ID_FILE_EXIT, IDB_M_EXIT },
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

MODULE void load_rc(void);
MODULE void save_rc(void);
MODULE void load_samples(void);
MODULE void make_stars(void);
MODULE void iconify(void);
MODULE void uniconify(FLAG exiting);
MODULE void traymenu(void);

EXPORT void usleep(unsigned long s)
{   TRANSIENT UBYTE first    = TRUE;
    PERSIST   ULONG waittill = 0;

    /* Wait for s microseconds.

       As soon as we have finished waiting, we calculate how much we
       should wait next time. This way the time consumed in running the
       game is taken into account.

       The way it is implemented, it will return immediately on the
       first frame, but all subsequent frames are timed correctly. */

    waittill += (s / 1000);

    if (timeGetTime() > waittill)
    {   waittill = timeGetTime();
    }
    while (timeGetTime() < waittill)
    {   Sleep(0);
}   }

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{   WNDCLASSEX    wc;
    JOYINFOEX     joyInfoEx;
    int           i, j;
    HMENU         SystemMenuPtr;
    OSVERSIONINFO Vers;

#ifdef USECONSOLE
    openconsole();
#endif

    for (i = 0; i < 64; i++)
    {   KeyMatrix[i] = 0;
    }
    for (i = 0; i < CHANNELS; i++)
    {   ChannelOpened[i] = ChannelPrepared[i] = FALSE;
    }
    for (i = 0; i < SAMPLES; i++)
    {   SampleBuffer[i] = NULL;
    }
    for (i = 0; i < MENUBITMAPS; i++)
    {   mib[i].bitmap  = NULL;
    }

    keys[0][0] = SCAN_W;
    keys[0][1] = SCAN_S;
    keys[0][2] = SCAN_A;
    keys[0][3] = SCAN_D;
    keys[1][0] = SCAN_I;
    keys[1][1] = SCAN_K;
    keys[1][2] = SCAN_J;
    keys[1][3] = SCAN_L;
    keys[2][0] = SCAN_UP;
    keys[2][1] = SCAN_DOWN;
    keys[2][2] = SCAN_LEFT;
    keys[2][3] = SCAN_RIGHT;

    // InitCommonControls(); not needed
    InstancePtr  = hInstance;

    titleheight  = GetSystemMetrics(SM_CYCAPTION)  // normally 27 (Win 8.1)
                 + GetSystemMetrics(SM_CYFRAME  ); // normally  4 (Win 8.1)
 // sidewidth    = GetSystemMetrics(SM_CXFRAME  ); // normally  4 (Win 8.1)
 // bottomheight = GetSystemMetrics(SM_CYFRAME  ); // normally  4 (Win 8.1)

    Vers.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx((LPOSVERSIONINFO) &Vers);
    if
    (   Vers.dwPlatformId == VER_PLATFORM_WIN32_NT
     || (   Vers.dwPlatformId != VER_PLATFORM_WIN32s
         && Vers.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS
    )   )
    {   lame = TRUE;
    }

    SmallIconPtr = LoadImage(InstancePtr, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);
    BigIconPtr   = LoadImage(InstancePtr, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 32, 32, 0);

    DISCARD GetCurrentDirectory(MAX_PATH, ProgDir);
    if (ProgDir[0])
    {   DISCARD SetCurrentDirectory(ProgDir);
    }

    load_rc();
    load_samples();

    srand((unsigned int) time(NULL));

    make_stars();

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = InstancePtr;
    wc.hIcon         = BigIconPtr;
    wc.hIconSm       = SmallIconPtr;
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
    wc.hbrBackground = (HBRUSH) (COLOR_GRAYTEXT + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;

    if (!RegisterClassEx(&wc))
    {   MessageBox
        (   NULL,
            "Window registration failed!",
            "Koules: Error",
            MB_ICONEXCLAMATION | MB_OK
        );
        cleanexit(EXIT_FAILURE);
    }

    if (!(WindowPtr = CreateWindowEx
    (   0,
        g_szClassName,
        "Koules",
        WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (WINWIDTH  / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (WINHEIGHT / 2),
        WINWIDTH                + 6, // WINWIDTH  + (sidewidth * 2) should work but doesn't
        WINHEIGHT + titleheight - 2, // WINHEIGHT + titleheight + bottomheight should work but doesn't
        NULL,
        NULL,
        InstancePtr,
        NULL
    )))
    {   MessageBox
        (   NULL,
            "Window creation failed!",
            "Koules: Error",
            MB_ICONEXCLAMATION | MB_OK
        );
        cleanexit(EXIT_FAILURE);
    }

    SystemMenuPtr = GetSystemMenu(WindowPtr, FALSE);
    if (SystemMenuPtr)
    {   DISCARD InsertMenu(SystemMenuPtr, 6, MF_BYPOSITION | MF_STRING, ID_FILE_ICONIFY, "&Iconify\tCtrl+F12");
    }

    ShowWindow(WindowPtr, SW_SHOW);
    UpdateWindow(WindowPtr);

    if (!showpointer)
    {   ShowCursor(FALSE);
    }

    j = joyGetNumDevs();
    for (i = 0; i < j; i++)
    {   /* DISCARD joyGetDevCaps(i, &JoyCaps, sizeof(JOYCAPS));
        printf("Joystick %d has %d buttons.\n", i, JoyCaps.wNumButtons); */
 
        joyInfoEx.dwSize  = sizeof(joyInfoEx);
        joyInfoEx.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(i, &joyInfoEx) == JOYERR_NOERROR)
        {   joys++;
    }   }

    for (i = 0; i < sizeof(BITMAPINFOHEADER) + (4 * 4); i++)
    {   bitmapbuffer[i] = 0;
    }
    BitmapHeaderPtr = (BITMAPINFO*) &bitmapbuffer;
    BitmapHeaderPtr->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BitmapHeaderPtr->bmiHeader.biPlanes = 1;
    BitmapHeaderPtr->bmiHeader.biWidth = WINWIDTH;
    BitmapHeaderPtr->bmiHeader.biHeight = -WINHEIGHT; // note well
    BitmapHeaderPtr->bmiHeader.biBitCount = 32;
    BitmapHeaderPtr->bmiHeader.biCompression = BI_BITFIELDS;
    ((unsigned long*) BitmapHeaderPtr->bmiColors)[0] = 0x00FF0000;
    ((unsigned long*) BitmapHeaderPtr->bmiColors)[1] = 0x0000FF00;
    ((unsigned long*) BitmapHeaderPtr->bmiColors)[2] = 0x000000FF;

    for (i = 0; i < MENUBITMAPS; i++)
    {   mib[i].bitmap = LoadBitmap(InstancePtr, MAKEINTRESOURCE(mib[i].resource));
    }

    create_bitmaps();
    make_logo();
    clearscreen();
    draw_logo();
    init_objects();
    game();
    cleanexit(EXIT_SUCCESS);

    return EXIT_SUCCESS; // never executed, but Visual C gets angry if it is missing
}

EXPORT FLAG UpdateInput(void)
{   TRANSIENT MSG   Msg;
    TRANSIENT ULONG scancode;
    TRANSIENT FLAG  rc      = FALSE;
    TRANSIENT int   i;
    PERSIST   FLAG  shifted = FALSE;

    while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
    {   switch (Msg.message)
        {
        case WM_KEYDOWN:
            scancode = (Msg.lParam & 33488896) >> 16;
            if (!(Msg.lParam & 0x40000000)) // ie. if not a repeated key
            {   rc = TRUE;
                KeyMatrix[scancode / 8] |= (1 << (scancode % 8));
                switch (scancode)
                {
                case SCAN_LSHIFT:
                case SCAN_RSHIFT:
                    shifted = TRUE;
                acase SCAN_RETURN:
                case SCAN_ENTER:
                    penter = 1;
                acase SCAN_ESCAPE:
                    if (gamemode == MENU || shifted)
                    {   cleanexit(EXIT_SUCCESS);
                    } // implied else
                    pesc = 1;
                    paused = FALSE;
                acase SCAN_UP:
                    pup = 1;
                acase SCAN_DOWN:
                    pdown = 1;
                acase SCAN_LEFT:
                    pleft = 1;
                acase SCAN_RIGHT:
                    pright = 1;
                acase SCAN_H:
                    ph = 1;
                acase SCAN_P:
                case SCAN_PAUSE:
                    if (paused)
                    {   paused = rc = FALSE;
                        if (gamemode == GAME)
                        {   for (i = 0; i < nrockets; i++)
                            {   if (controller[i] == MOUSE)
                                {   confine();
                                    break; // for speed
                        }   }   }
                    } else
                    {   paused = TRUE;
                        unconfine();
                        DrawWhiteText((MAPWIDTH / 2) - (4 * 6), 140, "PAUSED");
                        updatescreen();
                    }
                acase SCAN_X:
                    checkpointer();
                acase SCAN_LCTRL:
                case SCAN_RCTRL:
                    ctrl = TRUE;
                acase SCAN_F12:
                    if (ctrl)
                    {   iconify();
            }   }   }
        acase WM_KEYUP:
            scancode = (Msg.lParam & 33488896) >> 16;
            KeyMatrix[scancode / 8] &= (255 - (1 << (scancode % 8)));
            switch (scancode)
            {
            case SCAN_LSHIFT:
            case SCAN_RSHIFT:
                shifted = FALSE;
            acase SCAN_RETURN:
            case SCAN_ENTER:
                penter = 0;
            acase SCAN_ESCAPE:
                pesc = 0;
            acase SCAN_UP:
                pup = 0;
            acase SCAN_DOWN:
                pdown = 0;
            acase SCAN_LEFT:
                pleft = 0;
            acase SCAN_RIGHT:
                pright = 0;
            acase SCAN_H:
                ph = 0;
            acase SCAN_X:
                checkpointer();
            acase SCAN_LCTRL:
            case SCAN_RCTRL:
                ctrl = FALSE;
            }
        adefault:
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
    }   }

    ReadJoystick(0);
    ReadJoystick(1);
    ReadJoystick(2);
    ReadJoystick(3);
    if (pfires[0] || pfires[1] || pfires[2] || pfires[3] || MouseButtons)
    {   rc = TRUE;
    }

    return rc;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{   PAINTSTRUCT ps;

    switch (msg)
    {   case WM_PAINT:
            BeginPaint(hwnd, &ps);
            updatescreen(); // (not really needed unless paused)
            EndPaint(hwnd, &ps);
        acase WM_ACTIVATE:
            switch (LOWORD(wParam))
            {
            case WA_ACTIVE:
            case WA_CLICKACTIVE:
                inactive = FALSE;
            acase WA_INACTIVE:
                inactive = TRUE;
                if (gamemode == GAME)
                {   paused = TRUE;
                    unconfine();
                    DrawWhiteText((MAPWIDTH / 2) - (4 * 6), 140, "PAUSED");
                    updatescreen();
            }   }
        acase WM_MOUSEACTIVATE:
            inactive = FALSE;
            return MA_ACTIVATEANDEAT;
        acase WM_CLOSE:
            cleanexit(EXIT_SUCCESS);
        acase WM_DESTROY:
            PostQuitMessage(0);
        acase WM_MOUSEMOVE:
            MouseX = LOWORD(lParam);
            MouseY = HIWORD(lParam);
        acase WM_LBUTTONDOWN:
            if (!inactive)
            {   MouseButtons = 1;
            }
        acase WM_LBUTTONUP:
            MouseButtons = 0;
        acase USR_NOTIFYICON:
            // assert(iconified);
            switch (lParam)
            {
            case WM_LBUTTONDBLCLK:
                uniconify(FALSE);
            acase WM_RBUTTONUP:
                traymenu();
            }
            return 0;
        acase WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case ID_FILE_UNICONIFY:
                uniconify(FALSE);
            acase ID_FILE_EXIT:
                cleanexit(EXIT_SUCCESS);
            }
            return 0;
        acase WM_SYSCOMMAND:
            switch (LOWORD(wParam))
            {
            case ID_FILE_ICONIFY:
                iconify();
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        adefault:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void updatescreen(void)
{   HDC  /* hdcMem, */
            OnScreenRastPort;
 // HBITMAP OldBitmapPtr;

    OnScreenRastPort = GetDC(WindowPtr);
    DISCARD StretchDIBits
    (   OnScreenRastPort,
        0,               // dest leftx
        0,               // dest topy
        WINWIDTH,        // dest width
        WINHEIGHT,       // dest height
        0,               // source leftx
        0,               // source topy
        WINWIDTH,        // source width
        WINHEIGHT,       // source height
        &display[0][0],  // pointer to the bits
        BitmapHeaderPtr, // pointer to BITMAPINFO structure
        DIB_RGB_COLORS,  // format of data
        SRCCOPY          // blit mode
    );
    DISCARD ReleaseDC(WindowPtr, OnScreenRastPort);
}

BOOL IsPressed(int which)
{   if (KeyMatrix[which / 8] & (1 << (which % 8)) && !ctrl)
    {    return TRUE;
    } else
    {    return FALSE;
}   }

EXPORT int ReadJoystick(int joynum)
{   JOYINFOEX JoyInfoEx;
    int       s = 0;

    pfires[joynum] = 0;
    jx[joynum]     =
    jy[joynum]     = 0;
    
    if (joynum >= joys)
    {   return 0;
    }

    ZeroMemory(&JoyInfoEx, sizeof(JoyInfoEx));
    JoyInfoEx.dwSize  = sizeof(JoyInfoEx);
    JoyInfoEx.dwFlags = JOY_RETURNALL;

    if (joyGetPosEx(joynum, &JoyInfoEx) == JOYERR_NOERROR)
    {   if (JoyInfoEx.dwButtons)
        {   pfires[joynum] = 1;
        } else
        {   pfires[joynum] = 0;
        }

        jx[joynum] = -((signed int) (JoyInfoEx.dwXpos - 32768));
        jy[joynum] = -((signed int) (JoyInfoEx.dwYpos - 32768));

        if (    JoyInfoEx.dwXpos < (16 * 1024) && JoyInfoEx.dwYpos < (16 * 1024))
        {   s = 1; // up-left
        } elif (JoyInfoEx.dwXpos > (48 * 1024) && JoyInfoEx.dwYpos < (16 * 1024))
        {   s = 2; // up-right
        } elif (JoyInfoEx.dwXpos > (48 * 1024) && JoyInfoEx.dwYpos > (48 * 1024))
        {   s = 3; // down-right
        } elif (JoyInfoEx.dwXpos < (16 * 1024) && JoyInfoEx.dwYpos > (48 * 1024))
        {   s = 4; // down-left
        } elif (JoyInfoEx.dwXpos < (16 * 1024))
        {   s = 5; // left
        } elif (JoyInfoEx.dwXpos > (48 * 1024))
        {   s = 6; // right
        } elif (JoyInfoEx.dwYpos < (16 * 1024))
        {   s = 7; // up
        } elif (JoyInfoEx.dwYpos > (48 * 1024))
        {   s = 8; // down
    }   }

    return s;
}

MODULE void load_rc(void)
{   FILE* LocalHandle;
    UBYTE ConfigBuffer[CONFIGLENGTH];
    int   i;

    if ((LocalHandle = fopen("Koules.cfg", "rb")))
    {   if (fread(ConfigBuffer, CONFIGLENGTH, 1, LocalHandle) == 1)
        {   // byte 0: version
            if (ConfigBuffer[0] >= 6) // 6 means R1.16+
            {   for (i = 0; i <= 4; i++)
                {   controller[i] = (int) ConfigBuffer[i + 1];
                }
                nrockets    = (int) ConfigBuffer[6];
                if (ConfigBuffer[0] >= 5)
                {   gameplan    = (int) ConfigBuffer[7];
                }
                difficulty  = (int) ConfigBuffer[8];
                ssound      = (int) ConfigBuffer[9];
                level       = (int) ConfigBuffer[10];
                showpointer = (int) ConfigBuffer[11];
                volume      = (int) ConfigBuffer[12];
                stars       = (int) ConfigBuffer[13];
        }   }
        DISCARD fclose(LocalHandle);
        // LocalHandle = NULL;
}   }

MODULE void save_rc(void)
{   FILE* LocalHandle;
    UBYTE ConfigBuffer[CONFIGLENGTH];
    int   i;

    ConfigBuffer[ 0] = 6; // 6 means R1.16+
    for (i = 0; i <= 4; i++)
    {   ConfigBuffer[i + 1] = (UBYTE) controller[i];
    }
    ConfigBuffer[ 6] = (UBYTE) nrockets;
    ConfigBuffer[ 7] = (UBYTE) gameplan;
    ConfigBuffer[ 8] = (UBYTE) difficulty;
    ConfigBuffer[ 9] = (UBYTE) ssound;
    ConfigBuffer[10] = (UBYTE) startlevel;
    ConfigBuffer[11] = (UBYTE) showpointer;
    ConfigBuffer[12] = (UBYTE) volume;
    ConfigBuffer[13] = (UBYTE) stars;

    if ((LocalHandle = fopen("Koules.cfg", "wb")))
    {   DISCARD fwrite(ConfigBuffer, CONFIGLENGTH, 1, LocalHandle);
        DISCARD fclose(LocalHandle);
        // LocalHandle = NULL;
}   }

EXPORT void cleanexit(SBYTE rc)
{   int i;

    stop_sounds();

    for (i = 0; i < CHANNELS; i++)
    {   if (ChannelOpened[i])
        {   DISCARD waveOutClose(hChannel[i]);
            ChannelOpened[i] = FALSE;
    }   }

    for (i = 0; i < SAMPLES; i++)
    {   if (SampleBuffer[i])
        {   free(SampleBuffer[i]);
            // SampleBuffer[i] = NULL;
    }   }

    if (iconified)
    {   uniconify(TRUE); // to get rid of tray icon
    }
    if (WindowPtr)
    {   DestroyWindow(WindowPtr);
        // WindowPtr = NULL;
    }

    for (i = 0; i < MENUBITMAPS; i++)
    {   if (mib[i].bitmap)
        {   DeleteObject(mib[i].bitmap);
            mib[i].bitmap = NULL;
    }   }

 /* DestroyIcon(BigIconPtr);
    BigIconPtr = NULL;
    DestroyIcon(SmallIconPtr);
    SmallIconPtr = NULL; */
 
    save_rc();

    exit(rc);
}

MODULE ULONG getsize(STRPTR filename)
{   HANDLE hFile /* = NULL */ ;
    ULONG  size;

    hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {   return 0;
    }
    size = GetFileSize(hFile, NULL);
    CloseHandle(hFile);
    // hFile = NULL;

    if (size == (ULONG) -1)
    {   return 0;
    }

    return size;
}

MODULE void load_samples(void)
{   TRANSIENT       FILE*        LocalHandle;
    TRANSIENT       WAVEFORMATEX l_WaveFormatEx;
    TRANSIENT       int          i;
    PERSIST   const STRPTR       SampleName[SAMPLES] =
    {   "sounds\\start.wav",
        "sounds\\end.wav",
        "sounds\\collide.wav",
        "sounds\\destroy1.wav",
        "sounds\\destroy2.wav",
        "sounds\\creator1.wav",
        "sounds\\creator2.wav"
    };

    for (i = 0; i < SAMPLES; i++)
    {   if
        (   ((SampleLength[i] = getsize(SampleName[i])))
         && ((LocalHandle = fopen(SampleName[i], "rb")))
        )
        {   SampleBuffer[i] = malloc(SampleLength[i]); // should really check return code
            DISCARD fread(SampleBuffer[i], SampleLength[i], 1, LocalHandle); // should really check return code
            DISCARD fclose(LocalHandle);
            // LocalHandle = NULL;
    }   }

    for (i = 0; i < CHANNELS; i++)
    {   l_WaveFormatEx.wFormatTag      = WAVE_FORMAT_PCM;
        l_WaveFormatEx.nChannels       = 1;
        l_WaveFormatEx.nSamplesPerSec  = 11025;
        l_WaveFormatEx.wBitsPerSample  = 8;
        l_WaveFormatEx.nAvgBytesPerSec = 11025;
        l_WaveFormatEx.nBlockAlign     = 1;
     // l_WaveFormatEx.cbSize          = 0; ignored
        memset(&hChannel[i], 0, sizeof(HWAVEOUT));
        DISCARD waveOutOpen
        (   &hChannel[i],
            WAVE_MAPPER,
            &l_WaveFormatEx,
            0,
            0,
            CALLBACK_NULL
        );
        ChannelOpened[i] = TRUE;
}   }

EXPORT void play_sound(int sample)
{   PERSIST int channel = CHANNELS - 1;

    if
    (   !ssound
     || !SampleBuffer[sample]
    )
    {   return;
    }

    if (channel == CHANNELS - 1)
    {   channel = 0;
    } else
    {   channel++;
    }
    if (!ChannelOpened[channel])
    {   return;
    }
    if (ChannelPrepared[channel])
    {   DISCARD waveOutReset(hChannel[channel]);
        DISCARD waveOutUnprepareHeader(hChannel[channel], &ChannelHdr[channel], sizeof(WAVEHDR));
        ChannelPrepared[channel] = FALSE;
    }
              
    ChannelHdr[channel].lpData          = SampleBuffer[sample] + 44;
    ChannelHdr[channel].dwBufferLength  = SampleLength[sample] - 44;
    ChannelHdr[channel].dwBytesRecorded = 0;
    ChannelHdr[channel].dwUser          = 0;
    ChannelHdr[channel].dwFlags         = 0;
    ChannelHdr[channel].dwLoops         = 1;
    ChannelHdr[channel].lpNext          = NULL;
    ChannelHdr[channel].reserved        = (unsigned long) NULL;
    DISCARD waveOutPrepareHeader(hChannel[channel], &ChannelHdr[channel], sizeof(WAVEHDR));
    ChannelPrepared[channel] = TRUE;

    ChannelHdr[channel].dwFlags        |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
    DISCARD waveOutWrite(hChannel[channel], &ChannelHdr[channel], sizeof(WAVEHDR));
}

EXPORT void stop_sounds(void)
{   int i;

    for (i = 0; i < CHANNELS; i++)
    {   if (ChannelPrepared[i])
        {   DISCARD waveOutReset(hChannel[i]);
            DISCARD waveOutUnprepareHeader(hChannel[i], &ChannelHdr[i], sizeof(WAVEHDR));
            ChannelPrepared[i] = FALSE;
}   }   }

EXPORT void checksound(void)
{   PERSIST int fresh = TRUE;

    if
    (   (KeyMatrix[SCAN_S / 8] & (1 << (SCAN_S % 8)))
     && ctrl
    )
    {   if (fresh)
        {   ssound = !ssound;
            if (!ssound)
            {   stop_sounds();
            }
            main_menu(); // so that "SOUND ON" or "SOUND OFF" is correct. Necessary even during play
            fresh = FALSE;
    }   }
    else
    {   fresh = TRUE;
}   }

EXPORT void checkpointer(void)
{   PERSIST int fresh = TRUE;

    if
    (   (KeyMatrix[SCAN_X / 8] & (1 << (SCAN_X % 8)))
     && ctrl
    )
    {   if (fresh)
        {   if (showpointer)
            {   showpointer = FALSE;
            } else
            {   showpointer = TRUE;
            }
            ShowCursor(showpointer);
            fresh = FALSE;
    }   }
    else
    {   fresh = TRUE;
}   }

EXPORT void checktitlebar(void) { ; }

EXPORT void clearkybd(void)
{   MSG Message;

    while ((PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)));
}

#ifdef USECONSOLE
EXPORT void openconsole(void)
{   int   hScrn, hKybd;
    FILE* hf;

    // assert(!consoleopen);

    DISCARD AllocConsole();
    hScrn = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT); // might need freeing at program exit
    hKybd = _open_osfhandle((long) GetStdHandle(STD_INPUT_HANDLE ), _O_TEXT); // might need freeing at program exit
    hf = _fdopen(hScrn, "w"); // might need freeing at program exit
    *stdout = *hf;
    hf = _fdopen(hKybd, "r"); // might need freeing at program exit
    *stdin = *hf;
    DISCARD setvbuf(stdout, NULL, _IONBF, 0);
    DISCARD setvbuf(stdin , NULL, _IONBF, 0);

    DISCARD SetConsoleTitle("WinArcadia Console");
    Sleep(40); // to ensure the window title was updated
}
#endif

MODULE void make_stars(void)
{   int i, j, x, y;

#define STARDENSITY 256
// lower numbers mean denser stars

    for (y = 0; y < MAPHEIGHT; y++)
    {   for (x = 0; x < WINWIDTH; x++)
        {   starfield[y][x] = 0;
    }   }

    j = (WINWIDTH * MAPHEIGHT) / STARDENSITY;
    for (i = 0; i < j; i++)
    {   x = rand() % WINWIDTH;
        y = rand() % MAPHEIGHT;
        starfield[y][x] = ((rand() % 255) << 16)
                        + ((rand() % 255) <<  8)
                        +  (rand() % 255); 
}   }

EXPORT void confine(void)
{   POINT ThePoint;
    RECT  TheRect;

    ThePoint.x = 0;
    ThePoint.y = 0;
    DISCARD ClientToScreen(WindowPtr, &ThePoint);
    TheRect.left   = ThePoint.x;
    TheRect.right  = ThePoint.x + MAPWIDTH  - 5;
    TheRect.top    = ThePoint.y;
    TheRect.bottom = ThePoint.y + MAPHEIGHT;
    DISCARD ClipCursor(&TheRect);
}

EXPORT void unconfine(void)
{   DISCARD ClipCursor(NULL);
}

MODULE void iconify(void)
{   NOTIFYICONDATA tnid;
    BOOL           pvParam;

    if (iconified)
    {   return;
    }

    tnid.cbSize           = sizeof(NOTIFYICONDATA);
    tnid.hWnd             = WindowPtr;
    tnid.uID              = 1;
    tnid.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tnid.uCallbackMessage = USR_NOTIFYICON;
    tnid.hIcon            = SmallIconPtr;
    strcpy(tnid.szTip, "Koules");
    DISCARD Shell_NotifyIcon(NIM_ADD, &tnid);

    ShowWindow(WindowPtr, SW_HIDE);

    if (lame)
    {   SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &pvParam, 0);
        if (pvParam == FALSE)
        {   SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDWININICHANGE);
    }   }

    iconified = TRUE;
}

MODULE void uniconify(FLAG exiting)
{   int            i;
    NOTIFYICONDATA tnid;
    BOOL           pvParam;

    // assert(iconified);

    if (!exiting)
    {   ShowWindow(WindowPtr, SW_SHOW);
        SetActiveWindow(WindowPtr);
    }

    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd   = WindowPtr;
    tnid.uID    = 1;
    DISCARD Shell_NotifyIcon(NIM_DELETE, &tnid);
    DISCARD DestroyMenu(TrayMenuPtr);
    TrayMenuPtr = NULL;

    if (lame)
    {   SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &pvParam, 0);
        if (pvParam == TRUE)
        {   SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, SPIF_SENDWININICHANGE);
    }   }

    ctrl      =
    iconified = FALSE;

    if (gamemode == GAME && !paused)
    {   for (i = 0; i < nrockets; i++)
        {   if (controller[i] == MOUSE)
            {   confine();
                break; // for speed
}   }   }   }

MODULE void traymenu(void)
{   int          i;
    HMENU        TraySubMenuPtr;
    POINT        ThePoint;
    MENUITEMINFO mii =
    {   44,
        MIIM_STATE,
        0,
        MFS_DEFAULT, // to embolden it
        ID_FILE_ICONIFY,
        NULL,
        NULL,
        NULL,
        ID_FILE_ICONIFY,
        0,
        0
    };

    // assert(iconified);

    DISCARD SetForegroundWindow(WindowPtr);
    DISCARD GetCursorPos(&ThePoint);

    TrayMenuPtr = LoadMenu(InstancePtr, MAKEINTRESOURCE(IDR_TRAYMENU));
    for (i = 0; i < MENUBITMAPS; i++)
    {   DISCARD SetMenuItemBitmaps
        (   TrayMenuPtr,     // handle of menu
            mib[i].menuitem, // menu item to receive new bitmaps
            MF_BYCOMMAND,    // menu item flags
            mib[i].bitmap,   // handle of unchecked bitmap
            NULL             // handle of checked bitmap
        );
    }
    TraySubMenuPtr = GetSubMenu(TrayMenuPtr, 0);
    DISCARD SetMenuItemInfo
    (   TraySubMenuPtr,
        ID_FILE_UNICONIFY,
        FALSE,
        &mii
    );
    TrackPopupMenu
    (   TraySubMenuPtr,
        TPM_RIGHTALIGN | TPM_RIGHTBUTTON,
        ThePoint.x,
        ThePoint.y,
        0,
        WindowPtr,
        NULL
    );
    PostMessage(WindowPtr, WM_NULL, 0, 0);
}
