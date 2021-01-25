#include "instructions.h"

// Returns the key indicated by the given integer value.
SDL_KeyCode val_to_key(int value) {
    switch(value) {
        case 0:
            return SDL_SCANCODE_1;
        case 1:
            return SDL_SCANCODE_2;
        case 2:
            return SDL_SCANCODE_3;
        case 3:
            return SDL_SCANCODE_4;
        case 4:
            return SDL_SCANCODE_Q;
        case 5:
            return SDL_SCANCODE_W;
        case 6:
            return SDL_SCANCODE_E;
        case 7:
            return SDL_SCANCODE_R;
        case 8:
            return SDL_SCANCODE_A;
        case 9:
            return SDL_SCANCODE_S;
        case 10:
            return SDL_SCANCODE_D;
        case 11:
            return SDL_SCANCODE_F;
        case 12:
            return SDL_SCANCODE_Z;
        case 13:
            return SDL_SCANCODE_X;
        case 14:
            return SDL_SCANCODE_C;
        case 15:
            return SDL_SCANCODE_V; 
        default:
            printf("ERROR: Invalid value %d\n", value);
    }
}

// Returns the corresponding V index of the given keyboard key
int key_to_v_register(SDL_KeyboardEvent event) {
    switch(event.keysym.sym) {
        case SDL_SCANCODE_1:
            return 0;
        case SDL_SCANCODE_2:
            return 1;
        case SDL_SCANCODE_3:
            return 2;
        case SDL_SCANCODE_4:
            return 3;
        case SDL_SCANCODE_Q:
            return 4;
        case SDL_SCANCODE_W:
            return 5;
        case SDL_SCANCODE_E:
            return 6;
        case SDL_SCANCODE_R:
            return 7;
        case SDL_SCANCODE_A:
            return 8;
        case SDL_SCANCODE_S:
            return 9;
        case SDL_SCANCODE_D:
            return 10;
        case SDL_SCANCODE_F:
            return 11;
        case SDL_SCANCODE_Z:
            return 12;
        case SDL_SCANCODE_X:
            return 13;
        case SDL_SCANCODE_C:
            return 14;
        case SDL_SCANCODE_V:
            return 15;
    }
}