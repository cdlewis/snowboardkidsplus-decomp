#include "game/graphics.h"

SpritePrimitiveTemplate gSpritePrimitiveTemplate;
GsSPRITE gRotatedSpriteTemplate __attribute__((section(".bss.rotated_sprite")));

TexturedSpritePacket gTexturedSpriteTemplate = {
    0x05000000, 0xE1000000,
    0x80, 0x80, 0x80, 0x64,
    0, 0, 0, 0, 0, 0, 0
};
