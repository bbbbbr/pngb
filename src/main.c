/*****************************************************************************
**	main.c
**
**	Entry point and command line parsing for the PNGB graphics converter.
**
** 	Copyright (c) 2015 Elias Zacarias
**
** 	Permission is hereby granted, free of charge, to any person obtaining a
** 	copy of this software and associated documentation files (the "Software"),
** 	to deal in the Software without restriction, including without limitation
** 	the rights to use, copy, modify, merge, publish, distribute, sublicense,
** 	and/or sell copies of the Software, and to permit persons to whom the
** 	Software is furnished to do so, subject to the following conditions:
**
** 	The above copyright notice and this permission notice shall be included in
** 	all copies or substantial portions of the Software.

** 	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** 	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** 	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** 	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** 	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** 	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** 	IN THE SOFTWARE.
**
*****************************************************************************/
#include "pngb.h"

#include <stdio.h>
#include <string.h>

#include "tilemap_actions.h"

/*###########################################################################
 ##                                                                        ##
 ##                        M I S C   F U N C T I O N S                     ##
 ##                                                                        ##
 ###########################################################################*/
/*** reset_opts *************************************************************
 * Resets the options.                                                      *
 ****************************************************************************/
void reset_opts(){
	memset((void *)&globalOpts, 0, sizeof(globalOpts));

	globalOpts.type = TARGET_BKG;
	globalOpts.baseindex = 0;
	globalOpts.bank = 0;
	strcpy(globalOpts.name, "gbpic");
}

/*** parse_as_number ********************************************************
 * Attempts to parse a string as a number. Aborts execution on error.       *
 ****************************************************************************/
long parse_as_number (char *p, int base){
	char *numEnd;
	long val = strtol(p, &numEnd, base);
	if (numEnd != &p[strlen(p)]) error ("Couldn't parse %s as a number", p);
	return val;
}

/*** check_for_enough_args **************************************************
 * Checks if we still have more arguments left (index comparison, nothing   *
 * really fancy).                                                           *
 ****************************************************************************/
void check_for_enough_args(char *requested_by, int cur_ndx, int total_args){
	if (cur_ndx >= total_args) error ("Insufficient data for option %s", requested_by);
}

/*** transp_color_from_str **************************************************
 * Tries to get a valid sprite transparency color from a string.            *
 ****************************************************************************/
long transp_color_from_str(char *color_str){
	/* RGB colors will be stored as negative values. 1 will be substracted
	from the RGB val, so RGB #000000 maps to -1 instead of zero, keeping the
	while RGB spectrum negative */
	if (color_str[0] == '#') return -(parse_as_number(&color_str[1], 16) + 1);
	/* Palette index ( >= 0)*/
	return 1 + parse_as_number(color_str, 10);
}

/*** target_to_string *******************************************************
 * Fills a string buffer with the currently selected target type.           *
 ****************************************************************************/
char *target_to_string(char *dest, int mlen){

	memset((void *)dest, 0, mlen);

	if (globalOpts.type == TARGET_BKG){
		strncpy(dest, "BKG", mlen-1);
	}else if (globalOpts.type == TARGET_WINDOW){
		strncpy(dest, "WIN", mlen-1);
	}else if (globalOpts.type == TARGET_SPRITE){
		if (globalOpts.big_sprite){
			strncpy(dest, "SPRITE (8x16)", mlen-1);
		}else {
			strncpy(dest, "SPRITE (8x8)", mlen-1);
		}
	}
	return dest;
}

/*** transp_to_string *******************************************************
 * Fills a string buffer with the currently selected transparent color.     *
 ****************************************************************************/
char *transp_to_string(char *dest, int mlen){
	DWORD rgb;

	memset((void *)dest, 0, mlen);

	if (globalOpts.transparent < 0){
		rgb = -(globalOpts.transparent+1);
		snprintf(dest, mlen-1, "RGB(%ld, %ld, %ld)",(rgb>>16) & 0xff, (rgb>>8) & 0xff, (rgb) & 0xff);
	}else {
		snprintf(dest, mlen-1, "%ld", globalOpts.transparent);
	}
	return dest;
}

/*** print_help *************************************************************
 * Displays the usage info.                                                 *
 ****************************************************************************/
void print_help(){
    printf ("Usage\n");
    printf ("   pngb {input file}\n\n");

/*
	printf ("\nPNGB v%d.%02d ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n", PNGB_VERSION_MAJOR, PNGB_VERSION_MINOR);
	printf ("\nConverts PNG images to GB (GBDK) C Code\n");
	printf ("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
	printf ("Usage\n");
	printf ("   pngb <options> {input file} {output file}\n\n");
	printf ("Options\n");
	printf ("  -K          Generate code and data for the BKG layer.\n");
	printf ("  -W          Generate code and data for the WIN layer.\n");
	printf ("  -S          Generate code and data for 8x8 Sprites.\n");
	printf ("  -B          Generate code and data for 8x16 Sprites.\n");
	printf ("  -p          Generate 15 bit palette data (for GBC).\n");
	printf ("  -m          Generate a Tile Map of the source picture.\n");
	printf ("  -c          Output ready to compile test code with the data.\n");
	printf ("  -g          Convert to grayscale.\n");
	printf ("  -s          Sort the palette from light to dark (helps with GB compatibility).\n");
	printf ("  -e          Tile reduction; Remove identical/redundant tiles from the set.\n");
	printf ("  -v          Verbose output during conversion.\n");
	printf ("  -base NUM   Set the base tile/sprite index.\n");
	printf ("  -pal  NUM   Set the palette number.\n");
	printf ("  -name NAME  Set the name of the sprite/tileset.\n");
	printf ("  -tr COLOR   Set the transparent color for Sprites. COLOR is either \n");
	printf ("              an index from the source palette, or a color in #RRGGBB format.\n\n");
	printf ("Examples\n");
	printf ("   pngb -S spritesheet.png sprite.h\n");
	printf ("   pngb -S -base 1 -pal 2 -name my_sprite spritesheet.png sprite.h\n");
	printf ("   pngb -Kgpcmsev -name my_tileset tileset.png tileset.c\n\n");
*/
}

void ExtractFileName(char* path, char* file_name, int include_bank) {
	char* slash_pos = strrchr(path, '/');
	if(slash_pos == 0)
		slash_pos = strrchr(path, '\\');
	if(slash_pos != 0)
		slash_pos ++;
	else
		slash_pos = path;

	char* dot_pos = include_bank ? strrchr(slash_pos, '.') : strchr(slash_pos, '.');
	if(dot_pos == 0) {
 		strcpy(file_name, slash_pos);
	} else {
		strncpy(file_name, slash_pos, dot_pos - slash_pos);
		file_name[dot_pos - slash_pos] = '\0';
	}
}

int GetBank(char* str) {
	char* bank_info = strstr(str, ".b");
	if(bank_info) {
		return atoi(bank_info + 2);
	}
	return 0;
}

/*###########################################################################
 ##                                                                        ##
 ##                                   M A I N                              ##
 ##                                                                        ##
 ###########################################################################*/
/*** main *******************************************************************
 * There's no place like main().                                            *
 ****************************************************************************/
int main(int argc, char *argv[]){
	int a, n;
	char infile[256], outfile[256], temp[64];
	char *param;
	FILE *output;

	/* TO-DO:
		- color reduction
	 */
	memset((void *)infile, 0, sizeof(infile));
	memset((void *)outfile, 0, sizeof(outfile));

	reset_opts();
	for (a = 1; a < argc; a++){
		param = argv[a];

		if (param[0] == '-'){
			param = &param[1];
			/* First try to parse the option as standalone parameters that
			are more than 1 character long and require arguments. */
			if (!strcmp(param, "base")){
				a++;
				check_for_enough_args (param, a, argc);
				globalOpts.baseindex = parse_as_number(argv[a], 10);
			}else if (!strcmp(param, "pal")){
				a++;
				check_for_enough_args (param, a, argc);
				globalOpts.palnumber = parse_as_number(argv[a], 10);
			}else if (!strcmp(param, "name")){
				a++;
				check_for_enough_args (param, a, argc);
				strcpy(globalOpts.name, argv[a]);
			}else if (!strcmp(param, "tr")){
				a++;
				check_for_enough_args (param, a, argc);
				globalOpts.transparent = transp_color_from_str(argv[a]);
			}else {
				/*	Options that are a single character and don't require extra
					arguments can be combined in a single parameter, so we will
					try to parse the argument in a loop. */
				for (n = 0; n < strlen(param); n++){
					switch (param[n]){
						case 'W':
							globalOpts.type = TARGET_WINDOW;
							break;
						case 'K':
							globalOpts.type = TARGET_BKG;
							break;
						case 'S':
							globalOpts.type = TARGET_SPRITE;
							globalOpts.big_sprite = 0;
							break;
						case 'B':
							globalOpts.type = TARGET_SPRITE;
							globalOpts.big_sprite = 1;
							break;
						case 'g':
							globalOpts.grayscale = 1;
							break;
						case 'p':
							globalOpts.create_palette = 1;
							break;
						case 's':
							globalOpts.sort_palette = 1;
							break;
						case 'm':
							globalOpts.create_map = 1;
							break;
						case 'c':
							globalOpts.test_code = 1;
							break;
						case 'e':
							globalOpts.tile_reduction = 1;
							break;
						case 'v':
							globalOpts.verbose = 1;
							break;
						default:
							error ("Unrecognized option %c", param[n]);
					}
				}
			}
		}else {
			if (infile[0] == 0) {
				/* assume this is the input file */
				strncpy (infile, argv[a], sizeof(infile)-1);
				ExtractFileName(infile, globalOpts.name, 0);
				globalOpts.bank = GetBank(infile);
			} else if (outfile[0] == 0){
				/* if the input file is already defined, this must be the
				output file */
				strncpy (outfile, argv[a], sizeof(infile)-1);
			}else {
				error ("Too many parameters");
			}
		}
	}

    if (!infile[0]) {
        print_help();
        return 0;
    }
    else {
        // Found an input filename, process and export the image
        tilemap_process_image (infile);
    }

/*
	if (!infile[0] || !outfile[0]) {
		print_help();
		return 0;
	}

	output = fopen(outfile, "w");
 	PICDATA *gbdata = process_image (infile);
	// IMPORTANT! CALL THIS BEFORE CODE OUTPUT! This will fix wrong values

	gb_check_warnings (gbdata);

	verbose ("\n<PARAMETERS DEBUG>\nINPUT -\n");
	verbose (" File            : %s\n", infile);
	if (globalOpts.type == TARGET_SPRITE){
		verbose(" Sprite transp.  : %s\n", transp_to_string(temp, sizeof(temp)));
	}

	verbose ("\nOUTPUT -\n");
	verbose (" File            : %s\n", outfile);
	verbose (" Data name       : %s\n", globalOpts.name);
	verbose (" Grayscale       : %s\n", (globalOpts.grayscale ? "YES" : "NO"));
	verbose (" Data type       : %s\n", target_to_string(temp, sizeof(temp)));
	verbose (" Palette         : %s\n", (globalOpts.create_palette ? "YES" : "NO"));
	verbose (" TileMap         : %s\n", (globalOpts.create_map ? "YES" : "NO"));
	verbose (" Test Code       : %s\n", (globalOpts.test_code ? "YES" : "NO"));
	verbose (" Palette Index   : %d\n", globalOpts.palnumber);
	if (globalOpts.test_code || globalOpts.create_map){
		verbose (" Tile Base Index : %d\n", globalOpts.baseindex);
	}

	verbose ("\nADDITIONAL ACTIONS -\n");
	if (!globalOpts.grayscale){
		verbose (" Sort Palette    : %s\n", (globalOpts.sort_palette ? "YES" : "NO"));
	}
	verbose (" Tile reduction  : %s\n", (globalOpts.tile_reduction ? "YES" : "NO"));
	verbose ("\n");

	fprintf	(output, "#pragma bank %d\n", globalOpts.bank);
//	code_disclaimer_c (infile, outfile, output);
//	gbdk_c_code_output (gbdata, output);
    if (gbdata)
	   free_gb_pict(gbdata);

	if (output) fclose (output);
*/
	return 0;
}
