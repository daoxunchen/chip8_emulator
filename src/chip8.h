#ifndef CHIP8_H
#define CHIP8_H

typedef int bool;
#define true 1
#define false 0

bool drawFlag;
extern unsigned char gfx[64 * 32];
extern unsigned char key[16];

void initialize(void);
int loadGame(char *filename);
void emulateCycle(void);

#endif
