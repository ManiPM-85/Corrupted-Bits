#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// SCALE RATE IS 3

// ENUMS BEGIN
enum Scene
{
	MENU,
	GAME,
	END
};
// ENUMS END

// TYPEDEF BEGIN
typedef enum Scene Scene;
typedef struct Bit Bit;
typedef struct End End;
typedef struct Game Game;
typedef struct Menu Menu;
typedef struct Timer Timer;
typedef struct Color Color;
typedef struct Stage Stage;
typedef struct Display Display;
// TYPEDEF END

// STATICS BEGIN
static int i = 0;
static int k = 0;
static int activeBits = 0;
static char restart = 0;
static float delta = 0;
static float bsize = 30;
static float remTime = 210;
static Scene curScene = MENU;
// STATICS END

// UTILS BEGIN
struct Color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

int isPointInFRect(SDL_FRect *rect, float x, float y)
{
	if (x >= rect->x && y >= rect->y &&
		x <= rect->x + rect->w &&
		y <= rect->y + rect->h)
		return 1;
	else
		return 0;
}

void randomize()
{
	srand(time(0));
}

int randint(int min, int max)
{
	return (rand() % (max - min + 1)) + min;
}

float getDelta(uint64_t *ct, uint64_t *lt)
{
	*lt = *ct;
	*ct = SDL_GetPerformanceCounter();

	return (float)(*ct - *lt) /
	(float)SDL_GetPerformanceFrequency();
}
// UTILS END

// DISPLAY BEGIN
struct Display
{
	SDL_Window *window;
	SDL_Renderer *renderer;
};

Display *Display_init(const char *title)
{
	register int size = sizeof(Display);
	Display *self = (Display *)malloc(size);

	self->window = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
	    270, 600,
		SDL_WINDOW_SHOWN
	);

	assert(self->window != NULL);

	self->renderer = SDL_CreateRenderer(
		self->window,
		-1,
		SDL_RENDERER_ACCELERATED
	);

	return self;
}

void Display_changeColor(Display *self, Color *color)
{
	SDL_SetRenderDrawColor(
		self->renderer,
		color->r,
		color->g,
		color->b,
		255
	);
}

void Display_clear(Display *self)
{
	SDL_RenderClear(self->renderer);
}

void Display_show(Display *self)
{
	SDL_RenderPresent(self->renderer);
}

void Display_destroy(Display *self)
{
	SDL_DestroyRenderer(self->renderer);
	SDL_DestroyWindow(self->window);
	free(self);
}
// DISPLAY END

// TEXTURE BEGIN
typedef struct Texture Texture;

struct Texture
{
	SDL_Texture *texture;
	SDL_Rect transform;
};

Texture *Texture_init(Display *g, const char *filePath)
{
	register int size = sizeof(Texture);
	Texture *self = (Texture*)malloc(size);

	self->texture = NULL;
	self->texture = IMG_LoadTexture(g->renderer, filePath);
	assert(self->texture != NULL);

	return self;
}

void Texture_setTransform(Texture *self, int x, int y)
{
	self->transform.x = x;
	self->transform.y = y;
	self->transform.w = 270;
	self->transform.h = 47;
}

void Texture_render(Texture *self, Display *g)
{
	SDL_RenderCopy(g->renderer, self->texture, NULL, &self->transform);
}

void Texture_destroy(Texture *self)
{
	SDL_DestroyTexture(self->texture);
	free(self);
}
// TEXTURE END

// TIMER BEGIN
struct Timer
{
	SDL_FRect transform;
	Color color;
};

Timer *Timer_init()
{
	register int size = sizeof(Timer);
	Timer *self = (Timer*)malloc(size);
	
	self->transform.x = bsize;
	self->transform.y = (bsize * 2) / 3;
	self->transform.w = remTime;
	self->transform.h = bsize;
	
	self->color = (Color){255, 255, 255};
	
	return self;
}

void Timer_cutTime()
{
	remTime -= remTime/6;
}

void Timer_update(Timer *self)
{
	if(self->transform.w > 0)
		remTime -= bsize/12*delta;
	
	self->transform.w = remTime;
}

void Timer_render(Timer *self, Display *g)
{
	Display_changeColor(
		g, &self->color
	);

	SDL_RenderFillRectF(
		g->renderer,
		&self->transform
	);
}

void Timer_destroy(Timer *self)
{
	free(self);
}
// TIMER END

// PLAYER BEGIN
struct Bit
{
	SDL_FRect transform;
	Color color;

	int active, corrupted;
};

Bit *Bit_init(float x, float y)
{
	register int size = sizeof(Bit);
	Bit *self = (Bit*)malloc(size);

	self->transform.x = x;
	self->transform.y = y;
	self->transform.w = bsize;
	self->transform.h = bsize;

	self->active = randint(0, 1);

	if (self->active)
		self->corrupted = randint(0, 14);

	self->color = (Color){255, 255, 255};

	return self;
}

void Bit_setPos(Bit *self, char dir, float amt)
{
	switch (dir)
	{
	case 'x':
		self->transform.x = amt;
		break;
	case 'y':
		self->transform.y = amt;
		break;
	}
}

float Bit_getPos(Bit *self, char dir)
{
	switch (dir)
	{
	case 'x':
		return self->transform.x;
	case 'y':
		return self->transform.y;
	}
}

void Bit_update(Bit* self);

void Bit_render(Bit *self, Display *g)
{
	if (self->active)
	{
		Display_changeColor(
			g, &self->color
		);

		SDL_RenderFillRectF(
			g->renderer,
			&self->transform
		);
	}
	else
	{
		if (self->corrupted == 1)
		{
			self->color = (Color){255,0,0};

			Display_changeColor(
				g, &self->color
			);

			SDL_RenderFillRectF(
				g->renderer,
				&self->transform
			);

			self->color = (Color){
				255, 255, 255
			};
		}
	}
}

void Bit_touch(Bit *self, float x, float y)
{
	if (isPointInFRect(
			&self->transform,
			x, y) &&
		self->active)
	{
		self->active = 0;
		activeBits--;
		if(self->corrupted == 1)
		{
			Timer_cutTime();
		}
	}
}

void Bit_destroy(Bit *self)
{
	free(self);
}
// PLAYER END

// MENUSCENE BEGIN
struct Menu
{
	SDL_Rect square;
	Texture *title;
	Color color;
};

Menu *Menu_init()
{
	register int size = sizeof(Menu);
	Menu *self = (Menu*)malloc(size);
	
	self->square.x = 120;
	self->square.y = 210;
	self->square.w = 30;
	self->square.h = 30;

	self->color = (Color){255, 255, 0};
	
	return self;
}

void Menu_render(Menu *self, Display *g)
{
	Display_changeColor(g, &self->color);
	SDL_RenderFillRect(
		g->renderer,
		&self->square
	);
}

void Menu_getInput(Menu *self)
{
	curScene = GAME;
}

void Menu_destroy(Menu *self)
{
	free(self);
}
// MENUSCENE END

// GAMESCENR BEGIN
struct Game
{
	Bit *bit[105];
	Timer *timer;
};

Game *Game_init()
{
	register int size = sizeof(Game);
	Game *self = (Game*)malloc(size);

	activeBits = 0;
	for (i = 0; i < 7; i++)
	{
		for (k = 0; k < 15; k++)
		{
			self->bit[7 * k + i] = Bit_init(
				bsize * (i + 1),
				bsize * ((float)k + 2.5f)
			);
			if(self->bit[7 * k + i]->active)
				activeBits++;
		}
	}
	
	self->timer = Timer_init();

	return self;
}

void Game_restart(Game *self)
{
	remTime = 210;
	activeBits = 0;
	for (i = 0; i < 7; i++)
	{
		for (k = 0; k < 15; k++)
		{
			self->bit[7 * k + i] = Bit_init(
				bsize * (i + 1),
				bsize * ((float)k + 2.5f)
			);
			if(self->bit[7 * k + i]->active)
				activeBits++;
		}
	}
}

void Game_update(Game *self)
{
	/*
	for (i = 0; i < 105; i++)
	{
		Bit_update(
			self->bit[i]
		);
	}
	*/
	Timer_update(self->timer);
	if(activeBits <= 0)
	{
		curScene = END;
		Game_restart(self);
	}
	if(remTime <= 0)
	{
		assert(0);
	}
}


void Game_render(Game *self, Display *g)
{
	for (i = 0; i < 105; i++)
	{
		Bit_render(
			self->bit[i], g
		);
	}
	
	Timer_render(self->timer, g);
}

void Game_getInput(Game *self, SDL_MouseButtonEvent *mouse)
{
	for (i = 0; i < 105; i++)
	{
		Bit_touch(
			self->bit[i],
			mouse->x,
			mouse->y
		);
	}
}

void Game_destroy(Game *self)
{
	Timer_destroy(self->timer);
	for (i = 0; i < 105; i++)
		Bit_destroy(self->bit[i]);
	free(self);
}
// GAMESCENE END

// ENDSCENE BEGIN
struct End
{
	SDL_Rect square;
	Color color;
};

End *End_init()
{
	register int size = sizeof(End);
	End *self = (End*)malloc(size);
	
	self->square.x = 120;
	self->square.y = 210;
	self->square.w = 30;
	self->square.h = 30;
	self->color = (Color){0, 255, 0};
	
	return self;
}

void End_render(End *self, Display *g)
{
	Display_changeColor(g, &self->color);
	SDL_RenderFillRect(
		g->renderer,
		&self->square
	);
}

void End_getInput(End *self)
{
	curScene = MENU;
}

void End_destroy(End *self)
{
	free(self);
}
// ENDSCENE END

// STAGE BEGIN
struct Stage
{
	Display *display;
	SDL_Event event;
	Color bgColor;
	Texture *title;

	Menu *menu;
	Game *game;
	End *end;
	
	int running;
};

Stage *Stage_init()
{
	register int size = sizeof(Stage);
	Stage *self = (Stage *)malloc(size);

	self->running = 0;
	self->display = Display_init("Corrupte Bits");
	self->bgColor = (Color){0, 0, 0};
	self->title = Texture_init(self->display, "../res/title.png");
	Texture_setTransform(self->title, 0, 545);

	self->menu = Menu_init();
	self->game = Game_init();
	self->end = End_init();

	return self;
}

void Stage_loop(Stage *self)
{
	auto uint64_t curtTick, lastTick = 0;

	curtTick = SDL_GetPerformanceCounter();

	self->running = 1;
	while (self->running)
	{
		while (SDL_PollEvent(&self->event))
		{
			switch (self->event.type)
			{
			case SDL_QUIT:
				self->running = 0;
				break;
			case SDL_KEYDOWN:
				if(self->event.key.keysym.sym == SDLK_ESCAPE)
					self->running = 0;
					break;
			case SDL_MOUSEBUTTONDOWN:
				switch(curScene)
				{
				case MENU:
					Menu_getInput(
						self->menu
					);
					break;
				case GAME:
					Game_getInput(
						self->game,
						&self->event.button
					);
					break;
				case END:
					End_getInput(
						self->end
					);
				}
			}
		}

		delta = getDelta(
			&curtTick,
			&lastTick
		);

		Display_changeColor(
			self->display,
			&self->bgColor
		);
		Display_clear(self->display);
		
		switch(curScene)
		{
			case MENU:
				Menu_render(
					self->menu,
					self->display
				);
				Texture_render(
					self->title, 
					self->display
				);
				break;
			case GAME:
				Game_render(
					self->game,
					self->display
				);
				Texture_render(
					self->title, 
					self->display
				);
				Game_update(self->game);
				break;
			case END:
				End_render(
					self->end,
					self->display
				);
				Texture_render(
					self->title, 
					self->display
				);
		}
		
		
		Display_show(self->display);
	}
}

void Stage_destroy(Stage *self)
{
	Texture_destroy(self->title);
	End_destroy(self->end);
	Game_destroy(self->game);
	Menu_destroy(self->menu);
	Display_destroy(self->display);
	free(self);
}
// STAGE END

int main(int argc, char *argv[])
{
	assert(!SDL_Init(SDL_INIT_EVERYTHING));
	assert(IMG_Init(IMG_INIT_PNG));
	randomize();

	Stage *stage = Stage_init();
	Stage_loop(stage);
	Stage_destroy(stage);

	SDL_Quit();
	return 0;
}