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

    stream compressed_data = {
        0, 0, 0, false, NULL, NULL
    };

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
    u8 FLEVEL   = (idat_header->_FLG >> 6);
    assert(CM == 8 && FDICT == 0);

    printf("    CM: %u\n", CM);
    printf("    CINFO: %u\n", CINFO);
    printf("    FCHECK: %u\n", FCHECK);
    printf("    FDICT: %u\n", FDICT);
    printf("    FLEVEL: %u\n", FLEVEL);

    u32 BFINAL = 0;
    while (BFINAL == 0) {
        BFINAL = consume_bits(&compressed_data, 1);
        u32 BTYPE = consume_bits(&compressed_data, 2);

        if (BTYPE == 3) {
            fprintf(stderr, BHRED "ERROR" CRESET ": Invalid BTYPE.\n");
        } 
        else if (BTYPE == 0) {
            flush_byte(&compressed_data);
            u16 LEN = (u16)consume_bits(&compressed_data, 16);
            u16 NLEN = (u16)consume_bits(&compressed_data, 16);
            assert(~LEN == NLEN);
            // TODO: Consume LEN bytes of uncompressed data.
        } 
        else if (BTYPE == 1) {
        } 
        else if (BTYPE == 2) {
            u32 HLIT  = consume_bits(&compressed_data, 5) + 257;
            u32 HDIST = consume_bits(&compressed_data, 5) + 1;
            u32 HCLEN = consume_bits(&compressed_data, 4) + 4;
            
            const u8 CL_symbols[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
            assert(HCLEN <= sizeof(CL_symbols));
            u32 CL_table[sizeof(CL_symbols)];
            for (u32 i = 0; i < HCLEN; i++) {
                CL_table[CL_symbols[i]] = consume_bits(&compressed_data, 3);
            }
        }



    }

    return(result);
}
