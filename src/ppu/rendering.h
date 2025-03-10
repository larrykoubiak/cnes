#include <stdint.h>

typedef struct PPU PPU;

void evaluate_sprites(PPU* ppu);
void render_sprites(PPU* ppu);
void render_tile(PPU* ppu, uint8_t tile_id, uint8_t palette_id, int palette_table_id, int pattern_table_id, uint8_t* output_buffer );
void render_nametable(PPU* ppu, uint8_t nametable_id, uint8_t* output_buffer);
void render_patterntable(PPU* ppu, int table_id, uint8_t* output_buffer);
void render_palettetable(PPU* ppu, uint8_t* output_buffer);
uint16_t get_color_address(PPU* ppu, int pal_table_id, uint8_t pal_id, uint8_t pixel );