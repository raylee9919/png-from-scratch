#include <SDL2/SDL_events.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "sw_png.h"


stream read_entire_file(char* filename) {
    buffer buf = {};
    
    FILE* in = fopen(filename, "rb");
    if(in) {
        fseek(in, 0, SEEK_END);
        buf._count = ftell(in);
        fseek(in, 0, SEEK_SET);
        
        buf._data = (u8*)malloc(buf._count);
        fread(buf._data, buf._count, 1, in);
        fclose(in);
    }
    
    stream result = make_read_stream(buf);
    if(!in) {
        fprintf(stderr, "ERRORT : Cannot open file %s.\n", filename);
    }
    
    return(result);
}

void RenderImageU32(const image_u32* image) {
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);

    SDL_Window* window = SDL_CreateWindow("PNG test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, image->_width, image->_height, SDL_WINDOW_SHOWN);
    assert(window != NULL);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    assert(renderer != NULL);

    SDL_RenderClear(renderer);

    u32 width = image->_width;
    u32 height = image->_height;

    printf("Width: %u\n", width);
    printf("Height: %u\n", height);

    b32 quit = false;
    SDL_Event event;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        u8* sample = (u8*)image->_pixels;
        for (u32 y = 0; y < height; y++)
        {
            for (u32 x = 0; x < width; x++)
            {
                u8 r = *sample++;
                u8 g = *sample++;
                u8 b = *sample++;
                u8 a = *sample++;
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    stream file = read_entire_file("img/img1.png");
    image_u32 image = ParsePNG(file);

    RenderImageU32(&image);

    free(image._pixels);
    return 0;
}
