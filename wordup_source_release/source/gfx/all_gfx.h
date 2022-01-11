//Gfx converted using Mollusk's PAGfx Converter

//This file contains all the .h, for easier inclusion in a project

#ifndef ALL_GFX_H
#define ALL_GFX_H

#ifndef PAGfx_struct
    typedef struct{
    void *Map;
    int MapSize;
    void *Tiles;
    int TileSize;
    void *Palette;
    int *Info;
} PAGfx_struct;
#endif

// Sprite files : 
extern const unsigned char wordblocks_Sprite[51200] __attribute__ ((aligned (4))) ;  // Pal : wordblocks_Pal
extern const unsigned char black_mask_Sprite[8192] __attribute__ ((aligned (4))) ;  // Pal : black_mask_Pal

extern const unsigned char stylus_Sprite[1024] __attribute__ ((aligned (4))) ;  // Pal : stylus_Pal
extern const unsigned char tutorial_gfx_Sprite[6144] __attribute__ ((aligned (4))) ;  // Pal : tutorial_gfx_Pal


// Background files : 
extern const int bg_starlow_Info[3]; // BgMode, Width, Height
extern const unsigned short bg_starlow_Map[1024] __attribute__ ((aligned (4))) ;  // Pal : bg_starlow_Pal
extern const unsigned char bg_starlow_Tiles[27904] __attribute__ ((aligned (4))) ;  // Pal : bg_starlow_Pal

extern const int bg_sky_Info[3]; // BgMode, Width, Height
extern const unsigned short bg_sky_Map[1024] __attribute__ ((aligned (4))) ;  // Pal : bg_sky_Pal
extern const unsigned char bg_sky_Tiles[7232] __attribute__ ((aligned (4))) ;  // Pal : bg_sky_Pal

extern const int bg_starhigh_Info[3]; // BgMode, Width, Height
extern const unsigned short bg_starhigh_Map[1024] __attribute__ ((aligned (4))) ;  // Pal : bg_starhigh_Pal
extern const unsigned char bg_starhigh_Tiles[2240] __attribute__ ((aligned (4))) ;  // Pal : bg_starhigh_Pal

extern const int bg_startscreen_Info[3]; // BgMode, Width, Height
extern const unsigned short bg_startscreen_Map[1024] __attribute__ ((aligned (4))) ;  // Pal : bg_startscreen_Pal
extern const unsigned char bg_startscreen_Tiles[3392] __attribute__ ((aligned (4))) ;  // Pal : bg_startscreen_Pal

extern const int bg_startbubble_Info[3]; // BgMode, Width, Height
extern const unsigned short bg_startbubble_Map[1024] __attribute__ ((aligned (4))) ;  // Pal : bg_startbubble_Pal
extern const unsigned char bg_startbubble_Tiles[5312] __attribute__ ((aligned (4))) ;  // Pal : bg_startbubble_Pal

extern const int bg_startlogo_Info[3]; // BgMode, Width, Height
extern const unsigned short bg_startlogo_Map[1024] __attribute__ ((aligned (4))) ;  // Pal : bg_startlogo_Pal
extern const unsigned char bg_startlogo_Tiles[7488] __attribute__ ((aligned (4))) ;  // Pal : bg_startlogo_Pal

// Sprite palette files : 
extern const unsigned short wordblocks_Pal[249] __attribute__ ((aligned (4))) ;
extern const unsigned short black_mask_Pal[2] __attribute__ ((aligned (4))) ;
extern const unsigned short stylus_Pal[6] __attribute__ ((aligned (4))) ;
extern const unsigned short tutorial_gfx_Pal[8] __attribute__ ((aligned (4))) ;

// Background palette files:
extern const unsigned short bg_starlow_Pal[55] __attribute__ ((aligned (4))) ;
extern const unsigned short bg_sky_Pal[9] __attribute__ ((aligned (4))) ;
extern const unsigned short bg_starhigh_Pal[9] __attribute__ ((aligned (4))) ;
extern const unsigned short bg_startscreen_Pal[27] __attribute__ ((aligned (4))) ;
extern const unsigned short bg_startbubble_Pal[20] __attribute__ ((aligned (4))) ;
extern const unsigned short bg_startlogo_Pal[64] __attribute__ ((aligned (4))) ;

#endif

