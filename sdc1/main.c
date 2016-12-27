#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


int lives = 3;
int score = 0;

typedef enum {
    PLAYER,
    ENEMY,
    PBULLET,
    EBULLET
} SpriteImage;

static const SDL_Rect sprite_rects[] = {
    {.x = 0, .y = 0, .w = 32, .h = 32},
    {.x = 32, .y = 0, .w = 32, .h = 32},
    {.x = 64, .y = 0, .w = 32, .h = 32},
    {.x = 80, .y = 0, .w = 32, .h = 32},
};

SDL_Texture *spritesheet;

typedef struct {
    SDL_Rect r;

    SpriteImage img;
} Sprite;

struct Vect;

typedef enum {
    T_PLAYER,
    T_ENEMY,
    T_PBULLET,
    T_EBULLET
} EType;

typedef void (*updatefunc)(struct Entity *self, Item *items, Uint8 *keyboard);

typedef struct Entity {
    Sprite s;
    updatefunc update;
    EType t;
    int offset;
} Entity;

typedef struct {
    Entity item;
    bool exists;

} Item;

Item items[1024];

void sprite_draw(SDL_Renderer *r, Sprite *spr) {
    Sprite s = *spr;
    SDL_Rect real_r = s.r;
    real_r.x -= real_r.w/2;
    real_r.y -= real_r.h/2;
    SDL_RenderCopy(r, spritesheet, &sprite_rects[s.img], &real_r);
}

// simple optimization, don't go further in the array than needed
int last_item = -1;

void init_items() {
    for (unsigned int i = 0; i < sizeof(items); i++) {
        items[i] = (Item){.exists = false};
    }
}

void items_add(Entity item) {
    int i = 0;
    for (; i < 1024 && !items[i].exists; i++);

    item.offset = i;
    items[i].item = item;
    
    items[i].exists = true;
}

void player_update(Entity *self, Item *items, Uint8 *keyboard) {
    static int h_speed = 0;
    static int v_speed = 0;
    // Player is always 0
    for (int i = 1; i <= last_item; i++) {
/*        if (items[i].exists)
            if (entity_collide(self, &(items[i])) && items[i].item.t != T_PBULLET) {
                if (!--lives)
                    done = true;
                    }*/
    }

    if ((!keyboard[SDL_SCANCODE_RIGHT] && !keyboard[SDL_SCANCODE_LEFT])
        || (keyboard[SDL_SCANCODE_RIGHT] && keyboard[SDL_SCANCODE_LEFT]))
        h_speed = 0;
    else
        h_speed = 5 * (keyboard[SDL_SCANCODE_RIGHT] ? 1 : -1);

    if ((!keyboard[SDL_SCANCODE_DOWN] && !keyboard[SDL_SCANCODE_UP])
        || (keyboard[SDL_SCANCODE_DOWN] && keyboard[SDL_SCANCODE_UP]))
        v_speed = 0;
    else
        v_speed = 5 * (keyboard[SDL_SCANCODE_DOWN] ? 1 : -1);

    ((Sprite*)self)->r.x += h_speed;
    ((Sprite*)self)->r.y += v_speed;
}

void items_delete(int offset)
{
    items[offset].exists = false;
    if (offset == last_item) {
        if (last_item == 0)
            last_item = -1;
        // We already made sure the last item isn't 0, so we're not empty
        else
            for (int i = last_item - 1; i > 0; i--)
                if (items[i].exists)
                    last_item = i;
    }
}

bool done = false;

int main(int argc, char **argv) {
    SDL_Window *win;
    SDL_Renderer *r;

    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    
    SDL_Event e;

    win = SDL_CreateWindow("Batty", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                           800, 600, 0);

    r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    spritesheet = IMG_LoadTexture(r, "spritesheet.png");
    init_items();

    items_add((Entity){
            .s=(Sprite){.img=PLAYER,
                        .r=(SDL_Rect){.x = 400, .y = (600-32), .w = 1, .h = 1}},
            .update = player_update,
            .t = T_PLAYER});
    
    while (!done) {
        SDL_PollEvent(&e);
        const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
        if (e.type == SDL_QUIT){
            done = true;
            continue;
        }
        
        for (int i = last_item; i >= 0; i--) {
            if (items[i].exists) {
                sprite_draw(r, &((Sprite)items[i]));
                items[i].update(&(items[i]), items, keyboard);

            }
        }
    }
}
