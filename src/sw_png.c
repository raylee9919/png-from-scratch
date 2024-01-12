#include "sw_png.h"
#include "sw_stream.h"
#include <stdio.h>


image_u32 ParsePNG (stream file)
{
    image_u32 result;
    u32 width;
    u32 height;
    u8* pixels;

    stream* at = &file;
    
    const u8 png_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    png_header* file_header = consume(at, png_header);
    assert(file_header && !memcmp(file_header->_signature, png_signature, 8));

    stream compressed_data = {0, 0, 0, false, NULL, NULL};

    while (at->_contents._count > 0)
    {
        png_chunk_header* chunk_header = consume(at, png_chunk_header);
        assert(chunk_header);
        swap_endian_32(&chunk_header->_length);

        void* chunk_data = consume_size(at, chunk_header->_length);

        png_chunk_footer* chunk_footer = consume(at, png_chunk_footer);
        swap_endian_32(&chunk_footer->_CRC);


        for (u8 i = 0; i < 4; i++)
            fputc(chunk_header->_type[i], stdout);
        fputc('\n', stdout);

        if (!memcmp(chunk_header->_type, "IHDR", 4))
        {
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

        }
        else if (!memcmp(chunk_header->_type, "PLTE", 4)) 
        {
            assert(chunk_header->_length % 3 == 0);
        }
        else if (!memcmp(chunk_header->_type, "IDAT", 4))
        {
            append_chunk(&compressed_data, chunk_data, chunk_header->_length);
        }
        else if (!memcmp(chunk_header->_type, "IEND", 4))
        {
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
    while (BFINAL == 0)
    {
        BFINAL      = consume_bits(&compressed_data, 1);
        u32 BTYPE   = consume_bits(&compressed_data, 2);

        if (BTYPE == RESERVED)
        {
            fprintf(stderr, BHRED "ERROR" CRESET ": Invalid BTYPE.\n");
        } 
        else if (BTYPE == NON_COMPRESSED)
        {
            flush_byte(&compressed_data);
            u16 LEN     = (u16)consume_bits(&compressed_data, 16);
            u16 NLEN    = (u16)consume_bits(&compressed_data, 16);
            assert(~LEN == NLEN);
            // TODO: Consume LEN bytes of uncompressed data.
        } 
        else if (BTYPE == FIXED_HUFFMAN)
        {
        } 
        else if (BTYPE == DYNAMIC_HUFFMAN)
        {
            u32 HLIT  = consume_bits(&compressed_data, 5) + 257;
            u32 HDIST = consume_bits(&compressed_data, 5) + 1;
            u32 HCLEN = consume_bits(&compressed_data, 4) + 4;
            
            const u8 CL_symbols[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
            assert(HCLEN <= LEN(CL_symbols));

            u32 CL_array[LEN(CL_symbols)];
            memset(CL_array, 0, sizeof(CL_array));

            u32 symblCnt = 0;
            for (u32 i = 0; i < HCLEN; i++) 
            {
                u32 bits = consume_bits(&compressed_data, 3);
                if (bits != 0)
                    symblCnt++;
                CL_array[CL_symbols[i]] = bits;
            }

            Table clTable = {
                CL_CODE_LENGTH, symblCnt,
                (Entry*)malloc(symblCnt * sizeof(Table))
            };

            Entry* entry = clTable._entries;
            for (u32 symbol = 0; symbol < LEN(CL_symbols); symbol++) 
            {
                if (CL_array[symbol] != 0)
                {
                    entry->_symbol = symbol;
                    entry->_code   = CL_array[symbol];
                    entry++;
                }
            }

            CLToCode(&clTable);
            u32* clDecodeTable = BuildDecodeTable(&clTable);
            free(clTable._entries);

            // TODO: Decompress compressed LL and D code table,
            // using HLIT and HDIST
            

            free(clDecodeTable);

            // TODO: Decompress rest of the data.
            // You get Literal and (Length, Distance).


            // TODO: Get Filtered array.


            // TODO: Remove the corresponding filter.

            // TODO: If it uses PLTE, get the color.
            

            // CONGRATS!

        }



    }

    return(result);
}

void CLToCode(Table* table)
{
    // NOTE: 1. Count the number of codes for each code length.
    u32 len_cnt[MAX_CODE_LENGTH];
    memset(len_cnt, 0, sizeof(len_cnt));
    for (int i = 0; i < table->_entry_count; i++)
    {
        len_cnt[table->_entries[i]._code]++;
    }
    len_cnt[0] = 0;

    // NOTE: 2. Find the numerical value of the smallest code for each code length.
    u32 high = 0;
    u32 prev = 0;
    u32 low[MAX_CODE_LENGTH];
    memset(low, 0, sizeof(low));
    for (u32 len = 1; len <= MAX_CODE_LENGTH; len++)
    {
        if (len_cnt[len] == 0)
            continue;
        low[len] = (high << (len - prev));
        high = low[len] + len_cnt[len];
        prev = len;
    }

    // NOTE: 3. Assign numerical valuse to all codes.
    for (u32 i = 0; i < table->_entry_count; i++) {
        u32 key = table->_entries[i]._code;
        table->_entries[i]._code = low[key];
        low[key]++;
    }
}

u32* BuildDecodeTable(Table* table)
{
    u32 high = 0;
    for (int i = 0; i < table->_entry_count; i++)
    {
        if (high < table->_entries[i]._code)
        {
            high = table->_entries[i]._code;
        }
    }

    u32 size = ((high + 1) * sizeof(u32));
    u32* result = (u32*)malloc(size);
    memset(result, UINT_MAX, size);

    for (int i = 0; i < table->_entry_count; i++)
    {
        u32 symbol  = table->_entries[i]._symbol;
        u32 code    = table->_entries[i]._code;
        u32 val = result[code];
        while (code <= high && result[code] == val)
        {
            result[code] = symbol;
            code++;
        }
    }

    return result;
}
