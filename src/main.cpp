// main.cpp: initialisation & main loop

#include "cube.h"

void cleanup(char *msg)         // single program exit point;
{
	stop();
    disconnect(true);
    writecfg();
    cleangl();
    cleansound();
    cleanupserver();
    SDL_ShowCursor(1);
    if(msg)
    {
        #ifdef WIN32
        MessageBox(NULL, msg, "cube fatal error", MB_OK|MB_SYSTEMMODAL);
        #else
        printf(msg);
        #endif
    };
    SDL_Quit();
    exit(1);
};

void quit()                     // normal exit
{
    writeservercfg();
    cleanup(NULL);
};

void fatal(char *s, char *o)    // failure exit
{
    sprintf_sd(msg)("%s%s (%s)\n", s, o, SDL_GetError());
    cleanup(msg);
};

void *alloc(int s)              // for some big chunks... most other allocs use the memory pool
{
    void *b = calloc(1,s);
    if(!b) fatal("out of memory!");
    return b;
};

int scr_w = 640;
int scr_h = 480;
SDL_Window *screen = NULL;

// The HUD is laid out on a 2400x1800 (4:3) virtual screen. On a wider screen it gets wider, on a
// narrower one taller, so it fills the screen without stretching; the elements at the bottom
// and at the right move with the edges (renderextras.cpp).
int VIRTW = 2400, VIRTH = 1800;

void setvirtual()
{
    if(scr_w*3 >= scr_h*4) { VIRTH = 1800; VIRTW = 1800*scr_w/scr_h; }
    else { VIRTW = 2400; VIRTH = 2400*scr_h/scr_w; };
};

void screenshot()
{
    SDL_Surface *image;
    SDL_Surface *temp;
    int idx;
    if(image = SDL_CreateRGBSurface(SDL_SWSURFACE, scr_w, scr_h, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0))
    {
        if(temp  = SDL_CreateRGBSurface(SDL_SWSURFACE, scr_w, scr_h, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0))
        {
            glReadPixels(0, 0, scr_w, scr_h, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
            for (idx = 0; idx<scr_h; idx++)
            {
                char *dest = (char *)temp->pixels+3*scr_w*idx;
                memcpy(dest, (char *)image->pixels+3*scr_w*(scr_h-1-idx), 3*scr_w);
                endianswap(dest, 3, scr_w);
            };
            sprintf_sd(buf)("screenshots/screenshot_%d.bmp", lastmillis);
            SDL_SaveBMP(temp, path(buf));
            SDL_FreeSurface(temp);
        };
        SDL_FreeSurface(image);
    };
};

COMMAND(screenshot, ARG_NONE);
COMMAND(quit, ARG_NONE);

bool keyrepeating = false;               // SDL2 always repeats; repeats are dropped unless on

void keyrepeat(bool on)
{
    keyrepeating = on;
};

// SDL2 key codes to the SDL 1.2 numbers keymap.cfg uses (printable keys are the same)
int sdl1key(SDL_Keycode k)
{
    if(k<256) return k;
    if(k>=SDLK_F1 && k<=SDLK_F12) return 282+(k-SDLK_F1);
    if(k>=SDLK_KP_1 && k<=SDLK_KP_9) return 257+(k-SDLK_KP_1);
    switch(k)
    {
        case SDLK_UP: return 273; case SDLK_DOWN: return 274; case SDLK_RIGHT: return 275; case SDLK_LEFT: return 276;
        case SDLK_INSERT: return 277; case SDLK_HOME: return 278; case SDLK_END: return 279;
        case SDLK_PAGEUP: return 280; case SDLK_PAGEDOWN: return 281;
        case SDLK_KP_0: return 256; case SDLK_KP_PERIOD: return 266; case SDLK_KP_DIVIDE: return 267;
        case SDLK_KP_MULTIPLY: return 268; case SDLK_KP_MINUS: return 269; case SDLK_KP_PLUS: return 270;
        case SDLK_KP_ENTER: return 271; case SDLK_KP_EQUALS: return 272;
        case SDLK_NUMLOCKCLEAR: return 300; case SDLK_CAPSLOCK: return 301; case SDLK_SCROLLLOCK: return 302;
        case SDLK_RSHIFT: return 303; case SDLK_LSHIFT: return 304; case SDLK_RCTRL: return 305; case SDLK_LCTRL: return 306;
        case SDLK_RALT: return 307; case SDLK_LALT: return 308; case SDLK_RGUI: return 309; case SDLK_LGUI: return 310;
        case SDLK_MODE: return 313; case SDLK_HELP: return 315; case SDLK_PRINTSCREEN: return 316;
        case SDLK_SYSREQ: return 317; case SDLK_MENU: return 319; case SDLK_PAUSE: return 19;
    };
    return 0;
};

VARF(gamespeed, 10, 100, 1000, if(multiplayer()) gamespeed = 100);
VARP(minmillis, 0, 5, 1000);

int islittleendian = 1;
int framesinmap = 0;

int main(int argc, char **argv)
{
    bool dedicated = false;
    int fs = SDL_WINDOW_FULLSCREEN_DESKTOP, par = 0, uprate = 0, maxcl = 4;
    char *sdesc = "", *ip = "", *master = NULL, *passwd = "";
    islittleendian = *((char *)&islittleendian);

    #define log(s) conoutf("init: %s", s)
    log("sdl");

    for(int i = 1; i<argc; i++)
    {
        char *a = &argv[i][2];
        if(argv[i][0]=='-') switch(argv[i][1])
        {
            case 'd': dedicated = true; break;
            case 't': fs     = 0; break;
            case 'w': scr_w  = atoi(a); break;
            case 'h': scr_h  = atoi(a); break;
            case 'u': uprate = atoi(a); break;
            case 'n': sdesc  = a; break;
            case 'i': ip     = a; break;
            case 'm': master = a; break;
            case 'p': passwd = a; break;
            case 'c': maxcl  = atoi(a); break;
            default:  conoutf("unknown commandline option");
        }
        else conoutf("unknown commandline argument");
    };

    #ifdef _DEBUG
    par = SDL_INIT_NOPARACHUTE;
    fs = 0;
    #endif

    if(SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|par)<0) fatal("Unable to initialize SDL");

    log("net");
    if(enet_initialize()<0) fatal("Unable to initialise network module");

    initclient();
    initserver(dedicated, uprate, sdesc, ip, master, passwd, maxcl);  // never returns if dedicated

    log("world");
    empty_world(7, true);

    log("video: sdl");
    if(SDL_InitSubSystem(SDL_INIT_VIDEO)<0) fatal("Unable to initialize SDL Video");

    log("video: mode");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_DisplayMode dm;
    if(fs && !SDL_GetDesktopDisplayMode(0, &dm)) { scr_w = dm.w; scr_h = dm.h; };
    screen = SDL_CreateWindow("cube engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, scr_w, scr_h, SDL_WINDOW_OPENGL|fs);
    if(!screen || !SDL_GL_CreateContext(screen)) fatal("Unable to create OpenGL screen");
    SDL_GL_GetDrawableSize(screen, &scr_w, &scr_h);     // fullscreen: the display's own size
    setvirtual();

    log("video: misc");
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_StopTextInput();
    keyrepeat(false);
    SDL_ShowCursor(0);

    log("gl");
    gl1_init();
    gl_init(scr_w, scr_h);

    log("basetex");
    int xs, ys;
    if(!installtex(2,  path(newstring("data/newchars.png")), xs, ys) ||
       !installtex(3,  path(newstring("data/martin/base.png")), xs, ys) ||
       !installtex(6,  path(newstring("data/martin/ball1.png")), xs, ys) ||
       !installtex(7,  path(newstring("data/martin/smoke.png")), xs, ys) ||
       !installtex(8,  path(newstring("data/martin/ball2.png")), xs, ys) ||
       !installtex(9,  path(newstring("data/martin/ball3.png")), xs, ys) ||
       !installtex(4,  path(newstring("data/explosion.jpg")), xs, ys) ||
       !installtex(5,  path(newstring("data/items.png")), xs, ys) ||
       !installtex(1,  path(newstring("data/crosshair.png")), xs, ys)) fatal("could not find core textures (hint: run cube from the parent of the bin directory)");

    log("sound");
    initsound();

    log("cfg");
    newmenu("frags\tpj\tping\tteam\tname");
    newmenu("ping\tplr\tserver");
    exec("data/keymap.cfg");
    exec("data/menus.cfg");
    exec("data/prefabs.cfg");
    exec("data/sounds.cfg");
    exec("servers.cfg");
    if(!execfile("config.cfg")) execfile("data/defaults.cfg");
    exec("autoexec.cfg");

    log("localconnect");
    localconnect();
    changemap("metl3");		// if this map is changed, also change depthcorrect()

    log("mainloop");
    int ignore = 5;
    for(;;)
    {
        int millis = SDL_GetTicks()*gamespeed/100;
        if(millis-lastmillis>200) lastmillis = millis-200;
        else if(millis-lastmillis<1) lastmillis = millis-1;
        if(millis-lastmillis<minmillis) SDL_Delay(minmillis-(millis-lastmillis));
        cleardlights();
        updateworld(millis);
        if(!demoplayback) serverslice((int)time(NULL), 0);
        static float fps = 30.0f;
        fps = (1000.0f/curtime+fps*50)/51;
        computeraytable(player1->o.x, player1->o.y);
        readdepth(scr_w, scr_h);
        SDL_GL_SwapWindow(screen);
        extern void updatevol(); updatevol();
        if(framesinmap++<5)	// cheap hack to get rid of initial sparklies, even when triple buffering etc.
        {
			player1->yaw += 5;
			gl_drawframe(scr_w, scr_h, fps);
			player1->yaw -= 5;
        };
        gl_drawframe(scr_w, scr_h, fps);
        SDL_Event event;
        int lasttype = 0, lastbut = 0;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit();
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    if(event.key.repeat && !keyrepeating) break;
                    keypress(sdl1key(event.key.keysym.sym), event.key.state==SDL_PRESSED, 0);
                    break;

                case SDL_TEXTINPUT:                     // typed characters for the command line
                    for(char *c = event.text.text; *c; c++) if(*c>=32 && *c<127) keypress(0, true, *c);
                    break;

                case SDL_MOUSEWHEEL:                    // SDL 1.2 reported the wheel as buttons 4 and 5
                    if(event.wheel.y)
                    {
                        int b = event.wheel.y>0 ? -4 : -5;
                        keypress(b, true, 0);
                        keypress(b, false, 0);
                    };
                    break;

                case SDL_MOUSEMOTION:
                    if(ignore) { ignore--; break; };
                    mousemove(event.motion.xrel, event.motion.yrel);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    if(lasttype==event.type && lastbut==event.button.button) break; // why?? get event twice without it
                    keypress(-event.button.button, event.button.state!=0, 0);
                    lasttype = event.type;
                    lastbut = event.button.button;
                    break;
            };
        };
    };
    quit();
    return 1;
};


