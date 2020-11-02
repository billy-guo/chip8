#include "instructions.h"

// Returns the key indicated by the given integer value.
SDL_KeyCode val_to_key(int value) {
    switch(value) {
        case 0:
            return SDLK_1;
        case 1:
            return SDLK_2;
        case 2:
            return SDLK_3;
        case 3:
            return SDLK_4;
        case 4:
            return SDLK_q;
        case 5:
            return SDLK_w;
        case 6:
            return SDLK_e;
        case 7:
            return SDLK_r;
        case 8:
            return SDLK_a;
        case 9:
            return SDLK_s;
        case 10:
            return SDLK_d;
        case 11:
            return SDLK_f;
        case 12:
            return SDLK_z;
        case 13:
            return SDLK_x;
        case 14:
            return SDLK_c;
        case 15:
            return SDLK_v; 
        default:
            printf("ERROR: Invalid value %d\n", value);
    }
}

// Returns the corresponding V index of the given keyboard key
int key_to_v_register(SDL_KeyboardEvent event) {
    switch(event.keysym.sym) {
        case SDLK_1:
            return 0;
        case SDLK_2:
            return 1;
        case SDLK_3:
            return 2;
        case SDLK_4:
            return 3;
        case SDLK_q:
            return 4;
        case SDLK_w:
            return 5;
        case SDLK_e:
            return 6;
        case SDLK_r:
            return 7;
        case SDLK_a:
            return 8;
        case SDLK_s:
            return 9;
        case SDLK_d:
            return 10;
        case SDLK_f:
            return 11;
        case SDLK_z:
            return 12;
        case SDLK_x:
            return 13;
        case SDLK_c:
            return 14;
        case SDLK_v:
            return 15;
    }
}