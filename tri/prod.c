#include "simplex.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "loader.h"
#include "module.h"
#include "player.h"
#include "output_portaudio.h"

#include "muzzak.h"
#include "font.h"
#include "vgafont.h"

#define WSIZE_X 1024
#define WSIZE_Y 768

#define SSIZE_X 320
#define SSIZE_Y 240

#define FSIZE_X 800
#define FSIZE_Y 600
#define FSIZE_Z 1000

#define FONT_WIDTH 32
#define FONT_HEIGHT 32

#define SINE_TABLE_LENGTH 128

static const char message[] = "  selber   seruelser            yeah the oldskool strikez back, greetings fly out to Future Crew, triton.. hey wait, it's 2014... and you can still read this scroller? well... CHECK THIS OUT DUDE!!!               WOOOOOOOOOOOT NOW EAT THIS SUCKER!!!!!!!                                         ";  
static const int message_length = 293; //((char *)&message_length - message);

static const char schocker[] = {
	"DOS/4GW error (2001): exception 0Eh (page fault) at 170:0042C1B2\n"
	"TSF32: prev_tsf32 67D8\n"
	"SS       178 DS       178 ES        178 FS       0 GS  20\n"
	"EAX 1F000000 EBX        0 ECX    43201C EDX      E\n"
	"ESI        E EDI        0 EBP    431410 ESP 4313FC\n"
	"CS:IP  170:0042C1B2 ID 0E COD         0 FLG  10246\n"
	"CS= 170,USE32, page granular, limit FFFFFFFF, base    0, acc CF9B\n"
	"SS= 178,USE32, page granular, limit FFFFFFFF, base    0, acc CF93\n"
	"DS= 178,USE32, page granular, limit FFFFFFFF, base    0, acc CF93\n"
	"ES= 178,USE32, page granular, limit FFFFFFFF, base    0, acc CF93\n"
	"FS=   0,USE16, byte granular, limit        0, base   15, acc    0\n"
	"GS=  20,USE16, byte granular, limit     FFFF, base 6AA0, acc   93\n"
	"CR0: PG1:1 ET:1 TS:0 EM:0 MP:0 PE:1   CR2: 1F000000 CR3: 9067\n"
	"Crash address (unrelocated) = 1:000001B2\n"
	"C:\\DEMOS\\MISC\\VERYBAD\\>"
};


int prod_phase = 0;

typedef struct {
	int prod_phase;
	
	SDL_Renderer * ren;
	SDL_Texture * tex;
	SDL_Texture * tex2;
	SDL_Surface * sur;
	SDL_Surface * sur2;

	int sine_table[SINE_TABLE_LENGTH];

} prod_canvas_t;

void mk_sinetable(int * table, int table_length, float y_scale) {
	const float PI_2 = 6.28318530718;
	int i;
	for (i = 0; i < table_length; i++) 
		table[i] = sin((PI_2 / table_length) * i) * y_scale;
}

void order_callback(player_t * player, void * user_ptr) {
	switch (player->current_order) {
		case 0: prod_phase = 0; break;
		case 1: prod_phase = 1; break;
		case 3: prod_phase = 2; break;
		case 7: prod_phase = 3; break;
		case 13: prod_phase = 4; break;
		case 17: prod_phase = 23; break;
	}
	
	
	//printf("%i\n", player->current_order);
}


typedef enum {
	EVENT_TYPE_TICK		= 0,
	EVENT_TYPE_PRODPHASE	= 1
} PROD_EVENT_TYPES;

Uint32  timer_callback(Uint32 intervall, void * parameter) 
{
	SDL_Event e;
	SDL_UserEvent ue;
	
	ue.type = SDL_USEREVENT;
	ue.code = EVENT_TYPE_TICK;

	e.type = SDL_USEREVENT;
	e.user = ue;

	SDL_PushEvent (&e);

	return intervall;
}


void update_scene(prod_canvas_t * c) 
{

	int x, y, i, k, l;

	SDL_LockTexture(c->tex2, 0, &c->sur2->pixels, &c->sur2->pitch);
	SDL_LockTexture(c->tex, 0, &c->sur->pixels, &c->sur->pitch);


	static float zz = 0;
	static int message_index = 0;
	static int schocker_offset = 0;
	static int schocker_line;
	
	static int sco = 0;	
	static int y_offset = 0;
	static int last_prod_phase = -1;

	if (prod_phase != last_prod_phase) {
		memset(c->sur2->pixels, 0, SSIZE_X * SSIZE_Y * sizeof(uint32_t));
		if (prod_phase == 0) {
			message_index = 0;
			sco = 0;
		}
		if (prod_phase == 1) {
			message_index = 10;
			sco = 16;
		}

		last_prod_phase = prod_phase;
	}

	if (prod_phase != 23) {
	
	
	


		for (x=0;x<SSIZE_X;x++) {
			for (y=0;y<SSIZE_Y;y++) {
				unsigned char idx = simplex_noise3((float)x / ((float)SSIZE_X / 4), (float)y / ((float)SSIZE_Y / 4), zz)  *  128;
				idx ^= 0x80;
		
				uint32_t color =  prod_phase < 4 ? (idx) : (idx << 16);
				((uint32_t *)c->sur->pixels)[y * SSIZE_X + x] = color;				
			}

		}


		l = message_index;

		y_offset = 0;
		for (i = 0; i < SSIZE_X; i++) {
			k = (i + sco) % FONT_WIDTH;
			//c = font[(message[l] - 'a') * FONT_WIDTH + k];
	
			int x_offset = (message[l] - 32) * FONT_WIDTH + k;


			//printf ("%c", c);
	
			for (y = 0; y < FONT_HEIGHT; y++) {
				uint32_t fp =  ((uint32_t *)font.pixel_data)[x_offset + (y * 4096)];
				((uint32_t *)c->sur2->pixels)[(y + (prod_phase > 2 ? (c->sine_table[y_offset] >> (prod_phase < 4 ? 2 : 0) ) : 0)  + ((SSIZE_Y / 2) - (FONT_HEIGHT / 2))   ) * SSIZE_X + i] = fp;
			}

			y_offset = (y_offset + 1) % SINE_TABLE_LENGTH;

			if (k == (FONT_WIDTH - 1)) {
		
				l++;
				if (l == message_length)
					l = 0;
			}

		}


		if (prod_phase > 1) {
			sco = ((sco + 2) % FONT_WIDTH);

			if (sco == 0) {
	
				message_index++;
				if (message_index == message_length)
					message_index = 0;
			}
		}

		SDL_BlitSurface(c->sur2, 0, c->sur, 0);
		SDL_UnlockTexture(c->tex2);
		SDL_UnlockTexture(c->tex);
		SDL_RenderCopy(c->ren, c->tex, 0, 0);
		SDL_RenderPresent(c->ren);
		zz+=.05f;

	} else {
		SDL_LockTexture(c->tex, 0, &c->sur->pixels, &c->sur->pitch);

		x = 0;
		y = 0;

		for (l = 0; l<strlen(schocker); l++) {



			int offs = schocker[l] * 16;

			if (y > (SSIZE_Y - 14)) {
				y -= 14;
				x = 0;
				for (i=0; i<SSIZE_Y - 14; i++) {
					memcpy (c->sur->pixels + (i * SSIZE_X * sizeof(uint32_t)), c->sur->pixels + ((i + 14) * SSIZE_X * sizeof(uint32_t)), SSIZE_X * sizeof(uint32_t));
				}
				memset (c->sur->pixels + (i * SSIZE_X) * sizeof(uint32_t), 0, SSIZE_X * sizeof(uint32_t) * 14);
			}


			if (schocker[l] == '\n') {
				x = 0;
				y += 14;
			} else {

				for (i = 0; i < 16; i++) {
					char c1 = vgafont[offs++];
					if (i==0 || i==15)
						continue;
					for (k = 0; k < 8; k++) {
						((uint32_t *)c->sur->pixels)[((i-1) + y) * SSIZE_X + (k + x)] = (c1 & (1 << (7-k))) ? 0xff7f7f7f : 0xff000000;
					}
				}
				x +=8;
			}


			if (x > (SSIZE_X - 8)) {
				x = 0;
				y += 14;
			}

		}

		SDL_UnlockTexture(c->tex2);
		SDL_UnlockTexture(c->tex);
		SDL_RenderCopy(c->ren, c->tex, 0, 0);
		SDL_RenderPresent(c->ren);
	}

	
}

int main(int argc, char ** argv) {

	int fu = 0;
	prod_canvas_t c;

	module_t * mod;
	mod = loader_loadmem_by_header((void *)muzzak, muzzak_size);

	player_t * plr;
	plr = player_init(44100, player_resampling_linear);
	plr->loop_module = 1;
	plr->loop_pattern = -1;
	plr->solo_channel = -1;

	player_set_module(plr, mod);
	player_register_order_callback(plr, order_callback);

	output_opts_t opts = { 1024, 44100, 0 };
	output_portaudio_init(&opts);

	output_portaudio_start(plr);

	SDL_Window * win;
	SDL_Event e;
	SDL_TimerID timer;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "ERROR SDL_Init: %s\n", SDL_GetError());
		return 1;
	}

	win = SDL_CreateWindow("Hardcore", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WSIZE_X, WSIZE_Y, SDL_WINDOW_SHOWN);
	if (win == 0) {
		fprintf(stderr, "ERROR SDL_CreateWindow: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	c.ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (c.ren == 0) {
		fprintf(stderr, "ERROR SDL_CreateRenderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;

	}

	

	c.tex = SDL_CreateTexture(c.ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SSIZE_X, SSIZE_Y);
	c.tex2 = SDL_CreateTexture(c.ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SSIZE_X, SSIZE_Y);
	c.sur = SDL_CreateRGBSurfaceFrom(0, SSIZE_X, SSIZE_Y, 32, 0, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	c.sur2 = SDL_CreateRGBSurfaceFrom(0, SSIZE_X, SSIZE_Y, 32, 0, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	mk_sinetable(c.sine_table, SINE_TABLE_LENGTH, 64.0f);

	timer = SDL_AddTimer (25, timer_callback, NULL);


			

	while (SDL_WaitEvent(&e) && (!fu)) { //User requests quit 
		switch (e.type) {

		case SDL_QUIT:
			printf("quit request\n");
			fu = 1;
			break;

		case SDL_USEREVENT:
			update_scene(&c);
			break;
			
		default: 
			break;

		}


	}

	output_portaudio_stop(plr);

	SDL_DestroyRenderer(c.ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
		

}
