#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"
#include "SDL.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;
const int TILE_SIZE = 10;
const char *foreBMP = "D:\\fore.bmp";
const char *backBMP = "D:\\back.bmp";
bool quit = false;

void drawGfx();
void setKeys();
SDL_Texture* loadTexture(const char *filename, SDL_Renderer *ren);
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y);

int main(int argc, char *argv[]) {
	if (argc < 2){
		printf("please input gamename\n");
		system("pause");
		return 1;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		printf("SDL_Init Error: %s", SDL_GetError());
		return 1;
	}

	SDL_Window *win = SDL_CreateWindow("chip-8 emulator", 100, 100, \
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (win == NULL){
		printf("SDL_CreateWindow Error: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, \
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == NULL){
		SDL_DestroyWindow(win);
		printf("SDL_CreateRenderer Error: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Texture *fore = loadTexture(foreBMP, ren);
	SDL_Texture *back = loadTexture(backBMP, ren);
	if (fore == NULL || back == NULL){
		if (!fore){ SDL_DestroyTexture(fore); }
		if (!back){ SDL_DestroyTexture(back); }
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		printf("loadTexture Error: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	initialize();
	if (loadGame(argv[1])){
		return 1;
	}

	for (;;){
		emulateCycle();

		if (drawFlag){
			SDL_RenderClear(ren);
			renderTexture(back, ren, 0, 0);
			for (int i = 0; i < 64 * 32; i++){
				if (gfx[i]){
					renderTexture(fore, ren, (i % 64) * 10, (i / 64) * 10);
				}	
			}

			SDL_RenderPresent(ren);
			drawFlag = false;
		}

		setKeys();
		if (quit){
			break;
		}
	}

	SDL_DestroyTexture(fore);
	SDL_DestroyTexture(back);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}

void setKeys()
{
	SDL_Event e;
	if (SDL_PollEvent(&e)){
		if(e.type == SDL_KEYDOWN){
			switch (e.key.keysym.sym){
			case SDLK_1:
				key[1] = 1;
				break;
			case SDLK_2:
				key[2] = 1;
				break;
			case SDLK_3:
				key[3] = 1;
				break;
			case SDLK_q:
				key[4] = 1;
				break;
			case SDLK_w:
				key[5] = 1;
				break;
			case SDLK_e:
				key[6] = 1;
				break;
			case SDLK_a:
				key[7] = 1;
				break;
			case SDLK_s:
				key[8] = 1;
				break;
			case SDLK_d:
				key[9] = 1;
				break;
			case SDLK_x:
				key[0] = 1;
				break;
			case SDLK_z:
				key[0xa] = 1;
				break;
			case SDLK_c:
				key[0xb] = 1;
				break;
			case SDLK_4:
				key[0xc] = 1;
				break;
			case SDLK_r:
				key[0xd] = 1;
				break;
			case SDLK_f:
				key[0xe] = 1;
				break;
			case SDLK_v:
				key[0xf] = 1;
				break;
			default:
				break;
			}
		}else if (e.type == SDL_KEYUP){
			switch (e.key.keysym.sym){
			case SDLK_1:
				key[1] = 0;
				break;
			case SDLK_2:
				key[2] = 0;
				break;
			case SDLK_3:
				key[3] = 0;
				break;
			case SDLK_q:
				key[4] = 0;
				break;
			case SDLK_w:
				key[5] = 0;
				break;
			case SDLK_e:
				key[6] = 0;
				break;
			case SDLK_a:
				key[7] = 0;
				break;
			case SDLK_s:
				key[8] = 0;
				break;
			case SDLK_d:
				key[9] = 0;
				break;
			case SDLK_x:
				key[0] = 0;
				break;
			case SDLK_z:
				key[0xa] = 0;
				break;
			case SDLK_c:
				key[0xb] = 0;
				break;
			case SDLK_4:
				key[0xc] = 0;
				break;
			case SDLK_r:
				key[0xd] = 0;
				break;
			case SDLK_f:
				key[0xe] = 0;
				break;
			case SDLK_v:
				key[0xf] = 0;
				break;
			default:
				break;
			}
		}
		else if (e.type == SDL_QUIT){
			quit = true;
		}
	}
}

SDL_Texture* loadTexture(const char *filename, SDL_Renderer *ren)
{
	SDL_Texture *texture = NULL;
	SDL_Surface *loadImage = SDL_LoadBMP(filename);

	if (loadImage != NULL){
		texture = SDL_CreateTextureFromSurface(ren, loadImage);
		SDL_FreeSurface(loadImage);
		if (texture == NULL){
			printf("loadTexture Error: %s", SDL_GetError());
		}
	}else{
		printf("LoadBMP Error: %s", SDL_GetError());
	}
	
	return texture;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, NULL, &dst);
}