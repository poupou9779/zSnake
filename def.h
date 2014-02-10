#ifndef DEF_H

#include <SDL.h>
#include <SDL_mixer.h>

/*Used because SDL uses Sint16 type very much, then 20 warnings are set (int <-> Sint16)*/
#pragma GCC diagnostic ignored "-Wconversion"

/*Needs to be defined with the tileset dimensions value*/
#define HEIGHT_SPRITE	33
#define WIDTH_SPRITE	33

/*those are defined in tileset_e*/
#define NB_SPRITES	25
/*number of cells allocated to snake.body*/
#define MAX 500
/*volour of the backgroun : R : 173, G : 210, B : 137*/
#define BACKGROUND 0x00ADD289

/*const path defines*/
#define PATH_TILESET "data\\T.bmp"
#define PATH_MENU    "data\\Menu.bmp"
#define PATH_LEVELS  "data\\Levels.txt"
#define PATH_MUSIC   "data\\BMusic.wav"
#define PATH_EAT     "data\\eat.wav"
#define PATH_DIE     "data\\die.wav"
#define NAME_GAME    "Snake SDL"

/*ordered list of the tileset content*/
enum tileset_e {EMPTY, HEAD_UP, HEAD_DOWN, HEAD_RIGHT, HEAD_LEFT, BODY, WALL, APPLE, HEART,
                LETTER_S, LETTER_C, LETTER_O, LETTER_R, LETTER_E, COLON,
                NUMBER_0, NUMBER_1, NUMBER_2, NUMBER_3, NUMBER_4, NUMBER_5, NUMBER_6, NUMBER_7, NUMBER_8, NUMBER_9};
enum {UP = 0, DOWN, RIGHT, LEFT};
/*after moving, is the snake dead or is he alive*/
enum {DEAD, ALIVE};

typedef struct
{
    /*length of pos which is used as an array*/
    int length;
    /*the <i>real</i> tileset*/
	SDL_Surface *image;
	/*the array of positions (for each tile)*/
	SDL_Rect *pos;
}
tileset_t;

typedef struct
{
        /*length of body*/
    int length,
        /*UP, RIGHT, DOWN, LEFT*/
        direction,
        /*number of move per second*/
        speed;
    /*position of the head of the snake*/
    SDL_Rect head;
    /*array of positions (each cell of the body)*/
    SDL_Rect body[MAX];
}
Snake_t;

typedef struct
{
    /*each map (or game) has a predefined number of cell on each axis*/
    int X, Y,
        /*coordinates of the snake spawner*/
        xspawn, yspawn,
        /*array simple which is used as double (simple is costs less time in the execution)
          in this array is the <i>real</i> content of the map : each tile (tileset_t)*/
        *board,
        /*number of points needed to be earned before the game (map) stops*/
        maxpoints;
}
game_t;

typedef struct
{
    /*explicit*/
	SDL_Surface *screen;
	tileset_t tileset;
        /*number of levels, thus length of 'game'*/
	int nlevels,
        /*each level starts with 3 of it. When 0, game stops*/
        lives,
        /*current amount of points. Reset in the beggening of each level, but is kept with a addition-return-system*/
        points;
    /*array of maps*/
	game_t *game;
	/*informations on the snake are the same for each level*/
    Snake_t snake;
    /*music which is looped as long as the user has not asked to quit*/
    Mix_Music *music;
    /*sounds which are played when the snake eats an apple or hits a wall (or eats himself)*/
    Mix_Chunk *eat,
              *die;
}
context_t;

/*creates the whole context-structure*/
void InitContext(context_t *ctxt);
/*called by InitContext, does the initialisation of each tileset*/
void InitTileset(tileset_t *tileset, const char *path, int length);
/*called bu InitContext, does the initialisation of music, eat and die*/
void InitMusics(context_t *ctxt);
/*called explicitly just before leaving, it frees al of the allocated memory*/
void FreeContext(context_t *ctxt);

/*is called by Play every time a new life is started*/
void PrepareGame(context_t *ctxt, int level);
/*tries to move the snake, anr precies if whether he's still alive or not*/
int MoveSnake(context_t *ctxt, int level);
/*returns SDL_TRUE if there is nothing on the (x; y) cell*/
SDL_bool isfree(context_t *ctxt, int level, int x, int y);

/*generates a random coordinate for an apple, and stocks it*/
void GenApple(context_t *ctxt, int level);

/*main function of the game itself : contents the loops and the points system*/
int Play(context_t *ctxt, int level);
#define PLAY 1
#define QUIT 2
/*makes the user choose between playing or quitting the game*/
int Menu(context_t *ctxt);

/*functions that blits the whole map and the snake*/
void BlitAll(context_t *ctxt, int level);
/*called by BlitAll and blits "score:%3d", score*/
void BlitScore(context_t *ctxt, int level);

#endif
