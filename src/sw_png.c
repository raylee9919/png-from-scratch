#include "sw_png.h"
#include "sw_stream.h"

const u8 png_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

image_u32 parse_png (stream file) {
    image_u32 result = {};
    u32 width;
    u32 height;
    u8* pixels;

    stream* at = &file;
    
    png_header* file_header = consume(at, png_header);
    assert(file_header);
    for (u8 i = 0; i < 8; i++) {
        if (memcmp(file_header->_signature, png_signature, 8)) {
            fprintf(stderr, BHRED "ERROR" CRESET ": Invalid PNG_SIGNATURE.\n");
        }
    }

    stream compressed_data;
    compressed_data._contents._count = 0;
    compressed_data._contents._data = NULL;
    compressed_data._first = NULL;
    compressed_data._last = NULL;
    compressed_data._underflowed = false;

    while (at->_contents._count > 0) {
        png_chunk_header* chunk_header = consume(at, png_chunk_header);
        assert(chunk_header);
        swap_endian_32(&chunk_header->_length);

        void* chunk_data = consume_size(at, chunk_header->_length);

        png_chunk_footer* chunk_footer = consume(at, png_chunk_footer);
        swap_endian_32(&chunk_footer->_CRC);


        for (u8 i = 0; i < 4; i++) {
            fputc(chunk_header->_type[i], stdout);
        } fputc('\n', stdout);

        if (!memcmp(chunk_header->_type, "IHDR", 4)) {
            png_ihdr* ihdr = (png_ihdr*)chunk_data;

            swap_endian_32(&ihdr->_width);
            swap_endian_32(&ihdr->_height);

            fprintf(stdout, "    Width: %u\n", ihdr->_width);
            fprintf(stdout, "    Height: %u\n", ihdr->_height);
            fprintf(stdout, "    BitDepth: %u\n", ihdr->_bitdepth);
            fprintf(stdout, "    ColorType: %u\n", ihdr->_colortype);
            fprintf(stdout, "    CompressionMethod: %u\n", ihdr->_compressionmethod);
            fprintf(stdout, "    FilterMethod: %u\n", ihdr->_filtermethod);
            fprintf(stdout, "    InterlaceMethod: %u\n", ihdr->_interlacemethod);

        } else if (!memcmp(chunk_header->_type, "PLTE", 4)) {
            assert(chunk_header->_length % 3 == 0);

        } else if (!memcmp(chunk_header->_type, "IDAT", 4)) {
            append_chunk(&compressed_data, chunk_data, chunk_header->_length);


        } else if (!memcmp(chunk_header->_type, "IEND", 4)) {

        }



    }
    printf(BHGRN "OK" CRESET ": PNG parsing completed.\n");
    printf("zlibHeader\n");
    png_idat_header* idat_header = consume(&compressed_data, png_idat_header);

    u8 CM       = (idat_header->_CMP & 0xF);
    u8 CINFO    = (idat_header->_CMP >> 4);
    u8 FCHECK   = (idat_header->_FLG & 0x1F);
    u8 FDICT    = (idat_header->_FLG >> 5) & 0x1;
    assert(FDICT == 0);
    u8 FLEVEL   = (idat_header->_FLG >> 6);

    printf("    CM: %u\n", CM);
    printf("    CINFO: %u\n", CINFO);
    printf("    FCHECK: %u\n", FCHECK);
    printf("    FDICT: %u\n", FDICT);
    printf("    FLEVEL: %u\n", FLEVEL);

    return(result);
}
