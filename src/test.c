#include <stdio.h>
#include <stdlib.h>

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

int main() {
    stream file = read_entire_file("sample.png");
    image_u32 image = ParsePNG(file);
    free(image._pixels);


    return 0;
}
