#include <SDL/SDL.h>
#include "def.h"

/*those arrays are used for moving the snake.
  It helps to not have multiple conditions such as if(directionn = UP) --y; else if(...)*/
int moveX[4] = {-0, +0, +1, -1},
    moveY[4] = {-1, +1, +0, -0};

void InitContext(context_t *ctxt)
{
    int i, j, k, tmp;
    FILE *levels;
    InitTileset(&ctxt->tileset, PATH_TILESET, NB_SPRITES);
    /*open the file which contains the levels*/
    levels = fopen(PATH_LEVELS, "r");
    if(levels == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", PATH_LEVELS);
        exit(EXIT_FAILURE);
    }
    fscanf(levels, "%d", &ctxt->nlevels);
    /*allocate the games configs*/
    ctxt->game = malloc((unsigned int)ctxt->nlevels * sizeof(game_t));
    if(ctxt->game == NULL)
    {
        fprintf(stderr, "Unable to malloc the game array !\n");
        exit(EXIT_FAILURE);
    }
    /*read the game config in the file*/
    for(k = 0; k < ctxt->nlevels; ++k)
    {
        fscanf(levels, "%d%d%d\n", &ctxt->game[k].X, &ctxt->game[k].Y, &ctxt->game[k].maxpoints);
        ctxt->game[k].board = malloc((unsigned int)(ctxt->game[k].X*ctxt->game[k].Y) * sizeof(int));
        if(ctxt->game[k].board == NULL)
        {
            fprintf(stderr, "Unable to allocate the %dth board with %d*%d\n", k, ctxt->game[k].Y, ctxt->game[k].X);
            exit(EXIT_FAILURE);
        }
        /*reads each char in the file to interpret the map*/
        for(i = 0; i < ctxt->game[k].Y; ++i)
        {
            for(j = 0; j < ctxt->game[k].X; ++j)
            {
                tmp = fgetc(levels);
                /*'S' is the predefined const value which means spawner. So the snake will pop on this position*/
                if(tmp == 'S')
                {
                    ctxt->game[k].xspawn = j*HEIGHT_SPRITE;
                    ctxt->game[k].yspawn = i*WIDTH_SPRITE;
                    /*and of course, this cell is a non-occupied cell, thus an EMPTY one*/
                    ctxt->game[k].board[i*ctxt->game[k].X + j] = EMPTY;
                }
                else
                    ctxt->game[k].board[i*ctxt->game[k].X + j] = tmp - '0';
            }
            (void)fgetc(levels);
        }
    }
    ctxt->snake.head.h = HEIGHT_SPRITE;
    ctxt->snake.head.w = WIDTH_SPRITE;
    for(i = 0; i < MAX; ++i)
    {
        ctxt->snake.body[i].h = HEIGHT_SPRITE;
        ctxt->snake.body[i].w = WIDTH_SPRITE;
    }
    fclose(levels);
    ctxt->screen = NULL;
    ctxt->pause = SDL_LoadBMPAlpha(PATH_PAUSE, 128);
    InitMusics(ctxt);
}

void InitTileset(tileset_t *tileset, const char *path, int length)
{
    int i;
    tileset->image = SDL_LoadBMP(path);
    if(tileset->image == NULL)
    {
        fprintf(stderr, "Unable to load %s : %s\n", path, SDL_GetError());
        exit(EXIT_FAILURE);
    }
    tileset->length = length;
    tileset->pos = malloc(sizeof(SDL_Rect) * (unsigned int)length);
    if(tileset->pos == NULL)
    {
        fprintf(stderr, "Unable to malloc pos for %s-tileset\n", path);
        exit(EXIT_FAILURE);
    }
    for(i = 0; i < length; ++i)
    {
        /*sets the correct position of each tile*/
        tileset->pos[i].w = WIDTH_SPRITE;
        tileset->pos[i].h = HEIGHT_SPRITE;
        tileset->pos[i].x = i*WIDTH_SPRITE;
        tileset->pos[i].y = 0;
    }
}

void InitMusics(context_t *ctxt)
{
    /*explicit enough to not be commented*/
    ctxt->music = Mix_LoadMUS(PATH_MUSIC);
    if(ctxt->music == NULL)
    {
        fprintf(stderr, "Unable to load %s\n", PATH_MUSIC);
        exit(EXIT_FAILURE);
    }
    Mix_AllocateChannels(2);
    ctxt->eat = Mix_LoadWAV(PATH_EAT);
    if(ctxt->eat == NULL)
    {
        fprintf(stderr, "Unable to load %s\n", PATH_EAT);
        exit(EXIT_FAILURE);
    }
    ctxt->die = Mix_LoadWAV(PATH_DIE);
    if(ctxt->die == NULL)
    {
        fprintf(stderr, "Unable to load %s\n", PATH_DIE);
        exit(EXIT_FAILURE);
    }
    Mix_Volume(0, MIX_MAX_VOLUME/2);
    Mix_Volume(1, MIX_MAX_VOLUME/2);
}

int Menu(context_t *ctxt)
{
    SDL_Surface *img = SDL_LoadBMP(PATH_MENU);
    SDL_Event e;
    if(img == NULL)
    {
        fprintf(stderr, "Unable to load the Menu image : %n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    /*300, 300 are the dimensions of the PATH_MENU image*/
    SDL_FreeSurface(ctxt->screen);
    ctxt->screen = SDL_SetVideoMode(300, 300, 32, SDL_HWSURFACE);
    /*if the reallocation failed, leave*/
    if(ctxt->screen == NULL)
    {
        fprintf(stderr, "Unable to create screen : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_BlitSurface(img, NULL, ctxt->screen, NULL);
    SDL_Flip(ctxt->screen);
    SDL_FreeSurface(img);
    do
        SDL_WaitEvent(&e);
    while(e.type != SDL_KEYDOWN && e.key.keysym.sym < SDLK_KP0 && e.key.keysym.sym > SDLK_KP0+QUIT);
    /*while the user has not pressed a key which is on the keypad and which is between PLAY and QUIT*/
    return (int)(e.key.keysym.sym - SDLK_KP0);
}

void PrepareGame(context_t *ctxt, int level)
{
    int i, j;
    SDL_FreeSurface(ctxt->screen);
    ctxt->screen = SDL_SetVideoMode(ctxt->game[level].X*WIDTH_SPRITE, (ctxt->game[level].Y+2)*HEIGHT_SPRITE, 32, SDL_DOUBLEBUF|SDL_HWSURFACE);
    /*the snake has 1 head, and 3 cells in his body in the beggining and after each death*/
    ctxt->snake.length = 3;
    ctxt->snake.direction = UP;
    /*places the snake on his spawn position*/
    ctxt->snake.head.x = ctxt->game[level].xspawn;
    ctxt->snake.head.y = ctxt->game[level].yspawn;
    /*30 frames each 4500 ms, thus 1 frame every 150 ms in  the beggining. But this gets faster and faster !*/
    ctxt->snake.speed = 30;
    /*places snake.body in line with snake.head*/
    for(i = 0; i < ctxt->snake.length; ++i)
    {
        ctxt->snake.body[i].x = ctxt->snake.head.x;
        ctxt->snake.body[i].y = ctxt->snake.head.y + (i+1)*HEIGHT_SPRITE;
    }
    /*erases the apple if there is still one (from the previous life)*/
    for(i = 0; i < ctxt->game[level].Y; ++i)
        for(j = 0; j < ctxt->game[level].X; ++j)
            if(ctxt->game[level].board[i*ctxt->game[level].X + j] == APPLE)
               ctxt->game[level].board[i*ctxt->game[level].X + j] = EMPTY;
}

SDL_Surface *SDL_LoadBMPAlpha(const char *path, Uint8 alpha)
{
    SDL_Surface *ret = SDL_LoadBMP(path);
    if(ret == NULL)
    {
        fprintf(stderr, "Unable to load ret : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if(SDL_SetAlpha(ret, SDL_SRCALPHA, alpha) == -1)
    {
        fprintf(stderr, "Unable to set alpha to ret : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Play(context_t *ctxt, int level)
{
    SDL_Event e;
    SDL_bool _loop;
    Uint32 lasttime = SDL_GetTicks(),
           currtime;
    /*if this was the last level*/
    if(level == ctxt->nlevels)
        return 1;
    SDL_WM_SetCaption(NAME_GAME, NULL);
    /*resets the score, because the old one will be added to the current one in the end*/
    ctxt->points = 0;
    while(ctxt->lives > 0)
    {
        /*resets the settings before each life*/
        PrepareGame(ctxt, level);
        _loop = SDL_TRUE;
        GenApple(ctxt, level);
        while(_loop && ctxt->points < ctxt->game[level].maxpoints && ctxt->snake.length < MAX)
        {
            BlitAll(ctxt, level);
            SDL_Flip(ctxt->screen);
            /*if a key was pressed*/
            SDL_PollEvent(&e);
            /*if an arrow-key was pressed and the new direction is not the exact opposit of the current one*/
            if(e.type == SDL_KEYDOWN &&
               e.key.keysym.sym <= SDLK_LEFT && e.key.keysym.sym >= SDLK_UP &&
               !(ctxt->snake.head.x+moveX[e.key.keysym.sym-SDLK_UP]*WIDTH_SPRITE == ctxt->snake.body[0].x &&
                    ctxt->snake.head.y+moveY[e.key.keysym.sym-SDLK_UP]*HEIGHT_SPRITE == ctxt->snake.body[0].y))
                /*then update the current direction*/
                ctxt->snake.direction = (int)(e.key.keysym.sym - SDLK_UP);
            else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_p)
            {
                SDL_Rect dst;
                dst.w = ctxt->pause->w;
                dst.h = ctxt->pause->h;
                dst.x = (ctxt->screen->w - dst.w)/2;
                dst.y = (ctxt->screen->h - dst.h)/2;
                SDL_BlitSurface(ctxt->pause, NULL, ctxt->screen, &dst);
                SDL_Flip(ctxt->screen);
                do
                    SDL_WaitEvent(&e);
                while(e.type != SDL_KEYDOWN && e.key.keysym.sym != SDLK_RETURN);
            }
            else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            {
                ctxt->lives = 0;
                _loop = SDL_FALSE;
            }
            /*currtime and lasttime are used to get the program to sleep while it is not used*/
            currtime = SDL_GetTicks();
            /*if the snake has not moved for quite some times*/
            if(currtime - lasttime > (unsigned int)(9000/ctxt->snake.speed))
            {
                lasttime = currtime;
                /*get it to move*/
                if(MoveSnake(ctxt, level) == DEAD)
                {
                    /*and if the snake is on a non-free cell, play PATH_DIE and stops looping*/
                    Mix_PlayChannel(1, ctxt->die, 0);
                    _loop = SDL_FALSE;
                }
            }
            /*otherwise, sleep a bit*/
            else
                SDL_Delay(((Uint32)(9000/ctxt->snake.speed) - currtime + lasttime)/4);
        }
        /*here, the snake has finished or is dead*/
        BlitAll(ctxt, level);
        SDL_Flip(ctxt->screen);
        SDL_Delay(1500);
        /*if the max number of points is not achieved yet, it means that the snake ate a wall or itself*/
        if(ctxt->points != ctxt->game[level].maxpoints)
            /*then decrease the lives counter*/
            --ctxt->lives;
        else
            /*if(the snake has eaten maxpoints apples, return points*level² + points of the next level*/
            break;
    }
    return ctxt->points + Play(ctxt, level+1);
}

void GenApple(context_t *ctxt, int level)
{
    /*explicit enough to not be commented*/
    int x, y;
    do
    {
        x = rand()%ctxt->game[level].X;
        y = rand()%ctxt->game[level].Y;
    }
    while(!isfree(ctxt, level, x, y));
    ctxt->game[level].board[y*ctxt->game[level].X + x] = APPLE;
}

int MoveSnake(context_t *ctxt, int level)
{
    int i,
        currx = ctxt->snake.head.x/WIDTH_SPRITE, curry = ctxt->snake.head.y/HEIGHT_SPRITE,
        nextx = currx + moveX[ctxt->snake.direction], nexty = curry + moveY[ctxt->snake.direction];
    /*checks if the snake needs to pass through the map*/
    if(currx == ctxt->game[level].X-1 && ctxt->snake.direction == RIGHT)
        nextx = 0;
    else if(currx == 0 && ctxt->snake.direction == LEFT)
        nextx = ctxt->game[level].X-1;
    else if(curry == 0 && ctxt->snake.direction == UP)
        nexty = ctxt->game[level].Y-1;
    else if(curry == ctxt->game[level].Y-1&& ctxt->snake.direction == DOWN)
        nexty = 0;
    /*if the cell isn't free, return that the snake has failed*/
    for(i = 0; i < ctxt->snake.length-1; ++i)
        if(nextx == ctxt->snake.body[i].x/WIDTH_SPRITE && nexty == ctxt->snake.body[i].y/HEIGHT_SPRITE)
            return DEAD;
    switch(ctxt->game[level].board[nexty*ctxt->game[level].X + nextx])
    {
    /*if the snake tries to eat a wall, he dies*/
    case WALL:
        return DEAD;
    case EMPTY:
        /*update the position of each cell of the body and the head*/
        for(i = ctxt->snake.length-1; i > 0; --i)
            ctxt->snake.body[i] = ctxt->snake.body[i-1];
        ctxt->snake.body[0] = ctxt->snake.head;
        ctxt->snake.head.x = nextx*WIDTH_SPRITE;
        ctxt->snake.head.y = nexty*HEIGHT_SPRITE;
        break;
    case APPLE:
        Mix_PlayChannel(0, ctxt->eat, 0);
        /*adds a new cell to the body*/
        ++ctxt->snake.length;
        for(i = ctxt->snake.length-1; i > 0; --i)
            ctxt->snake.body[i] = ctxt->snake.body[i-1];
        ctxt->snake.body[0] = ctxt->snake.head;
        ctxt->snake.head.x = nextx*WIDTH_SPRITE;
        ctxt->snake.head.y = nexty*HEIGHT_SPRITE;
        /*gets rid of the apple*/
        ctxt->game[level].board[nexty*ctxt->game[level].X + nextx] = EMPTY;
        /*but we need a new one*/
        GenApple(ctxt, level);
        /*increase the counter of points and of course the speed to get it harder*/
        ++ctxt->points;
        ++ctxt->snake.speed;
    default:
        break;
    }
    return ALIVE;
}

SDL_bool isfree(context_t *ctxt, int level, int x, int y)
{
    int i;
    /*if (x; y) is not empty, return SDL_FALSE*/
    if(ctxt->game[level].board[y*ctxt->game[level].X + x] != EMPTY)
        return SDL_FALSE;
    else
    {
        /*if snake.head or snake.body is on (x; y), returns SDL_FALSE*/
        if(ctxt->snake.head.x/WIDTH_SPRITE == x && ctxt->snake.head.y/HEIGHT_SPRITE == y)
            return SDL_FALSE;
        for(i = 0; i < ctxt->snake.length; ++i)
            if(ctxt->snake.body[i].x/WIDTH_SPRITE == x && ctxt->snake.body[i].y/HEIGHT_SPRITE == y)
                return SDL_FALSE;
    }
    /*otherwise, everything went good, and then the cell is free. Thus returns SDL_TRUE*/
    return SDL_TRUE;
}

void BlitAll(context_t *ctxt, int level)
{
    int i, j;
    SDL_Rect dst;
    dst.h = HEIGHT_SPRITE;
    dst.w = WIDTH_SPRITE;
    /*erases the content of the screen*/
    SDL_FillRect(ctxt->screen, NULL, BACKGROUND);
    /*Blits each tile*/
    for(i = 0; i < ctxt->game[level].Y; ++i)
    {
        dst.y = i*dst.h;
        for(j = 0; j < ctxt->game[level].X; ++j)
        {
            dst.x = j*dst.w;
            SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[ctxt->game[level].board[i*ctxt->game[level].X + j]],
                            ctxt->screen,        &dst);
        }
    }
    /*Blits the whole snake*/
    SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[ctxt->snake.direction + HEAD_UP], ctxt->screen, &ctxt->snake.head);
    for(i = 0; i < ctxt->snake.length; ++i)
        SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[BODY], ctxt->screen, &ctxt->snake.body[i]);
    /*explicit*/
    BlitScore(ctxt, level);
    /*Blits the lifes that are left*/
    dst.y = (ctxt->game[level].Y+1)*HEIGHT_SPRITE;
    for(i = 0; i < ctxt->lives; ++i)
    {
        dst.x = WIDTH_SPRITE*i;
        SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[HEART], ctxt->screen, &dst);
    }
}

void BlitScore(context_t *ctxt, int level)
{
    int i, tmp = ctxt->points;
    SDL_Rect src, dst;
    /*Gets prepared to blit "score:"*/
    src.h = HEIGHT_SPRITE;
    src.w = 6*WIDTH_SPRITE;
    src.y = 0;
    src.x = LETTER_S*WIDTH_SPRITE;

    dst.h = HEIGHT_SPRITE;
    dst.w = WIDTH_SPRITE;
    dst.x = 0;
    dst.y = ctxt->game[level].Y*HEIGHT_SPRITE;
    /*Blits "SCORE:"*/
    SDL_BlitSurface(ctxt->tileset.image, &src, ctxt->screen, &dst);
    /*Places the 'cursor' on the last writing-tile*/
    dst.x = 9*WIDTH_SPRITE;
    src.w = WIDTH_SPRITE;
    /*Writes Right to Left the score*/
    for(i = 0; i < 3; ++i)
    {
        dst.x -= WIDTH_SPRITE;
        src.x = ((tmp%10) + NUMBER_0) * WIDTH_SPRITE;
        tmp /= 10;
        SDL_BlitSurface(ctxt->tileset.image, &src, ctxt->screen, &dst);
    }
}

void FreeContext(context_t *ctxt)
{
    /*explicit enough to not be commented*/
    int i;
    SDL_FreeSurface(ctxt->tileset.image);
    for(i = 0; i < ctxt->nlevels; ++i)
        free(ctxt->game[i].board);
    free(ctxt->game);
    free(ctxt->tileset.pos);
    Mix_FreeMusic(ctxt->music);
}

