#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"
#include "fontset.h"

unsigned short opcode;

unsigned char memory[4096];	//	4k ram

unsigned char V[16];	//	16 general purpose regs V0-Vf

unsigned short I;
unsigned short pc;
unsigned short stack[16];
unsigned short sp;
unsigned char delay_timer;
unsigned char sound_timer;

unsigned char gfx[64 * 32];
unsigned char key[16];

void initialize()
{
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	//clear display
	for(int i=0; i<64*32; i++) {
		gfx[i] = 0;
	}
	
	//clear stack
	for(int i=0; i<16; i++){
		stack[i] = 0;
	}
	
	//clear reg
	for(int i=0; i<16; i++){
		V[i] = 0;
		key[i] = 0;
	}
	
	//clear memory
	for(int i=0; i<4096; i++){
		memory[i] = 0;
	}
	
	// Load fontset
	for(int i = 0; i < 80; ++i){
		memory[i] = chip8_fontset[i];
	}

	//reset timer
	delay_timer = 0;
	sound_timer = 0;
	
	srand((unsigned int)time(NULL));
	
	drawFlag = true;
}

int loadGame(const char *filename)
{
	FILE *game;
	errno_t err;
	unsigned char buf[4096];
	int i;
	
	if (err = fopen_s(&game, filename, "rb") != 0){
		printf("gamefile open error1!");
		return 1;
	}

	i = fread(buf, sizeof(unsigned char), 4096, game);
	if(i == 0){
		printf("gamefile read error2!");
		fclose(game);
		return 2;
	}
    if(i > (4096 - 512)){
    	printf("gamefile size error3");
    	fclose(game);
    	return 3;
	}
	
	for(int j=0; j<i; j++){
		memory[512 + j] = buf[j];
	}
	
	fclose(game);
	return 0;
}

void emulateCycle()
{
	// Fetch Opcode
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Decode Opcode
	pc += 2;
	switch(opcode & 0xf000) {
		case 0x0000:
			if(opcode == 0x00e0) {
				//clear display
				for(int i=0; i<64*32; i++) {
					gfx[i] = 0;
				}
				drawFlag = true;
			} else if(opcode == 0x00ee) {
				sp--;
				pc = stack[sp];
			} else {
				printf("Unknow opcode[0x0000] : 0x%X", opcode);
			}
			break;
		case 0x1000:
			pc = opcode & 0x0fff;
			break;
		case 0x2000:
			stack[sp] = pc;
			sp++;
			pc = opcode & 0x0fff;
			break;
		case 0x3000:
			if(V[(opcode & 0x0f00) >> 8] == (opcode & 0x00ff)){
				pc += 2;
			}
			break;
		case 0x4000:
			if(V[(opcode & 0x0f00) >> 8] != (opcode & 0x00ff)){
				pc += 2;
			}
			break;
		case 0x5000:
			if((opcode & 0x000f) != 0x0){
				printf("Unknow opcode[0x5000] : 0x%X", opcode);
				break;
			}
			if(V[(opcode & 0x0f00) >> 8] == V[(opcode & 0x00f0) >> 4]){
				pc += 2;
			}
			break;
		case 0x6000:
			V[(opcode & 0x0f00) >> 8] = (opcode & 0x00ff);	
			break;
		case 0x7000:
			V[(opcode & 0x0f00) >> 8] += (opcode & 0x00ff);		
			break;
		case 0x8000:
			switch(opcode & 0x000f){
			case 0x0:
				V[(opcode & 0x0f00) >> 8] = V[(opcode & 0x00f0) >> 4];
				break;
			case 0x1:
				V[(opcode & 0x0f00) >> 8] |= V[(opcode & 0x00f0) >> 4];
				break;
			case 0x2:
				V[(opcode & 0x0f00) >> 8] &= V[(opcode & 0x00f0) >> 4];
				break;
			case 0x3:
				V[(opcode & 0x0f00) >> 8] ^= V[(opcode & 0x00f0) >> 4];
				break;
			case 0x4:
				V[(opcode & 0x0f00) >> 8] += V[(opcode & 0x00f0) >> 4];
				if(V[(opcode & 0x0f00) >> 8] > (0xff - V[(opcode & 0x00f0) >> 4])){
					V[0xf] = 1;
				}else{
					V[0xf] = 0;
				}
				break;
			case 0x5:
				V[(opcode & 0x0f00) >> 8] -= V[(opcode & 0x00f0) >> 4];
				if(V[(opcode & 0x0f00) >> 8] >= V[(opcode & 0x00f0) >> 4]){
					V[0xf] = 1;
				}else{
					V[0xf] = 0;
				}
				break;
			case 0x6:
				V[0xf] = V[(opcode & 0x0f00) >> 8] & 0x1;
				V[(opcode & 0x0f00) >> 8] >>=1;
				break;
			case 0x7:
				V[(opcode & 0x0f00) >> 8] = V[(opcode & 0x00f0) >> 4] - V[(opcode & 0x0f00) >> 8];
				if(V[(opcode & 0x0f00) >> 8] > V[(opcode & 0x00f0) >> 4]){
					V[0xf] = 0;
				}else{
					V[0xf] = 1;
				}
				break;
			case 0xe:
				V[0xf] = V[(opcode & 0x0f00) >> 8] >> 7;
				V[(opcode & 0x0f00) >> 8] <<=1;
				break;
			default:
				printf("Unknow opcode[0x8000] : 0x%X", opcode);
			}
			break;
		case 0x9000:
			if((opcode & 0x000f) != 0x0){
				printf("Unknow opcode[0x9000] : 0x%X", opcode);
				break;
			}
			if(V[(opcode & 0x0f00) >> 8] != V[(opcode & 0x00f0) >> 4]){
				pc += 2;
			}
			break;
		case 0xa000:
			I = opcode & 0xfff;
			break;
		case 0xb000:
			pc = (opcode & 0xfff) + V[0];
			break;
		case 0xc000:
			V[(opcode & 0x0f00) >> 8] = (rand() % 0xFF) & (opcode & 0x00ff);
			break;
		case 0xd000:
			{
			unsigned short x = V[(opcode & 0x0f00) >> 8];
			unsigned short y = V[(opcode & 0x00f0) >> 4];
			unsigned short n = opcode & 0x000f;
			unsigned short dat;
			unsigned short index;
			V[0xf] = 0;

			for (int i = 0; i < n; i++){

				dat = memory[I + i];
				for (int j = 0; j < 8; j++){
					if ((dat & (0x80 >> j)) != 0){
						index = (y + i) * 64 + x + j;
						if (gfx[index] == 1){
							V[0xf] = 1;
						}
						gfx[index] ^= 1;
					}
				}
			}
			}
			drawFlag = true;
			break;
		case 0xe000:
			switch(opcode & 0x00ff){
			case 0x9e:
				if(key[V[(opcode & 0x0f00) >> 8]] == 1){
					pc += 2;
				}
				break;
			case 0xa1:
				if(key[V[(opcode & 0x0f00) >> 8]] == 0){
					pc += 2;
				}
				break;
			default:
				printf("Unknow opcode[0xe000] : 0x%X", opcode);
			}
			break;
		case 0xf000:
			switch(opcode & 0x00ff){
			case 0x07:
				V[(opcode & 0x0f00) >> 8] = delay_timer;
				break;
			case 0x0a:
				;
				int i;
				for(i=0; i<16; i++){
					if(key[i] == 1){
						V[(opcode & 0x0f00) >> 8] = i;
					}
				}
				if(i ==  16){
					pc -= 2;
					return;
				}
				break;
			case 0x15:
				delay_timer = V[(opcode & 0x0f00) >> 8];
				break;
			case 0x18:
				sound_timer = V[(opcode & 0x0f00) >> 8];
				break;
			case 0x1e:
				I += V[(opcode & 0x0f00) >> 8];
				if(I > 0xfff){
					V[0xf] = 1;
				}else{
					V[0xf] = 0;
				}
				break;
			case 0x29:
				I = V[(opcode & 0x0F00) >> 8] * 5;
				break;
			case 0x33:
				memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
				memory[I + 1] = V[(opcode & 0x0F00) >> 8] / 10 % 10;
				memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
				break;
			case 0x55:
				for(int i=0; i<=((opcode & 0x0F00) >> 8); i++){
					memory[I + i] = V[i];
				}
				I += ((opcode & 0x0F00) >> 8) + 1;
				break;
			case 0x65:
				for(int i=0; i<=((opcode & 0x0F00) >> 8); i++){
					V[i] = memory[I + i];
				}
				I += ((opcode & 0x0F00) >> 8) + 1;
				break;
			default:
				printf("Unknow opcode[0xf000] : 0x%X", opcode);
			}
	}
	
	// Update timers
	if(delay_timer > 0){
		delay_timer--;
	}
	if(sound_timer > 0){
		if(sound_timer == 1){
			printf("Beep!\n");
		}
		sound_timer--;
	}
}
