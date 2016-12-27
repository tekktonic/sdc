#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/************************************* Constants ************************************/
static const SDL_Rect sprite_rects[] = {
    {.x = 0, .y = 0, .w = 32, .h = 32},
    {.x = 32, .y = 0, .w = 32, .h = 32},
    {.x = 78, .y = 14, .w = 5, .h = 5},
    {.x = 110, .y = 14, .w = 5, .h = 5},
    {.x = 128, .y = 0, .w = 32, .h = 32},
};

static const SDL_Rect collision_boxes[] = {
    {.x = 0, .y = 0, .w = 8, .h = 8},
    {.x = 0, .y = 0, .w = 32, .h = 32},
    {.x = 0, .y = 0, .w = 5, .h = 5},
};

typedef enum {
    T_SHIP,
    T_BULLET
} EType;

typedef enum {
    A_PLAYER,
    A_ENEMY
} Allegiance;

typedef enum {
    PLAYER,
    ENEMY,
    PBULLET,
    EBULLET,
    GRASS
} SpriteImage;

/*************************************** Types **************************************/
struct Entity;
struct Item;

typedef void (*updatefunc)(struct Entity *self, const Uint8 *keyboard);

typedef struct {
    SDL_Rect r;

    SpriteImage img;
} Sprite;

typedef struct Entity {
    Sprite s;
    updatefunc update;
    EType t;
    Allegiance a;
    SDL_Rect hitbox;
    void *extra;
    void (*cleanup)(struct Entity *self);
    int offset;
} Entity;


typedef struct PlayerInfo {
    int h_speed;
    int v_speed;
    int countdown;
};

typedef struct Item {
    Entity item;
    bool exists;
} Item;

/********************************* Global variables *********************************/
int lives = 3;
int score = 0;
SDL_Texture *spritesheet;
bool done = false;
// simple optimization, don't go further in the array than needed
int last_item = -1;

Item items[1024] = {};

// Correct for the fact that the sprite's "position" is its middle
Sprite background = {.r={.x=400,.y=300,.w=800,.h=600},.img=GRASS};

/************************************* Code *****************************************/
void draw_sprite(SDL_Renderer *r, Sprite *spr);

void init_enemy(Entity *enemy);
void init_player(Entity *player);

void items_add(Entity item);
void items_delete(int offset);

void update_bullet(Entity *self, const Uint8 *keyboard);
void update_enemy(Entity *self, const Uint8 *keyboard);
void update_player(Entity *self,  const Uint8 *keyboard);


int main(int argc, char **argv) {
    SDL_Window *win;
    SDL_Renderer *r;

    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    srand(time(NULL));
    SDL_Event e;

    win = SDL_CreateWindow("Batty", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                           800, 600, 0);

    r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    spritesheet = IMG_LoadTexture(r, "spritesheet.png");

    items_add((Entity){
            .s=(Sprite){.img=PLAYER,
                        .r=(SDL_Rect){.x = 400, .y = 300, .w = 32, .h = 32}},
            .update = update_player,
             .t = T_SHIP,
                 .a = A_PLAYER});

    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT){
                done = true;
                goto end;
            }

        }
        
        const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
        draw_sprite(r, &background);
        for (int i = last_item; i >= 0; i--) {
            if (items[i].exists) {
                draw_sprite(r, (Sprite*)&(items[i]));
                Entity e = *((Entity*)&(items[i]));
                e.update(((Entity*)&(items[i])),  keyboard);

            }
        }

        SDL_RenderPresent(r);

        SDL_Delay(1000/60.);
    }

end:
    SDL_DestroyTexture(spritesheet);
    for(int i = 0; i < last_item; i++)
    {
        if (items[i].exists) {
            items_delete(i);
        }
    }
    IMG_Quit();
    SDL_Quit();
    return 0;
}

void draw_sprite(SDL_Renderer *r, Sprite *spr) {
    Sprite s = *spr;
    SDL_Rect real_r = s.r;
    real_r.x -= real_r.w/2;
    real_r.y -= real_r.h/2;
    SDL_RenderCopy(r, spritesheet, &sprite_rects[s.img], &real_r);
}

void init_enemy(Entity *self) {
    
}

void init_player(Entity *self) {
    
}

void items_add(Entity item) {
    int i = 0;
    for (; i < 1024 && items[i].exists; i++);

    item.offset = i;
    items[i].item = item;
    
    items[i].exists = true;

    if (i > last_item)
        last_item = i;
}

void items_delete(int offset)
{
    Entity *item = (Entity*)&items[offset];
    if (item->cleanup) {
        item->cleanup(item);
    }
    
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

void update_bullet(Entity *self, const Uint8 *keyboard) {
    if (self->s.r.x < 0 || self->s.r.x > 800 || self->s.r.y < 0 || self->s.r.y > 600)
        items_delete(self->offset);
/*    for (int i = 0; i <= last_item; i++) {
        Entity *e = (Entity*)&(items[i]);
        if (items[i].exists)
            if (entity_collide(self, e) && self->a != e->a) {
                items_delete(self->offset);
            }
            }*/


    self->s.r.y += 8 * (self->a == A_ENEMY ? 1 : -1);
}

void update_enemy(Entity *self, const Uint8 *keyboard) {
    if ( self->s.r.y > 800)
        items_delete(self->offset);

    self->s.r.y += 5;
    SDL_Rect r = self->s.r;
    if (*(int*)self->extra <= 0) {
        items_add(
            (Entity){.s=(Sprite){.img=EBULLET, .r=(SDL_Rect){.x = r.x, .y=r.y-(r.w/2),.w=5,.h=5}},
                    .update = update_bullet, .t = T_BULLET, .a = A_ENEMY});
        (*(int*)self->extra) = (random() % 45) + 1;
    }

    (*(int*)self->extra)--;
}

void update_player(Entity *self, const Uint8 *keyboard) {
    static int h_speed = 0;
    static int v_speed = 0;

    static int cooldown = 0;
    // Player is always 0
/*    for (int i = 1; i <= last_item; i++) {
        if (items[i].exists)
            if (entity_collide(self, &(items[i])) && items[i].item.t != T_PBULLET) {
                if (!--lives)
                    done = true;
            }
            }*/
    
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


    if (keyboard[SDL_SCANCODE_Z] && !cooldown) {
        SDL_Rect r = self->s.r;
        cooldown = 45;
        items_add(
            (Entity){.s=(Sprite){.img=PBULLET, .r=(SDL_Rect){.x = r.x-2, .y=r.y,.w=5,.h=5}},
                .update = update_bullet, .t = T_BULLET, .a = A_PLAYER});
    }
//    printf("Before: %d %d\n", self->s.r.x, self->s.r.y);
    ((Sprite*)self)->r.x += h_speed;
    ((Sprite*)self)->r.y += v_speed;

    if (cooldown > 0)
        cooldown--;
//    printf("After: %d %d\n", self->s.r.x, self->s.r.y);
}

