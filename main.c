#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define PI 3.1415927f
#define MINUS_PI -3.1415927f
#define TAU 6.2831854f
#define DEG_TO_RAD 0.017453292f
#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define ROTATE_SPEED 90.0f
#define PLAYER_SPEED 1.0f
#define FOV 60.0f
#define RENDER_DIST 5.0f
#define WALL_MODIFIER 10.0f

typedef struct {
    float x;
    float y;
} posf;

const bool map[MAP_WIDTH][MAP_HEIGHT] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
    const float rotate_speed = ROTATE_SPEED * DEG_TO_RAD, half_fov = FOV / 2.0f * DEG_TO_RAD,
    pixel_angle = (FOV / WINDOW_WIDTH) * DEG_TO_RAD, ray_step = RENDER_DIST / 256.0f,
    window_widthf = WINDOW_WIDTH, window_heightf = WINDOW_HEIGHT,
    map_widthf = MAP_WIDTH, map_heightf = MAP_HEIGHT;

    const int half_height = WINDOW_HEIGHT / 2;
    float dt = 0.0f, player_angle = 0.0f;
    posf player_pos;
    player_pos.x = 5.0f;
    player_pos.y = 5.0f;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("3D Maze", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    bool done = false;
    Uint64 last_time = 0, current_time;

    while (!done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        const bool *key_states = SDL_GetKeyboardState(NULL);

        if (key_states[SDL_SCANCODE_LEFT]) {
            player_angle -= rotate_speed * dt;
        }

        if (key_states[SDL_SCANCODE_RIGHT]) {
            player_angle += rotate_speed * dt;
        }

        while (player_angle < MINUS_PI || player_angle > PI) {
            if (player_angle < MINUS_PI) {
                player_angle += TAU;
            }
            else {
                player_angle -= TAU;
            }
        }

        const posf last_player_pos = player_pos;

        if (key_states[SDL_SCANCODE_W]) {
            player_pos.x += PLAYER_SPEED * sinf(player_angle) * dt;
            player_pos.y -= PLAYER_SPEED * cosf(player_angle) * dt;
        }

        if (key_states[SDL_SCANCODE_S]) {
            player_pos.x -= PLAYER_SPEED * sinf(player_angle) * dt;
            player_pos.y += PLAYER_SPEED * cosf(player_angle) * dt;
        }

        if (key_states[SDL_SCANCODE_A]) {
            player_pos.x -= PLAYER_SPEED * cosf(player_angle) * dt;
            player_pos.y -= PLAYER_SPEED * sinf(player_angle) * dt;
        }

        if (key_states[SDL_SCANCODE_D]) {
            player_pos.x += PLAYER_SPEED * cosf(player_angle) * dt;
            player_pos.y += PLAYER_SPEED * sinf(player_angle) * dt;
        }

        if (player_pos.x < 0.0f) {
            player_pos.x = 0.0f;
        }

        if (player_pos.x > map_widthf) {
            player_pos.x = map_widthf;
        }

        if (player_pos.y < 0.0f) {
            player_pos.y = 0.0f;
        }

        if (player_pos.y > map_heightf) {
            player_pos.y = map_heightf;
        }

        if (map[(int) player_pos.y][(int) player_pos.x]) {
            if (!map[(int) player_pos.y][(int) last_player_pos.x]) {
                player_pos.x = last_player_pos.x;
            }
            else if (!map[(int) last_player_pos.y][(int) player_pos.x]) {
                player_pos.y = last_player_pos.y;
            }
            else {
                player_pos = last_player_pos;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0U, 0U, 0U, 255U);
        SDL_RenderClear(renderer);
        float ray_angle = player_angle - half_fov;

        for (int i = 0; i < WINDOW_WIDTH; i++) {
            const posf ray_direction = {ray_step * sinf(ray_angle), ray_step * cosf(ray_angle)};
            posf ray = player_pos;
            int range;

            for (range = 0; range < 256; range++) {
                ray.x += ray_direction.x;
                ray.y -= ray_direction.y;

                if (ray.x < 0.0f || ray.x >= map_widthf || ray.y < 0.0f || ray.y >= map_heightf) {
                    break;
                }
                else {
                    if (map[(int) ray.y][(int) ray.x]) {
                        break;
                    }
                }
            }

            int render_color = 255 - range;

            if (render_color < 0) {
                render_color = 0;
            }

            SDL_SetRenderDrawColor(renderer, render_color, render_color, render_color, 255U);
            const float wall_height = window_heightf / (range * cosf(ray_angle - player_angle)) * WALL_MODIFIER;
            SDL_RenderLine(renderer, i, half_height - wall_height, i, half_height + wall_height);
            ray_angle += pixel_angle;
        }

        SDL_RenderPresent(renderer);

        current_time = SDL_GetTicks();
        dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}