#include <SDL.h>
#include <SDL_mixer.h>
#include "def.h"
#include <time.h>

int main(int ac, char **av)
{
    int m, result;
    char title[20];
    context_t ctxt;
    /*Initialisation of SDL*/
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) == -1)
    {
        /*if failure, leave*/
        fprintf(stderr, "Unable to init video : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    atexit(SDL_Quit);
    /*Initialisation of SDL_mixer*/
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1)
    {
        /*if failure, leave*/
        fprintf(stderr, "Unable to init SDL_mixer : %s\n", Mix_GetError());
        return EXIT_FAILURE;
    }
    atexit(Mix_CloseAudio);
    srand((unsigned int)time(NULL));
    /*Set Title of the window*/
    SDL_WM_SetCaption(NAME_GAME, NULL);
    InitContext(&ctxt);
    Mix_PlayMusic(ctxt.music, -1);
    do
    {
        m = Menu(&ctxt);
        /*if user presses PLAY (1)*/
        if(m == PLAY)
        {
            /*Play the game, and change the title with the user's result ex : 931 points !*/
            result = Play(&ctxt, 0);
            sprintf(title, "%d points !", result);
            SDL_WM_SetCaption(title, NULL);
            /*Wait a second with the score as title, then sets the normal title back*/
            SDL_Delay(1000);
            SDL_WM_SetCaption(NAME_GAME, NULL);
        }
    }
    while(m != QUIT);
    Mix_PauseMusic();
    FreeContext(&ctxt);
    return 0;
    (void)ac;
    (void)av;
}

