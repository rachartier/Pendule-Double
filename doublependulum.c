#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>

#include <math.h>

#include <SDL2/SDL.h>

#define WINDOW_WIDTH  720
#define WINDOW_HEIGHT 720

#define G             9.80665f

struct pendulum_t {
    double mass;
    double lenght;
    
    double th;
    double th_d;
    
    SDL_Point coord;
};

typedef struct pendulum_t pendulum_t;

pendulum_t PendulumCreate(double mass, double lenght, double th, double th_d) {
    pendulum_t p;
    
    p.mass    = mass;
    p.lenght  = lenght;
    p.th      = th;
    p.th_d    = th_d;
    
    p.coord.x = 0;
    p.coord.y = 0;
    
    return p;
}

double ThetaFirstPendulum(pendulum_t p1, pendulum_t p2) {
    double m = p1.mass + p2.mass;
    
    double a = p2.mass * p1.lenght * p1.th_d * p1.th_d * sin(p2.th - p1.th) * cos(p2.th - p1.th);
    double b = p2.mass * G * sin(p2.th) * cos(p2.th - p1.th);
    double c = p2.mass * p2.lenght * p2.th_d * p2.th_d * sin(p2.th - p1.th);
    double d = -m * G * sin(p1.th);
    
    double e = (m * p1.lenght) - p2.mass * p1.lenght * cos(p2.th - p1.th) * cos(p2.th - p1.th);
    
    return (a + b + c + d) / e;

}

double ThetaSecondPendulum(pendulum_t p1, pendulum_t p2) {
    double m = p1.mass + p2.mass;
    
    double a = -p2.mass * p2.lenght * p2.th_d * p2.th_d * sin(p2.th - p1.th) * cos(p2.th - p1.th);
    double b = m * (G * sin(p1.th) * cos(p2.th - p1.th) - p1.lenght * p1.th_d * p1.th_d * sin(p2.th - p1.th) - G * sin(p2.th));
    double c = (m * p2.lenght) - p2.mass * p2.lenght * cos(p2.th - p1.th) * cos(p2.th - p1.th);
    
    return (a + b) / c;
}

int ThFirstPendulumToX(pendulum_t p1) {
    return (int)(WINDOW_WIDTH / 2 - sin(p1.th) * p1.lenght);
}

int ThFirstPendulumToY(pendulum_t p1) {
    return (int)(WINDOW_HEIGHT / 2 + cos(p1.th) * p1.lenght);
}

int ThSecondPendulumToX(pendulum_t p1, pendulum_t p2) {
    return (int)(ThFirstPendulumToX(p1) - sin(p2.th) * p2.lenght);
}

int ThSecondPendulumToY(pendulum_t p1, pendulum_t p2) {
    return (int)(ThFirstPendulumToY(p1) + cos(p2.th) * p2.lenght);
}

void CalcPosition(pendulum_t *p1, pendulum_t *p2) {
    p1->coord.x = (int)ThFirstPendulumToX(*p1);
    p1->coord.y = (int)ThFirstPendulumToY(*p1);
    
    p2->coord.x = (int)ThSecondPendulumToX(*p1, *p2);
    p2->coord.y = (int)ThSecondPendulumToY(*p1, *p2);
}

void DrawPendulums(SDL_Renderer *renderer, pendulum_t p1, pendulum_t p2) {    
    int x1 = p1.coord.x;
    int y1 = p1.coord.y;
    
    int x2 = p2.coord.x;
    int y2 = p2.coord.y;
    
    int xoffset = 1;
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    SDL_Point points[3] = {
        {WINDOW_WIDTH/2, WINDOW_HEIGHT/2},
        {x1,    y1},
        {x2,    y2}
    };
    
    SDL_Point points2[3] = {
        {WINDOW_WIDTH/2 + xoffset, WINDOW_HEIGHT/2},
        {x1 + xoffset,    y1},
        {x2 + xoffset,    y2}
    };
    
    SDL_RenderDrawLines(renderer, points, 3);
    SDL_RenderDrawLines(renderer, points2, 3);
}

void DrawPath(SDL_Renderer *renderer, pendulum_t p) {
    static SDL_Point point[2048] = {0};
    static int       i = 0;
    
    point[i].x = p.coord.x;
    point[i].y = p.coord.y;
    
    if(++i > 2048)
        i = 0;
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLines(renderer, point, i);
}

void AnimatePendulums(pendulum_t *p1, pendulum_t *p2) {
    float dt = 0.05;
    
    p1->th_d += ThetaFirstPendulum(*p1, *p2) * dt;
    p2->th_d += ThetaSecondPendulum(*p1, *p2) * dt;
    
    p1->th += p1->th_d * dt;
    p2->th += p2->th_d * dt;
}

float RandFloat(float min, float max) {
    return min + (((float)rand() / (float)(RAND_MAX / (max - min))));
}

void PrintPendulumInfo(pendulum_t p, int i) {
    printf("Pendulum %d:", i);
    
    printf("\n\t- Lenght: %.3lf\n", p.lenght);
    printf("\t- Mass: %.3lf\n\n", p.mass);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    
    SDL_Window   *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Event     event;
    
    bool          running = true;
 
    assert(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, 
                                       &window, &renderer) == 0);
 
    srand(time(NULL));
    
    pendulum_t p1 = PendulumCreate(100, 120, RandFloat(0, 1) * M_PI * 2, (RandFloat(0, 1) - 0.5) * 2);
    pendulum_t p2 = PendulumCreate(100, 120, RandFloat(0, 1) * M_PI * 2, (RandFloat(0, 1) - 0.5) * 2);
    
    while(running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
            }
            
            switch(event.type) {
                case SDL_QUIT:
                running = false;
                break;
                case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                case SDLK_a:
                    p1.lenght -= 1;
                    break;
                case SDLK_z:
                    p1.lenght += 1;
                    break;
                case SDLK_e:
                    p2.lenght -= 1;
                    break;
                case SDLK_r:
                    p2.lenght += 1;
                    break;
                case SDLK_q:
                    p1.mass -= 5;
                    break;
                case SDLK_s:
                    p1.mass += 5;
                    break;
                case SDLK_d:
                    p2.mass -= 5;
                    break;
                case SDLK_f:
                    p2.mass += 5;
                    break;
                case SDLK_n:
                    p1 = PendulumCreate(100, 120, RandFloat(0, 1) * M_PI * 2, (RandFloat(0, 1) - 0.5) * 2);
                    p2 = PendulumCreate(100, 120, RandFloat(0, 1) * M_PI * 2, (RandFloat(0, 1) - 0.5) * 2);
                    break;
                case SDLK_i:
                    PrintPendulumInfo(p1, 1);
                    PrintPendulumInfo(p2, 2);
                    break;
                }
                break;
            }
        }
    
        AnimatePendulums(&p1, &p2);        
       
        CalcPosition(&p1, &p2);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        
        DrawPendulums(renderer, p1, p2);
        DrawPath(renderer, p2);
        
        SDL_RenderPresent(renderer);
    
        SDL_Delay(5);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    SDL_Quit();
    return 0;
}
