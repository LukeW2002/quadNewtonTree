#include "../include/globals.h"

void handleKeyPress(SDL_KeyboardEvent *event)
{
	if (event->type == SDL_KEYDOWN)
	{
		switch (event->keysym.sym)
		{
			case SDLK_SPACE:
				show_grid = !show_grid;
				break;
		}
	}
}
