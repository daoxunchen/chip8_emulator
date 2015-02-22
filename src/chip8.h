#ifndef CHIP8_H
#define CHIP8_H

bool drawFlag;

void initialize(void);
int loadGame(char *filename);
void emulateCycle(void);
void drawGfx();
void setKeys();

#endif

