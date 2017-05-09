/* 
	sdlvnc SDL Based VNC Client - Zonyl

	- Based code from on   SDL_vnc.c - VNC client implementation  LGPL (c) A. Schiffler, aschiffler@appwares.com
        - Uses SDL_vnc lib and SDL_image for parameter fetching

*/

#ifdef WIN32
 #include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <SDL/SDL.h>
#ifdef USE_TTF_SPLASH
#include <SDL/SDL_ttf.h>
#endif
#include "SDL_vnc.h"

// Enable rotozoom (experimental!)
#define ALLOW_ZOOMING
#ifdef ALLOW_ZOOMING
#include "SDL/SDL_rotozoom.h"
#endif

#define DEFAULT_W	320
//#define DEFAULT_H	480
#define DEFAULT_H	240
#define VERSION		"1.06"
//Ubuntu
//#define FONT		"/usr/share/fonts/truetype/msttcorefonts/Arial.ttf"
//Palm Pre
//#define FONT		"/usr/share/fonts/PreludeCondensed-Medium.ttf"
#define FONT		"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf"



#define false		-1

/* Commandline configurable items */

char *vnc_server = NULL;
int   vnc_display = 0;
int   vnc_port = 5900;
char *vnc_method = NULL;
char *vnc_password = NULL;
int   vnc_framerate = 12;
int   vnc_mbswap = 0;

#ifdef USE_TTF_SPLASH
void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, NULL, destination, &offset );
}

int SplashScreen(SDL_Surface *screen)
{
	TTF_Font *font = NULL;
	SDL_Surface *title = NULL;

	//The color of the font 
	SDL_Color textColor = { 255, 255, 255 }; 

	 /* Black screen */
	 SDL_FillRect(screen,NULL,0);
 	SDL_UpdateRect(screen,0,0,0,0);

	//Initialize SDL_ttf 
	if( TTF_Init() == -1 ) 
	{ 
		return false; 
	} 

	//Open the font 
	font = TTF_OpenFont(FONT, 28 ); 

	//If there was an error in loading the font 
	if( font == NULL ) { 
		fprintf(stderr,"Cannot open font\n");
		return false; 
	} 

	//Render the text 
	title = TTF_RenderText_Solid( font, "sdlVNC", textColor ); 

	//If there was an error in rendering the text 
	if( title == NULL ) { return 1; } 

	//Apply the images to the screen 
	apply_surface( 0, 0, title, screen ); 

	title = TTF_RenderText_Solid( font, VERSION, textColor ); 
	apply_surface( 200, 0, title, screen ); 


 	SDL_UpdateRect(screen,0,0,0,0);

	SDL_FreeSurface( title ); 
	//Close the font that was used 
	TTF_CloseFont( font ); 
	//Quit SDL_ttf 
//	TTF_Quit(); 
}

char GetTextKey(SDL_Surface *screen)
{
 SDL_Event event; 
 Uint32 key = 0;

  /* Check for events */
 //  while ( SDL_PollEvent(&event) ) {
 while (key == 0) {
   SDL_WaitEvent(&event);
   switch (event.type) {
    case SDL_KEYDOWN:
     /* Map SDL key to VNC key */
     key = event.key.keysym.sym;
     switch (event.key.keysym.sym) {
      case SDLK_BACKSPACE: key=0x08; break;
      case SDLK_RETURN: key=0x0d; break;
      default: 
	key = event.key.keysym.sym;
	if (key & 0xFFFFFF00)
	  key = 0;
	/* Handle upper case letters. */
	else if (event.key.keysym.mod & KMOD_SHIFT) {
	  key=toupper(key);
	}
     }
     break;
   }
  }
  return (char)(key & 0x00ff);
}

/* Very hacky! Just want to get something that doesnt requrie command line for now */
char* GetHostname(SDL_Surface *screen)
{
        TTF_Font *font;
	int	inloop=1;
	char	hostName[80] = "";
	char	keyval[4] = "";

	char	key;
	SDL_Surface	*title;
	SDL_Color textColor = { 255, 255, 255 }; 

	//	font = TTF_OpenFont(FONT, 28 ); 
	font = TTF_OpenFont(FONT, 16 ); 

	//If there was an error in loading the font 
	if( font == NULL ) { 
		fprintf(stderr,"Cannot open font2\n");
		return ""; 
	} 

	title = TTF_RenderText_Solid( font, "Enter Hostname:", textColor ); 
	if (title == NULL){
		fprintf(stderr,"Cannot render text?");
	}
	apply_surface( 0, 120, title, screen ); 
        // Update the screen
        SDL_UpdateRect(screen, 0, 0, 0, 0);

	while (inloop)
	{
		key=GetTextKey(screen);
		fprintf(stderr,"Key:%d\n",key);
		if (key > 0) // MOdifiers are mostly negative
		{
		switch (key) {
			case 0x0d:
				inloop=0;
				break;
			// Catch anything that isnt text -- Horrid code but I dont have the time to clean up
			case 0:
			//case 234:
			case 32:
			case 24:
			case 16: //Nothing
				break;
			case 8:
				if (strlen(hostName))
				  hostName[strlen(hostName)-1]=0;
				title = TTF_RenderText_Solid( font, hostName, textColor ); 
				apply_surface( 0,200, title, screen ); 
			        SDL_UpdateRect(screen, 0, 0, 0, 0);
				break;
			default:
				keyval[0] = key;
				strncat(hostName,keyval,1);
				title = TTF_RenderText_Solid( font, hostName, textColor ); 
				apply_surface( 0,200, title, screen ); 
			        SDL_UpdateRect(screen, 0, 0, 0, 0);
			}
		}
	        SDL_Delay(50);

	}

	SDL_FreeSurface( title ); 
	//Close the font that was used 
	TTF_CloseFont( font ); 
	//Quit SDL_ttf 
	TTF_Quit(); 

	return strdup(hostName);
}
#endif /* USE_TTF_SPLASH */

/* Drawing loop */
void Draw(SDL_Surface *screen, tSDL_vnc *vnc)
{
 SDL_Event event; 
 SDL_Rect updateRect;
 SDL_Surface *virt;
 SDL_Rect viewport;
 SDL_Rect origin;

 float outScale = 1.0;

 int inPan=0;
 int inloop;
 Uint8 mousebuttons, buttonmask;
 int mousex=0;
 int mousey=0;
 int priormousex=0;
 int priormousey=0;
 int mousedown=0;
 int velocity=0;

 Uint32 key;   
 /* Black screen */
 SDL_FillRect(screen,NULL,0);
 SDL_UpdateRect(screen,0,0,0,0);
// Create virtual screen 

#if 1 /* ZIPIT_Z2  (was ifdef ALLOW_ZOOMING) */
	virt = SDL_CreateRGBSurface( SDL_SWSURFACE, vnc->serverFormat.width+16, vnc->serverFormat.height+16, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, 0 ); 
#else
	virt = SDL_CreateRGBSurface( SDL_SWSURFACE, 2000, 1080, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, 0 ); 
#endif
	Uint32 red = SDL_MapRGB(screen->format, 240, 0, 20);
	SDL_FillRect(virt,NULL,red);
	viewport.x =0;
	viewport.y =0;
	viewport.w = DEFAULT_W;
	viewport.h = DEFAULT_H;
	origin.x=0;
	origin.y=0;
	origin.w=DEFAULT_W;
	origin.h=DEFAULT_H;
  
 inloop=1;
 while (inloop) {
    
  /* Check for events */
  while ( SDL_PollEvent(&event) ) {
//	fprintf(stderr,"***EVENT**%d\n",event.type);
   if (inPan) inPan=1;
   switch (event.type) {
#if 0
   case SDL_VIDEORESIZE:
     //Surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 0, SDL_SWSURFACE | SDL_RESIZABLE);
     screen = SDL_SetVideoMode(0,0, 0, SDL_SWSURFACE | SDL_RESIZABLE);
     if (Surface == NULL) {
       SDL_Quit();
       exit(1);
     }
     vncBlitFramebufferAdvanced(vnc, screen, &updateRect, outx, outy, outScale, 1);
     break;
#endif
#ifdef IZ2S
    case SDL_KEYDOWN:
#if 1 /* ZIPIT_Z2 */
     if (event.key.keysym.sym == SDLK_LCTRL) {
       inPan=1;
     }
#endif
    case SDL_KEYUP:
#if 1 /* ZIPIT_Z2 */
     if ((event.key.keysym.sym == SDLK_LCTRL) && (event.type == SDL_KEYUP)) {
       inPan=0;
     }
#endif
#else 
     // Use SHIFT for pan on openwrt
    case SDL_KEYDOWN:
#if 1 /* ZIPIT_Z2 */
     if (event.key.keysym.sym == SDLK_LSHIFT) {
       inPan=1;
     }
#endif
    case SDL_KEYUP:
#if 1 /* ZIPIT_Z2 */
     if ((event.key.keysym.sym == SDLK_LSHIFT) && (event.type == SDL_KEYUP)) {
       inPan=0;
     }
#endif
#endif
     /* Map SDL key to VNC key */
     key = event.key.keysym.sym;
//     fprintf(stderr,"**KEY0** %d\n",key);

     switch (event.key.keysym.sym) {
      case SDLK_BACKSPACE: key=0xff08; break;
      case SDLK_TAB: key=0xff09; break;
      case SDLK_RETURN: key=0xff0d; break;
      case SDLK_ESCAPE: key=0xff1b; 
	if (event.key.keysym.mod & KMOD_CTRL) 
	inloop=0; /* QUIT on the zipit for now. */
	break;
      case SDLK_INSERT: key=0xff63; break;
      case SDLK_DELETE: key=0xffff; break;
      case SDLK_HOME: key=0xff50; break;
      case SDLK_END: key=0xff57; break;
      case SDLK_PAGEUP: key=0xff55; break;
      case SDLK_PAGEDOWN: key=0xff56; break;
      case SDLK_LEFT: key=0xff51; break;
      case SDLK_UP: key=0xff52; break;
      case SDLK_RIGHT: key=0xff53; break;
      case SDLK_DOWN: key=0xff54; break;
      case SDLK_F1: key=0xffbe; break;
      case SDLK_F2: key=0xffbf; break;
      case SDLK_F3: key=0xffc0; break;
      case SDLK_F4: key=0xffc1; break;
      case SDLK_F5: key=0xffc2; break;
      case SDLK_F6: key=0xffc3; break;
      case SDLK_F7: key=0xffc4; break;
      case SDLK_F8: key=0xffc5; break;
      case SDLK_F9: key=0xffc6; break;
      case SDLK_F10: key=0xffc7; break;
      case SDLK_F11: key=0xffc8; break;
      case SDLK_F12: key=0xffc9; break;
      case SDLK_LSHIFT: key=0xffe1; break;
      case SDLK_RSHIFT: key=0xffe2; break;
      case SDLK_LCTRL: key=0xffe3; break;
// RCTRL is the sym key
      case SDLK_RCTRL: key=0xffe4; break;
      case SDLK_LMETA: key=0xffe7; break;
      case SDLK_RMETA: key=0xffe8; break;
#if 1 /*ZIPIT_Z2 lame alt key support for odd zipit kbd.  Fixme!  */
      case SDLK_LALT: 
      case SDLK_RALT: 
	key=0x0000; 
	break;
#else
      case SDLK_LALT: key=0xffe9; break;
// RALT is the Orange Key
// Need to dump it in order for the alternate keys to work for now
//      case SDLK_RALT: key=0xffea; break;
      case SDLK_RALT: key=0x0000; break;
#endif
      case 234: key=0x0000; break;
#ifdef ALLOW_ZOOMING
      case ',':
	fprintf(stderr,"**KEY0** %d\n",key);
	if (event.type == SDL_KEYDOWN && (event.key.keysym.mod & KMOD_CTRL)) {
	  float oldScale = outScale;
	  outScale = outScale - 0.05;
	  if (outScale < 0.25) outScale = 0.25;
	  fprintf(stderr,"**outScale = %1.2f \n",outScale);
	  viewport.x = (int)((float)viewport.x * (outScale / oldScale));
	  viewport.y = (int)((float)viewport.y * (outScale / oldScale));
	  key=0x0000; 
	}
	else key=','; 
	break;
      case '.':
	fprintf(stderr,"**KEY0** %d\n",key);
	if (event.type == SDL_KEYDOWN && (event.key.keysym.mod & KMOD_CTRL)) {
	  float oldScale = outScale;
	  outScale = outScale + 0.05;
	  if (outScale > 1.0) outScale = 1.0;
	  fprintf(stderr,"**outScale = %1.2f \n",outScale);
	  viewport.x = (int)((float)viewport.x * (outScale / oldScale));
	  viewport.y = (int)((float)viewport.y * (outScale / oldScale));
	  key=0x0000; 
	}
	else key='.'; 
	break;
#endif
      default: key = event.key.keysym.sym;
     }
#if 1 /*ZIPIT_Z2 lame alt key support for odd zipit kbd.  Fixme!  */
     if (event.key.keysym.mod & KMOD_SHIFT) {
       if (event.key.keysym.mod & KMOD_ALT) {
	 switch (event.key.keysym.sym) {
	 case 'k': key='`'; break;
	 case 'l': key='|'; break;
	 case 'x': key='\\'; break;
	 case 'm': key='%'; break;
	 case ';': key='^'; break;
	 case ',': key='{'; break;
	 case '.': key='}'; break;
	 }
       }
       else { // (event.key.keysym.mod & KMOD_SHIFT) 
	 switch (event.key.keysym.sym) {
	 /* shift keys are unusual for some zipit symbol chars. */
	 case ';': key='~'; break;
	 case ',': key='('; break;
	 case '.': key=')'; break;
	 }
	 key=toupper(key);
       }
     } /* NO shift, just ALT */
     else if (event.key.keysym.mod & KMOD_ALT) {
	 switch (event.key.keysym.sym) {
	 case 'q': key='1'; break;
	 case 'w': key='2'; break;
	 case 'e': key='3'; break;
	 case 'r': key='4'; break;
	 case 't': key='5'; break;
	 case 'y': key='6'; break;
	 case 'u': key='7'; break;
	 case 'i': key='8'; break;
	 case 'o': key='9'; break;
	 case 'p': key='0'; break;
	 case 'a': key='$'; break;
	 case 's': key='#'; break;
	 case 'd': key='&'; break;
	 case 'f': key='@'; break;
	 case 'g': key='"'; break;
	 case 'h': key='\''; break;
	 case 'j': key='['; break;
	 case 'k': key=']'; break;
	 case 'l': key='-'; break;
	 case SDLK_BACKSPACE: key=0xffff; break;
	 case 'z': key='!'; break;
	 case 'x': key='/'; break;
	 case 'c': key='+'; break;
	 case 'v': key='*'; break;
	 case 'b': key='='; break;
	 case 'n': key='_'; break;
	 case 'm': key='?'; break;
	 case ';': key=':'; break;
	 case SDLK_RETURN: key=0xff0d; break;
	 case ',': key='<'; break;
	 case '.': key='>'; break;
	 case SDLK_ESCAPE: key='|'; 
	 case SDLK_TAB: key=0xff63; break;
	 case SDLK_HOME: key=0xff55; break;
	 case SDLK_END: key=0xff56; break;
	 case SDLK_PAGEUP: key=0xff50; break;
	 case SDLK_PAGEDOWN: key=0xff57; break;
	 }
     }
//     fprintf(stderr,"**KEY** %d\n",key);
#else
     /* Handle upper case letters. */
     if (event.key.keysym.mod & KMOD_SHIFT) {
      key=toupper(key);
     }
#endif
//     fprintf(stderr,"**KEY** %d\n",key);

     if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 231) {
		inPan=1;
     } else if (event.type == SDL_KEYUP && event.key.keysym.sym == 231) {
                inPan=0;
     } else if (key != 0x0000) {
//     fprintf(stderr,"**KEY2** %d\n",key);
	/* Add client event */
	vncClientKeyevent(vnc, (event.type==SDL_KEYDOWN), key);
     }
     break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEMOTION:

     /* Get current mouse state */
	priormousex = mousex;
	priormousey = mousey;
     mousebuttons=SDL_GetMouseState(&mousex,&mousey);

	if (event.type == SDL_MOUSEBUTTONDOWN)
		mousedown=1;
	if (event.type == SDL_MOUSEBUTTONUP)
		mousedown=0;

	if (event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEBUTTONDOWN)
	{
		priormousex = mousex;
		priormousey = mousey;
	}
	// Pan?
//		fprintf(stderr,"**Mouse**%d-%d\n",mousex,mousey);
	if (inPan==1)
	{
		viewport.x=viewport.x-(mousex - priormousex);
		viewport.y=viewport.y-(mousey - priormousey);
//		fprintf(stderr,"**PAN**%d-%d  %d-%d   %d-%d\n",mousex,mousey,priormousex,priormousey,viewport.x,viewport.y);
		if (viewport.x < 0) {
			viewport.x=0;
		}
		if (viewport.y < 0) {
			viewport.y=0;
		}
		inPan = 2; /* We actually did some panning.  Remember that. */
	}
     /* Map SDL buttonmask to VNC buttonmask */
     buttonmask=0;
     if (vnc_mbswap) {
     if (mousebuttons & SDL_BUTTON(SDL_BUTTON_LEFT)) buttonmask       |= 2;
     if (mousebuttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) buttonmask     |= 1;
     }
     else {
     if (mousebuttons & SDL_BUTTON(SDL_BUTTON_LEFT)) buttonmask       |= 1;
     if (mousebuttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) buttonmask     |= 2;
     }
     if (mousebuttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) buttonmask      |= 4;
     if (mousebuttons & SDL_BUTTON(SDL_BUTTON_WHEELUP)) buttonmask    |= 8;
     if (mousebuttons & SDL_BUTTON(SDL_BUTTON_WHEELDOWN)) buttonmask  |= 16;
     /* Add client event */
    if (inPan != 2){
#ifdef ALLOW_ZOOMING
      //vncClientPointerevent(vnc, buttonmask, (mousex / outScale)+viewport.x, (mousey / outScale)+viewport.y);
      vncClientPointerevent(vnc, buttonmask, (mousex+viewport.x) / outScale, (mousey+viewport.y) / outScale);
#else
      vncClientPointerevent(vnc, buttonmask, mousex+viewport.x, mousey+viewport.y);
#endif
	}
     break;

    case SDL_QUIT:
     inloop=0;
     break;
    }
   }

   /* Blit VNC screen */
   if (vncBlitFramebuffer(vnc, virt, &updateRect) || inPan==2) {
    /* Display by updating changed parts of the display */
//    SDL_UpdateRect(screen,updateRect.x,updateRect.y,updateRect.w,updateRect.h);
//    apply_surface(viewport.x,viewport.y,virt,screen);
#ifdef ALLOW_ZOOMING
     static SDL_Surface *zoomvirt = NULL;
     
     if (outScale == 1.0)
       SDL_BlitSurface(virt,&viewport,screen,&origin);
     else {
       SDL_FreeSurface(zoomvirt); 
       //fprintf(stderr,"**zoomScale = %1.2f \n",outScale);
       if ((zoomvirt = zoomSurface(virt, outScale, outScale, 1)) != NULL) {
	 //fprintf(stderr,"**zoomBlit = %1.2f \n",outScale);
	 SDL_BlitSurface(zoomvirt,&viewport,screen,&origin);
       }
       else 
	 SDL_BlitSurface(virt,&viewport,screen,&origin);
     }
#else
     SDL_BlitSurface(virt,&viewport,screen,&origin);
#endif
     //SDL_UpdateRect(virt,0,0,0,0);
     SDL_UpdateRect(screen,0,0,0,0);
   }
   /* Delay to limit rate */                   
  if(inPan != 2)
   SDL_Delay(1000/vnc_framerate);
   
 }
}

void PrintUsage()
{
 fprintf (stderr,"Usage: sdlvnc [SDL options] server:display [VNC opts]\n");
 fprintf (stderr," SDL parameters:\n");
 fprintf (stderr,"  -width [i]      Set screen width (default: %i)\n",DEFAULT_W);
 fprintf (stderr,"  -height [i]     Set_screen height (default: %i)\n",DEFAULT_H);
 fprintf (stderr,"  -bpp [i]        Set [i] bits per pixel\n");
 fprintf (stderr,"  -warp           Use hardware palette\n");
 fprintf (stderr,"  -hw             Use hardware surface\n");
#if 1 /* ZIPIT_Z2 */
 fprintf (stderr,"  -mb             Swap middle and left mouse buttons\n");
#else
 fprintf (stderr,"  -fullscreen     Go into fullscreen mode\n");
#endif
 fprintf (stderr," VNC parameters:\n");
 fprintf (stderr,"  server:display  VNC server (eg. localhost:1)\n");
 fprintf (stderr,"  -port [i]       VNC base port (default: 5900)\n");
 fprintf (stderr,"  -method [s]     Method to use, first to last.\n");
 fprintf (stderr,"                  OK: hextile,rre,copyrect,raw,cursor\n");
 fprintf (stderr,"                  Missing: corre,zrle,tight,desktop\n");
 fprintf (stderr,"                  (default: hextile,rre,copyrect,raw)\n");
 fprintf (stderr,"  -password [s]   VNC password to use\n");
 fprintf (stderr,"                  (default: none)\n");
 fprintf (stderr,"  -framerate [i]  Target rate for RFB reqs, redraws\n");
 fprintf (stderr,"                  (default: 12)\n");
}

#ifdef WIN32
 extern char ** __argv;
 extern int __argc;
 int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
#else // non WIN32
 int main ( int argc, char *argv[] )
#endif
{
	SDL_Surface *screen;
	int w, h;
	int desired_bpp;
	Uint32 video_flags;
#ifdef WIN32
	int argc;
	char **argv;

	argv = __argv;
	argc = __argc;
#endif
	tSDL_vnc vnc;
	int result;

	/* Title */
//	fprintf (stderr,"sdlvnc %s - Based on SDL_vnc sample client - LGPL, A. Schiffler\n\n",VERSION);

        if (argc==1) {
//         PrintUsage();
//         exit(0);
        }
        
	/* Set default options and check command-line */
	w = DEFAULT_W;
	h = DEFAULT_H;
	desired_bpp = 0;
	video_flags = 0;
	while ( argc > 1 ) {

		if ( strcmp(argv[1], "-help") == 0 ) {
			PrintUsage();
			exit(0);
		}

		/* SDL specific arguments */
		
		if ( strcmp(argv[1], "-width") == 0 ) {
			if ( argv[2] && ((w = atoi(argv[2])) > 0) ) {
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,
				"The -width option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-height") == 0 ) {
			if ( argv[2] && ((h = atoi(argv[2])) > 0) ) {
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,"The -height option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-bpp") == 0 ) {
			if ( argv[2] ) {
				desired_bpp = atoi(argv[2]);
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,"The -bpp option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-warp") == 0 ) {
			video_flags |= SDL_HWPALETTE;
			argv += 1;
			argc -= 1;
		} else
		if ( strcmp(argv[1], "-hw") == 0 ) {
			video_flags |= SDL_HWSURFACE;
			argv += 1;
			argc -= 1;
		} else
		if ( strcmp(argv[1], "-fullscreen") == 0 ) {
			video_flags |= SDL_FULLSCREEN;
			argv += 1;
			argc -= 1;
		} else

		
		/* VNC specific arguments */

		if ( strcmp(argv[1], "-port") == 0 ) {
			if (argv[2]) {
				vnc_port = atoi(argv[2]);
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,"The -port option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-method") == 0 ) {
			if (argv[2]) {
				vnc_method = strdup(argv[2]);
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,"The -method option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-password") == 0 ) {
			if (argv[2]) {
				vnc_password = strdup(argv[2]);
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,"The -password option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-framerate") == 0 ) {
			if (argv[2]) {
				vnc_framerate = atoi(argv[2]);
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,"The -framerate option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-mb") == 0 ) {
			vnc_mbswap = 1;
			argv += 1;
			argc -= 1;
		} else {
			vnc_server = strdup(argv[1]);
			argv += 1;
			argc -= 1;
		}
	}

	/* Check/Adjust VNC parameters */
        if (vnc_method==NULL) {	
         vnc_method=strdup("hextile,rre,copyrect,raw");
        }
        if (vnc_password==NULL) {	
         vnc_password=strdup("");
        }
        if ((vnc_framerate<1) || (vnc_framerate>100)) {
	 fprintf (stderr,"Bad framerate (%i). Use a value from 1 to 100.\n",vnc_framerate);
	 PrintUsage();
         exit(1);
	}        
        
#ifndef USE_TTF_SPLASH
	/* Make sure to get server:display BEFORE SDL_Init(). */
	if (vnc_server==NULL) {
	  printf("Server:Display = ");
          fflush(stdout);
	  vnc_server = calloc(sizeof(char), 128);
	  fgets(vnc_server, 128, stdin);
	  vnc_server = strtok(vnc_server, "\r\n");
	  //sscanf("%s",vnc_server);
	}
	if (!strlen(vnc_password)) {
          strcpy(vnc_password, getpass("Password = "));
	  //printf("password = ");
          //vnc_password = calloc(sizeof(char), 128);
	  //fgets(vnc_password, 128, stdin);
	  //strtok(vnc_password, "\r\n");
	}
	if (!vnc_server || !strlen(vnc_server)) {
	 fprintf (stderr,"Need Servername.\n");
	 PrintUsage();
         exit(1);
	}
	else
#else
	if (vnc_server==NULL) {
	  /* Get it later via SDL_ttf prompts once SDL win is open. */
	}
	else
#endif
	{
	    	struct hostent *he;
    		struct in_addr **addr_list;

		char *buf;
		printf("Address: %s\n",vnc_server);
		if(buf = strrchr(vnc_server, ':')){
		  *buf++ = 0;
		  vnc_display = atoi(buf);
		}
		vnc_port += vnc_display;

		he =gethostbyname(vnc_server);
		addr_list = (struct in_addr **)he->h_addr_list;
		vnc_server=strdup(inet_ntoa(*addr_list[0]));

		printf("Server = %s:%d\n",vnc_server,vnc_port);

		/* Open vnc connection */
		result =vncConnect(&vnc,vnc_server,vnc_port,vnc_method,vnc_password,vnc_framerate);
		if (result == 0)
		{
		  printf("No connection to %s:%d\n",vnc_server,vnc_port);
		  exit(1);
		}
		else printf("Connected(%dx%d)\n",vnc.serverFormat.width,vnc.serverFormat.height);
	}

	/* Force double buffering */
	video_flags |= SDL_DOUBLEBUF;

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);			/* Clean up on exit */

	/* Initialize the display */
	screen = SDL_SetVideoMode(w, h, desired_bpp, video_flags);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
					w, h, desired_bpp, SDL_GetError());
		exit(1);
	}

	/* Check for double buffering */
	if ( screen->flags & SDL_DOUBLEBUF ) {
		printf("Double-buffering enabled - good!\n");
	}

	/* Set the window manager title bar */
	SDL_WM_SetCaption("sdlvnc", "sdlvnc");

#if 1 /* ZIPIT_Z2 */
	SDL_ShowCursor( SDL_DISABLE ); 
#endif

#ifdef USE_TTF_SPLASH
	/* Get Parameters */
	if (vnc_server == NULL)
	{
	    	struct hostent *he;
    		struct in_addr **addr_list;
		SplashScreen(screen);
		vnc_server=GetHostname(screen);
		printf("Address: %s\n",vnc_server);

		char *buf;
		buf = strtok(vnc_server, ":");
		if ( (buf = strtok(NULL, "")) )
		  vnc_display = atoi(buf);
		vnc_port += vnc_display;

		he =gethostbyname(vnc_server);
		addr_list = (struct in_addr **)he->h_addr_list;
		vnc_server=strdup(inet_ntoa(*addr_list[0]));

		printf("Server = %s:%d\n",vnc_server,vnc_port);

		/* Open vnc connection */
		result =vncConnect(&vnc,vnc_server,vnc_port,vnc_method,vnc_password,vnc_framerate);
		if (result = 0)
		{
		  printf("No connection to %s:%d\n",vnc_server,vnc_port);
		  exit(1);
		}
	}
#endif /* USE_TTF_SPLASH */

	/* Do all the drawing work */
	Draw (screen, &vnc);
			
	/* Close connection */
	vncDisconnect(&vnc);

	//TTF_Quit();
	free(vnc_server);	
	return(0);
}
