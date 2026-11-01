#include "common.h"

extern int _new_card(int), _card_write(int, int, void *);

int _card_clear(int channel) {
    _new_card(channel);
    return _card_write(channel, 0x3f, 0);
}
