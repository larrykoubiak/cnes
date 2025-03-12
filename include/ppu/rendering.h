#include <stdint.h>

typedef struct PPU PPU;

void ppu_step(PPU* ppu);
void ppu_trigger_nmi(PPU* ppu);
void render_scanline(PPU* ppu);
void evaluate_sprites(PPU* ppu);
void render_sprites(PPU* ppu);
