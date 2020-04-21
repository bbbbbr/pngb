#include "lodepng.h"

// #include "tilemap_write.h"
// #include "tilemap_read.h"
#include "tilemap_error.h"

#include "lib_tilemap.h"
#include "lib_gbr.h"
#include "lib_gbm.h"

#include "lib_rom_bin.h"

#include "image_info.h"
#include "options.h"



int tilemap_load_image(image_data * p_src_image, color_data * p_src_colors, const char * filename) {

    int status = true;
    unsigned int err, c;
    unsigned char * p_png = NULL;
    unsigned int width, height;
    unsigned int color_count = 0;
    long image_size_bytes = 0;
    size_t pngsize;
    LodePNGState state;


    // Initialize png library
    lodepng_state_init(&state);

    // Load the PNG file
    lodepng_load_file(&p_png, &pngsize, filename);

    // Zero out buffer pointer
    p_src_image->p_img_data = NULL;

    // Decode it with conversion disabled
    state.decoder.color_convert = 0;
    err = lodepng_decode(&(p_src_image->p_img_data), &width, &height, &state, p_png, pngsize);
    free(p_png);

    // Fail on errors and Require indexed color
    if (err) {
        status = false;
        printf("ERROR %u: %s\n", err, lodepng_error_text(err));
    }
    if (state.info_png.color.colortype != LCT_PALETTE) {
        status = false;
        printf("ERROR: PNG colortype 3 (indexed, 256 colors max) expected!");
    }
    if (state.info_png.color.bitdepth  > 8) {
        status = false;
        printf("ERROR: Decoded image must be 8 bits per pixel");
    }

    if (status) {

        // == IMAGE PROPERTIES from decoded PNG ==
        // Determine the array size for the app's image then allocate it
        p_src_image->bytes_per_pixel = (unsigned char)(state.info_png.color.bitdepth / 8); // Should only be 1 byte per pixel at this point
        p_src_image->width      = width;
        p_src_image->height     = height;
        p_src_image->size       = image_size_bytes * p_src_image->bytes_per_pixel;
        // p_src_image->p_img_data = malloc(p_src_image->size);  // lodepng_decode() handles allocation

        // == COLOR DATA ==
        // Load color palette info and data
        p_src_colors->color_count = state.info_png.color.palettesize;

        // Load colors, stripping out the Alpha component of the RGBA palette
        for (c=0; c < p_src_colors->color_count; c++){
            p_src_colors->pal[c * 3    ] = (unsigned char)state.info_png.color.palette[c*4];   /* R */
            p_src_colors->pal[c * 3 + 1] = (unsigned char)state.info_png.color.palette[c*4+1]; /* G */
            p_src_colors->pal[c * 3 + 2] = (unsigned char)state.info_png.color.palette[c*4+2]; /* B */
        }
    }

    // Free resources
    lodepng_state_cleanup(&state);

    return status;
}


/*###########################################################################
 ##                                                                        ##
 ##                        I M A G E   L O A D I N G                       ##
 ##                                                                        ##
 ###########################################################################*/
/*** process_image *********************************************************
 * Loads and process a PNG, generating the tile and palette data that will *
 * be later output in code.                                                *
 ****************************************************************************/
int tilemap_process_image(const char * input_filename) {

    #define MAX_FILENAME_LEN 255

    int status;
    image_data src_image;
    color_data src_colors;
    tile_process_options options;
    char filename[MAX_FILENAME_LEN];

    status = true;

    // Load the image
    status = tilemap_load_image(&src_image, &src_colors, input_filename);

    printf("Image load status: status= %d, colors=%d\n", status, src_colors.color_count);

    if (status) {


// TODO: WIRE UP COMMAND LINE ARGUMENTS TO SELECT OUTPUT MODE
options.image_format = FORMAT_GBM;
// options.image_format = FORMAT_GBR;
// options.image_format = FORMAT_GBDK_C_SOURCE;
// options.image_format = hypothetical_function_get_mode_from_output_filename(plugin_procedure_name);

        tilemap_options_load_defaults(src_colors.color_count, &options);

// TODO: WIRE UP COMMAND LINE ARGUMENTS TO OVERRIDE DEFAULT SETTINGS
// // Export options
// options.gb_mode              = true;
// options.tile_dedupe_enabled  = true;
// options.tile_dedupe_flips    = true;
// options.tile_dedupe_palettes = true;

        tilemap_options_set(&options);


        switch (options.image_format) {

            case FORMAT_GBDK_C_SOURCE:

                snprintf(filename, MAX_FILENAME_LEN, "%s%s",  input_filename, ".c");
                status = tilemap_export_process(&src_image);
                printf("tilemap_export_process: status= %d\n", status);

                if (status)
                    status = tilemap_save(filename, options.image_format);
                printf("tilemap_save: status= %d\n", status);
                break;

            case FORMAT_GBR:

                // Load cached settings in the image parasite metadata
                // tilemap_export_parasite_gbr(image_id);

                snprintf(filename, MAX_FILENAME_LEN, "%s%s",  input_filename, ".gbr");
                status = gbr_save(filename, &src_image, &src_colors, options); // TODO: CGB MODE: send entire options struct down?
                printf("gbr_save: status= %d\n", status);
                break;

            case FORMAT_GBM:

                // Load cached settings in the image parasite metadata
                // tilemap_export_parasite_gbr(image_id);
                // tilemap_export_parasite_gbm(image_id);

                // Set processed Map tile set and map array
                snprintf(filename, MAX_FILENAME_LEN, "%s%s",  input_filename, ".gbm");
                status = gbm_save(filename, &src_image, &src_colors, options);
                printf("gbm_save: status= %d\n", status);
                break;
        }
    }

    // Free the image data
    if (src_image.p_img_data)
        free(src_image.p_img_data);

    tilemap_free_resources();

    return status;
}
