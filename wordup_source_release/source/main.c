

/**************************************************************************
 *                                                                        *
 *     ALL GAME DESIGN, GRAPHICS, SOUND AND CODE BY FREDRIK JOSEFSSON     *
 *                         fredrik@superpanic.com                         *
 *                           www.superpanic.com                           *
 *                                                                        *
 *     Exclusively composed mod trax by Syphys, syphus.untergrund.net     *
 *                                                                        *
 **************************************************************************/


// Includes
#include <PA9.h>

// fatlib, for save functionality
#include <fat.h>

#include <string.h>
#include <stdio.h>
#include <float.h>

// include gfx
#include "gfx/all_gfx.c"
#include "gfx/all_gfx.h"

// include lexicon
#include "text/dictionary.c"
#include "text/dictionary.h"

// include music
#include "music_ingame.h"
#include "music_break.h"
#include "music_intro.h"

// include sounds
#include "sound_blockhit.h"
#include "sound_bomb.h"
#include "sound_gameover.h"
#include "sound_drawlogo.h"
#include "sound_award.h"

#include "sound_levelclear.h"

// include voices
#include "voice_wordup.h"
#include "voice_levelclear.h"
#include "voice_getready.h"
#include "voice_thankyouforplaying.h"
#include "voice_letsplay.h"







/*************
 *   TOOLS   *
 *************/
#define MAX(a,b)	( (a) > (b) ? (a):(b) )
#define MIN(a,b)	( (a) > (b) ? (b):(a) )
#define ABS(a)		( ((a) < 0) ? -(a):(a) )
#define ARRAY_LEN(x)	( sizeof (x) / sizeof *(x) )




/*****************************
 *   MAIN BLOCK PROPERTIES   *
 *****************************/
// all the info for a charblock
typedef struct {
        
	// ADDED A PROPERTY? DON'T FORGET TO CLEAN IT UP IN resetCHARBLOCK() below.

	float x;		// screen position
	float y;		// screen position
	
	s8 gridx;		// x position in gamegrid
	s8 gridy;		// y position in gamegrid

	s16 yoffset;		// offset is used when blocks fall
	s16 velocity;		// falling speed of block
	
	// floats used for game over animation
	float yMomentum;
	float xMomentum;
	float xFloatOffset;
	float yFloatOffset;
	
	float xSlideSpeed;

	u8 character;		// what character that this block contains
	u8 spriteNum;		// sprite number, used to sort sprites, 0 is top.

	s16 angle;		// rotation - one whole rotation is 0-511, not 360 degrees, use signed, easier to limit 0-511
	u16 zoom;		// zoom - 256 is no zoom, 512 is twice as small, 128 is twice as big, unsigned

	bool explode;		// if hit by a bomb set to true
	u8 explosionCounter;	// explosion counter;

	bool busy;		// if the block is busy, it can't be selected.
	u8 rotSet;		// which rotation set does this blocks sprite belong to
	u8 rotDelay;		// when shoud it start to shrink, just for effect

	bool active;		// true if its active in the game, false if empty
	
	u8 motionCounter;
	
	// ADDED A PROPERTY? DON'T FORGET TO CLEAN IT UP IN resetCHARBLOCK() below.

} CHARBLOCK;

const float blockFriction	= 0.99;
const float blockGravity	= 0.1;
const float blockAcceleration	= 1.01;


/****************************
 *   SET RANDOM ALGORITHM   *
 ****************************/

enum { 
	SIMPLE,
	SUPERPANIC,
	JAYENKAI
};

u8 randomAlgorithm = SUPERPANIC;


bool voiceEffects = true;



/*************
 *   SCORE   *
 *************/
//                             A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, *
const u8 SCORE_ARRAY[ 27 ] = { 1, 2, 2, 2, 1, 2, 2, 2, 1, 4, 4, 2, 2, 1, 1, 2, 4, 1, 1, 1, 2, 4, 2, 4, 1, 4, 0 };
const u8 BASE_VALUE = 100;
u32 score = 0;
u32 displayScore = 0;



/**************************
 *   PLAYERS STATISTICS   *
 **************************/
u32 highestScore = 0;
u8 longestWord = 0;
u8 longestLevelWord = 0;
char longestWordStr[8];
char longestLevelWordStr[8];
u32 numberOfWords = 0;
u32 numberOfLevelWords = 0;
#define MAX_NUMBER_OF_WORDS (1024)
char allWordsArray[ MAX_NUMBER_OF_WORDS ][8];
u32 charsCollected = 0;




bool award_a = false;	// 200 words!
bool award_b = false;	// 100 words!
bool award_c = false;	// 7 char word!







u32 longestWordBonus = 0;
u32 timeBonus = 0;
u32 charsBonus = 0;

u8 const MAX_NAME_LENGTH = 4;
char currentNameString[5];
u8 currentNameStringLen;



/***************
 *   HISCORE   *
 ***************/
u16	hiscoreArray[10] 	= {   1000,    900,    800,    700,    600,    500,    400,    300,    200,    100 };
char	hiscoreNameArray[10][5] = { "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE" };

u16	hiscoreWordCount[10] 	= {      1,      1,      1,      1,      1,      1,      1,      1,      1,      1 };
bool	hiscoreAwards_a[10]	= {  false,  false,  false,  false,  false,  false,  false,  false,  false,  false };
bool	hiscoreAwards_b[10]	= {  false,  false,  false,  false,  false,  false,  false,  false,  false,  false };
bool	hiscoreAwards_c[10]	= {  false,  false,  false,  false,  false,  false,  false,  false,  false,  false };
char	hiscoreLongWord[10][8]	= { "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE" };

u8	hiscoreShowCounter = 0;
float	hiscoreMotionArray[22] = { 1.0, 0.800, 0.700, 0.600, 0.500, 0.450, 0.400, 0.350, 0.300, 0.275, 0.250, 0.225, 0.200, 0.175, 0.150, 0.125, 0.100, 0.075, 0.050, 0.025, 0.01, 0 };

u16 showHiscoreDelayCounter = 0;




/**************
 *   CURSOR   *
 **************/
// properties for the selector sprite
typedef struct {
        u8	xpos;
        u8	ypos;
        u8	spriteNum;
        u8	frame;
        u8	target;
        bool	idle;
        u8	stepCounter;
} CURSOR;
const u8 CURSOR_MOVE_LUT[ 14 ] = { 1, 1, 2, 2, 3, 3, 4, 4, 3, 3, 2, 2, 1, 1 };






/********************
 *   WARNING TABS   *
 ********************/
typedef struct {
	u8	offset;
	u8	textColumn;
	u16	xpos;
	u16	ypos;
	bool	active;
	u16	counter;
	u16	movementCounter;
	u8	spriteNum;
} WARNING_TAB;

const u8 WARNING_TAB_MOVEMENT[ 46 ] = { 1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 10, 11, 12, 13 };



/**********************
 *   SPRITE WRAPPER   *
 **********************/
typedef struct {
	u8	spriteNum;
        
	s16	x;
        s16	y;

	double	floatXOffset;
        double	floatYOffset;
        
        double xVelocity;
	double yVelocity;

        bool	idle;
        
        u16	counter;
        u16	timer;
} SPRITE_WRAPPER;

double	smoothMotionArray[21] 		= { 1.0, 0.800, 0.700, 0.600, 0.500, 0.450, 0.400, 0.350, 0.300, 0.275, 0.250, 0.225, 0.200, 0.175, 0.150, 0.125, 0.100, 0.075, 0.050, 0.025, 0 };

double	smoothMotionArrayWithBounce[27]	= { 1.0, 0.800, 0.700, 0.600, 0.500, 0.450, 0.400, 0.350, 0.300, 0.275, 0.250, 0.225, 0.200, 0.175, 0.150, 0.125, 0.100, 0.075, 0.050, 0.025, 0, 0.050, 0.100, 0.150, 0.100, 0.050, 0 };

// 10, 8, 7, 6, 5, 4.5, 4, 3.5, 3, 2.75, 2.5, 2.25, 2, 1.75, 1.5, 1.25, 1.0, 0.75, 0.5, 0.25, 0 




SPRITE_WRAPPER	flyingSprites[6];
void setupFlyingSprites();
void runFlyingSprites();
void cleanFlyingSprites();
void setFlyingSpriteVelocity( SPRITE_WRAPPER *fs, bool xdir, bool ydir );




/************************************
 *   MOVE TUTORIAL STYLUS TARGET    *
 ************************************/
bool targetReached = false;


/**********************************
 *   IMPORTANT GAME PROPERTIES    *
 **********************************/
const u8 HALF_SCREEN_WIDTH = 128;
const u8 HALF_SCREEN_HEIGHT = 96;
const u8 GRIDH = 6;
const u8 GRIDW = 8;

bool levelClearFlag = false;



enum { HARD, MEDIUM, EASY };

u8 difficulty = MEDIUM;


u8 gameClockSpeed = 60;


u32 gameSpeed;
u32 gameSpeedTimer;


bool gameOver = false;
u8 level;
u8 levelGoal = 0;
u8 levelGoals[15] = { 3, 5, 7, 10, 15, 20, 25, 30, 35, 40, 45, 50, 75, 100, 200 };
u8 maxLevels = 15;

bool addBomb = false;

u8 playerAddedBlockCounter = 0;




/*************************************
 *   SPECIAL CHARACTER PROPERTIES    *
 *************************************/
const u8 ASCII_OFFSET = 'A'; 	// here starts 'A-Z' in ascii, used to convert to string
const char STAR = 26;




/*********************
 *   MUSIC STATES    *
 *********************/
bool musicIsPlaying = false;




/********************************
 *   PROGRAM AND GAME STATES    *
 ********************************/
enum { 
        
SAVEQUESTION_A,
SAVEQUESTION_B,

        MENU,
        TITLE_APPEAR,
        RUN_MENU,

	CREDITS,
	RUN_CREDITS,
	FADE_OUT_CREDITS,

        OPTIONS_INIT,
        RUN_OPTIONS,
        FADE_OUT_OPTIONS,

	FADE_OUT_MENU,
	
	INIT_TUTORIAL,
	FADE_IN_TUTORIAL,
	TUTORIAL_TITLE,			
	RUN_TUTORIAL_A,
	RUN_TUTORIAL_B,		
	RUN_TUTORIAL_C,
	WAIT_FOR_NEXT_TUTORIAL,
	FADE_OUT_TUTORIAL,
	CLEAR_TUTORIAL,
	
        INIT_GAME,
	FADE_IN_GAME, 
	RUN_GAME,

PAUSE_FADE,
PAUSE,
QUIT_GAME,

	LEVEL_CLEAR,
	LEVEL_CLEAR_FADE_OUT,
	LEVEL_CLEAR_ANIMATION,
	LEVEL_CLEAR_FADE_IN,
	LEVEL_SCORE,
	NEXT_LEVEL_GOAL,

	GAME_CLEAR,
	GAME_OVER,
	GAME_OVER_ANIMATION,
	GAME_OVER_WAIT,
	
	AWARD_A,
	AWARD_B,
	AWARD_C,

	INIT_PRINT_NAME,
	PRINT_NAME,
	SET_NAME,
	RESET_GAME,
	SAVE_SCORE,
	SHOW_HISCORES,

	WAIT_FOR_NEXT_STATE_SCROLL,
	WAIT_FOR_NEXT_STATE,

	ERROR

};

u8	gameState;

u8	nextState = WAIT_FOR_NEXT_STATE;
u32	stateTimer = 0;



enum{ CLASSIC, ARCADE };
u8 gameStyle = CLASSIC;


bool palIsNotLoaded = true;


/****************************************
 *   GAME WORD AND STRING PROPERTIES    *
 ****************************************/
const u8 MAX_WORD_LENGTH = 7;
char currentWord[8];
bool realWord = false;		// true if the selected word is found in the lexicon
u8 wordLength = 0;



/***********************
 *    DEBUG OUTPUT     *
 ***********************/
char debugStr[31] = "null";		// used for debug
s32 debugVal = -1;			// used for debud output



/********************************************
 *    GLOBAL COUNTER TO USE FOR ANYTHING    *
 ********************************************/
u32 anyCounter = 0;



/***********************************************************
 *   STUFF FOR USING FATLIB (SAVING AND LOADING HISCORE)   *
 ***********************************************************/
char	fileName[13+1] = "wordupsav.sav";
bool	saveEnabled = false;

// prototypes for saving
bool	saveExist( void );
bool	createSave( void );
bool	tryEnableSave( void );
bool	saveHiscore( void );
bool	loadHiscore( void );




/*****************************
 *    FUNCTION PROTOTYPES    *
 *****************************/
bool		hasAward( void );

void		lockSpriteCreate( u8 *lockSpriteNum );
void		lockSpriteHide( u8 *lockSpriteNum );
void		lockSpriteShow( u8 *lockSpriteNum );
void		lockSpriteDelete( u8 *lockSpriteNum );

void		balanceSound( void );
 
bool		fadeOutMod( u8 vol );
bool 		fadeUpMod( u8 vol );
 
s8		GetFreeLimitedSoundChannel();

void 		deleteWarningTabs( WARNING_TAB warningTabs[8] );

void 		bounceGameLogo( s8 *bounceCounter, s8 const bounceArray[14] );
void 		printOptions( void );
 
bool 		moveStylus( s16 tx, s16 ty, SPRITE_WRAPPER *p_stylus );
// void 		textFader( char s[30] );

s8 		setHiscore( void );
void 		initScrollHiscore( CHARBLOCK blockArray[GRIDH][GRIDW] );
void 		scrollHiscore( CHARBLOCK blockArray[GRIDH][GRIDW], u8 awardBottomSprites[3] );
void 		clearScrollHiscore( CHARBLOCK blockArray[GRIDH][GRIDW] );


u32 		getLongestWordBonus( void );
u32		getTimeBonus( void );
u32 		getCharsBonus( void );

void		displayTopInfo( u8 *nSelected, char currentWord[8] );

void 		scrollSky( float *skyScroll );
void		addScore( char s[8], u8 n );
u32		getScore( char s[8], u8 n );



void		addAndDisplayWordScore( char s[8], u8 n, u8 arr[8], bool *showScoreFlag, s16 xpos, s16 ypos, s16 *scoreYPos );
void		scrollScoreDisplay( u8 scoreSprites[8], const double *SCORE_DISPLAY_OFFSET, u8 *scoreDisplayMotionCounter, s16 *scoreYpos );


void		clearScoreDisplay( u8 arr[8], bool *showScoreFlag, u8 *scoreDisplayMotionCounter );
bool		showScoreDisplay( u16 *scoreDisplayCounter, float *scoreDisplayFade );
u32 		getMostUsedWord( u32 numberOfWords, char allWordsArray[ MAX_NUMBER_OF_WORDS ][8] );
u32 		getMostUsedWordCount( u32 numberOfWords, char allWordsArray[ MAX_NUMBER_OF_WORDS ][8] );
u32		getNewBlockTime( void );
void		addNewBlock( CHARBLOCK topBlockArray[GRIDH][GRIDW], CHARBLOCK blockArray[GRIDH][GRIDW] );
char		getNextChar( void );

void 		fillBag();
// char		getRandomLexiconChar( void );

char		simpleRandomChar( void );
char		superpanicRandomChar( void );
char		jayenkaiRandomChar( void );


void		hideCursor( CURSOR *cursor );
void		runCursor( CURSOR *cursor );
CHARBLOCK	*getPointerToTouchedCharBlock( CHARBLOCK ba[GRIDH][GRIDW] );
CHARBLOCK	*getPointerToTouchedCharBlock_noBoundCheck( CHARBLOCK ba[GRIDH][GRIDW] );
void		buildWord( CHARBLOCK *charBlock );
void		buildName( CHARBLOCK *charBlock );
u16		getAlpha( u16 _alpha );
void		addSelection( CHARBLOCK *p_arr[MAX_WORD_LENGTH], CHARBLOCK *p_add, u8 nSel );
void		clearSelection( CHARBLOCK *p_selectedBlocksArray[MAX_WORD_LENGTH], u8 nSelected );
void		clearSelectionRange( CHARBLOCK *p_selectedBlocksArray[MAX_WORD_LENGTH], u8 *nSelected, u8 s );
bool 		starSelected( char s[8] );
bool		searchLexicon( char s[8] );
void		resetCHARBLOCK( CHARBLOCK *p_block );

void 		stop();

// Function: main()
int main(int argc, char ** argv) {




	/****************************
	 *    INIT MAIN DS LIBS     *
	 ****************************/
	// initializing libs
	PA_Init();			// Initializes PA_Lib
	PA_InitVBL();			// Initializes a standard VBL	
	PA_InitSound();			// Init the sound system




	/*******************************
	 *    INIT RANDOM FUNCTION     *
	 *******************************/
	// initializing functions
	PA_InitRand();			// init the random seed	





	/***********************************
	 *   INIT BOTTOM SCREEN SPRITES    *
	 ***********************************/

	CHARBLOCK blockArray[GRIDH][GRIDW];			// array of blocks	
	u8 xi, yi;			// screen coordinates		
	u8 i=0;				// sprite counter
		
	// to be able to draw in front of the block sprites, the z has to be set to a value > 0.
	u8 zOffsetBottomScreen = 32;

	// setup the main grid with blocks on bottom screen
	for( yi=0; yi < GRIDH; yi++ ) {
		for( xi=0; xi < GRIDW; xi++ ) {
			
			// increase sprite number
			i++;
			blockArray[yi][xi].spriteNum = i + zOffsetBottomScreen;
			
			blockArray[yi][xi].gridx = xi;
			blockArray[yi][xi].gridy = yi;
			
		        // set screen position
			blockArray[yi][xi].x = xi << 5; // multiply with 32
			blockArray[yi][xi].y = yi << 5; // multiply with 32
						
		}
	}

	u8 lockSpriteNum = 1;

	/*****************************
	*      SELECTION CURSOR      *
	******************************/

	CURSOR cursor;

	cursor.xpos = -32;
	cursor.ypos = -32;

	cursor.spriteNum = zOffsetBottomScreen-1;
	
	cursor.frame = 27;
	cursor.target = 0;
	cursor.stepCounter = 0;
	cursor.idle = true;

	PA_CreateSprite(			// create the cursor (selection rectangle) sprite
		0,				// screen
		cursor.spriteNum,		// sprite number
		(void*)wordblocks_Sprite,	// sprite name
		OBJ_SIZE_32X32,			// sprite size
		1,				// 256 color mode
		0,				// sprite palette number
		cursor.xpos,			// x pos (* 32)
		cursor.ypos			// y pos
	);
		
	PA_SetSpriteAnim(
		0,			// screen
		cursor.spriteNum,	// sprite number
		cursor.frame		// set frame to i
	);


	PA_StartSpriteAnim(		// Start the animation. Once started, it works on its own!
		0,			// screen
		cursor.spriteNum,	// sprite number
		cursor.frame,		// first frame
		cursor.frame+2,		// last frame
		12			// Speed, n frames per second
	);


	// let the text background print on top of sprites
	PA_SetSpritePrio( 0, cursor.spriteNum, 1 );


	/****************************
	*     TOP SCREEN BLOCKS     *
	*****************************/
	
	CHARBLOCK topBlockArray[GRIDH][GRIDW];	// falling block on top screen
	u8 zOffsetTopScreen = 32; // to be able to draw in front of the block sprites, the z has to be set to a value > 0

	i = zOffsetTopScreen;
	// setup the currently empty falling block array
	for( yi=0; yi < GRIDH; yi++ ) {
		for( xi=0; xi < GRIDW; xi++ ) {
			// increase spritenum counter (we DON'T want to use spriteNum 0)
			i++;			
			topBlockArray[yi][xi].spriteNum = i;
			
			// grid positions
			topBlockArray[yi][xi].gridx = xi;
			topBlockArray[yi][xi].gridy = yi;
			
			topBlockArray[yi][xi].velocity = 0;
			topBlockArray[yi][xi].yoffset = 0;

			// used for game over animation
			topBlockArray[yi][xi].xFloatOffset = 0;
			topBlockArray[yi][xi].yFloatOffset = 0;
			topBlockArray[yi][xi].xMomentum = (float)(PA_RandMinMax(0,4))-2;
			topBlockArray[yi][xi].yMomentum = (float)(PA_RandMinMax(0,4))-2;
						
		        // set screen position
			topBlockArray[yi][xi].x = xi << 5; // multiply with 32
			topBlockArray[yi][xi].y = (yi << 5); // multiply with 32


			float fx = (float)topBlockArray[yi][xi].x + 16.0;
			
			if( fx >= 144 ) {
				// right side
				topBlockArray[yi][xi].xSlideSpeed = (255 - fx) / 10;
			} else {
				// left side
				topBlockArray[yi][xi].xSlideSpeed = -fx / 10;
			}


			resetCHARBLOCK( &(topBlockArray[yi][xi]) );
		}
	}
	
	
	
	/*******************************************
	 *     WORD SCORE DISPLAY ON TOP SCREEN    *
	 *******************************************/
	u8 scoreSpriteArray[8] = { zOffsetBottomScreen-11, zOffsetBottomScreen-12, zOffsetBottomScreen-13, zOffsetBottomScreen-14, zOffsetBottomScreen-15, zOffsetBottomScreen-16, zOffsetBottomScreen-17, zOffsetBottomScreen-18 };	

	u16 scoreDisplayCounter = 0;
	float scoreDisplayFade = 0;
	bool showScoreFlag = false;
	
	s16 scoreYPos = 0;
	
	const double SCORE_DISPLAY_OFFSET = 16.;
	u8 scoreDisplayMotionCounter = 0;

	u16 showMissionTimer = 0;




	/*****************************************
	 *     AWARD SPRITES ON BOTTOM SCREEN    *
	 *****************************************/
	u8 awardBottomSprites[3] = { zOffsetBottomScreen-19, zOffsetBottomScreen-20, zOffsetBottomScreen-21 };

	


	/**************************************
	 *     AWARD SPRITES ON TOP SCREEN    *
	 **************************************/
	u8 awardTopSprites[3] = { zOffsetTopScreen-16, zOffsetTopScreen-17, zOffsetTopScreen-18 };




	/************************************************************************
	 *     SPRITES USED TO REVEAL THE SPEAK BUBBLE LOGO ON START SCREEN     *
	 ************************************************************************/
	u8 introLogoMaskSprites[4] = { 0, 1, 2, 3 };




	/****************************************
	 *     SPRITES USED IN THE TUTORIAL     *
	 ****************************************/
	SPRITE_WRAPPER tutorialSprite_G;
	SPRITE_WRAPPER tutorialSprite_A;
	SPRITE_WRAPPER tutorialSprite_M;
	SPRITE_WRAPPER tutorialSprite_E;

	
	tutorialSprite_G.spriteNum = zOffsetTopScreen-10;
	tutorialSprite_A.spriteNum = zOffsetTopScreen-11;
	tutorialSprite_M.spriteNum = zOffsetTopScreen-12;
	tutorialSprite_E.spriteNum = zOffsetTopScreen-13;


	SPRITE_WRAPPER stylusSprite;
	stylusSprite.spriteNum = zOffsetTopScreen-15;
		
	SPRITE_WRAPPER *tutorialSpriteArray[4] = { &tutorialSprite_M, &tutorialSprite_E, &tutorialSprite_G, &tutorialSprite_A };

	
		
	/****************************************
	*      TUTORIAL CURSOR (TOP SCREEN)     *
	*****************************************/

	SPRITE_WRAPPER tutorialCursor;
	tutorialCursor.spriteNum = zOffsetTopScreen-14;

	SPRITE_WRAPPER tutorialButtonL;
	tutorialButtonL.spriteNum = zOffsetTopScreen-19;

	SPRITE_WRAPPER tutorialButtonR;
	tutorialButtonR.spriteNum = zOffsetTopScreen-20;

	SPRITE_WRAPPER tutorialFallingBlock1;
	tutorialFallingBlock1.spriteNum = zOffsetTopScreen-21;

	SPRITE_WRAPPER tutorialFallingBlock2;
	tutorialFallingBlock2.spriteNum = zOffsetTopScreen-22;
	

	/****************************
	 *     SCROLLING EFFECT     *
	 ****************************/	
	float skyScroll = 0; // used to scroll top screen backgrounds



	/*********************************
	 *     PANIC BOUNCING EFFECT     *
	 *********************************/	
	s8		bounceCounter = 0;
	s8 const	bounceArray[ 14 ] = { 1, 2, 3, 3, 2, 1, 0, -1, -2, -3, -3, -2, -1, 0  };
	bool		columnsBouncing[ 8 ] 	= {  false, false, false, false, false, false, false, false  };

	
	
	
	/********************
	 *   WARNING TABS   *
	 ********************/
	WARNING_TAB	warningTabs[8];
			
	for( i=0; i<8; i++ ) {
		warningTabs[i].offset = 13;
		warningTabs[i].spriteNum = ( zOffsetBottomScreen - 2 )-i;
		// we want the serie: 1, 5, 9, 13, 17, 21, 25, 29:
		warningTabs[i].textColumn = (i*4)+1;
		warningTabs[i].xpos = (i*4) * 8;
		warningTabs[i].ypos = 192-32;
		warningTabs[i].active = false;
		warningTabs[i].counter = 600;
		warningTabs[i].movementCounter = ARRAY_LEN( WARNING_TAB_MOVEMENT )-1;
	};
	

	
	/***********************************
	 *   CREATE WARNING TAB SPRITES    *
	 ***********************************/



	/***************************************************
	 *     ALPHA BLENDING EFFECTS FOR BLOCK SPRITES    *
	 ***************************************************/
	PA_EnableSpecialFx( 0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD );	// Enable the alpha-blending for BOTTOM SCREEN
	PA_EnableSpecialFx( 1, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD );	// Enable the alpha-blending for TOP SCREEN
	u16 alpha = 15;	// special effect for fading acive block



	/*********************************
	 *     SCREEN BRIGHTNESS FADE    *
	 *********************************/
	s8 screenFade = 0;



	/*****************************************************
	 *     EXPLOSION EFFECTS FOR BOMBED BLOCK SPRITES    *
	 *****************************************************/
	u8 explosionAnimationArray[ 30 ] = { 40, 40, 40, 41, 41, 41, 41, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 42, 42, 42, 42, 42, 41, 41, 41, 41, 40, 40, 40 };
	const u8 EXPLOSION_FRAMES = ARRAY_LEN( explosionAnimationArray );



	/*************************************
	 *     CURRENTLY SELECTED BLOCKS     *
	 *************************************/

	CHARBLOCK *p_selectedBlocksArray[MAX_WORD_LENGTH];	// create an array of pointers to blocks from blockArray that is selected	
	u8 nSelected = 0;					// number of blocks currently selected
	CHARBLOCK *p_currentBlock = NULL;			// pointer to currently selected block
	CHARBLOCK *p_lastBlock = NULL;				// pointer to last selected block
	
	bool stylusIsOnCursor = false;				// used when writing players name after game over, see gameState PRINT_NAME in game loop


	/********************
	*     GAME SPEED    *
	*********************/
	u32 blockTimer = getNewBlockTime(); // get the time for next top block to appear
	
	
	
	/***********************
	 *     STYLUS INFO     *
	 ***********************/
	bool firstTouch = true; // when any (first) block is touched after the stylus is realeased	
	bool pressed = FALSE;



	/**************************************
	 *     MAIN INFINITE PROGRAM LOOP     *
	 *     The main loop is divided       *
	 *     into different game states.    *
	 *     Only one state at a time       *
	 *     is running.                    *
	 **************************************/
	gameState = SAVEQUESTION_A;

	while (1) {
	        
		switch( gameState ) {
			

			case SAVEQUESTION_A: {
				/*******************************************
				 *    INIT TEXT FUNCTIONS ON BOTTOM SCREEN    *
				 *******************************************/			
				PA_InitText(0,0);	// init the default text functions (screen, background)
				PA_SetTextCol(		// set the text columns white
					0,		// screen
					31,		// Red
					31,		// Green
					31		// Blue
				);

				gameState = SAVEQUESTION_B;
				break;
			}


			case SAVEQUESTION_B: {
				// clear all text
//				PA_ClearTextBg( 0 );
				
				PA_OutputText( 0, 0, 10,  "      ENABLE HISCORE SAVE?     " );
				PA_OutputText( 0, 0, 12, "         NO        YES         " );

				
				if( Stylus.Held ) {
					if( Stylus.Y > 8*10 && Stylus.Y < 8*13 ) {
						// check for menu items
						if( Stylus.X > 8*8 && Stylus.X < 8*12 ) { // NO
							PA_ResetBgSys();
							gameState = MENU;
						} 
						if( Stylus.X > 8*18 && Stylus.X < 8*23 ) { // YES
							if( tryEnableSave() ) {
								PA_ResetBgSys();
								gameState = MENU;
							} else {
								PA_OutputText( 0, 0, 16,  "   CAN'T SAVE ON THIS DEVICE!" );
							}						
						} 
						
					}
				}

				break;
			}			
			
			case MENU: {
			        
			        gameOver = false;
			        
				/*******************************************
				 *    INIT TEXT FUNCTIONS ON BOTTOM SCREEN    *
				 *******************************************/			
				PA_InitText(0,0);	// init the default text functions (screen, background)
				PA_SetTextCol(		// set the text columns white
					0,		// screen
					31,		// Red
					31,		// Green
					31		// Blue
				);
				
				
				
				/*********************************************
				 *    PREPARE LOGO GRAPHICS ON TOP SCREEN    *
				 *********************************************/
				// load palette to top screen, set to palette 1
				PA_LoadSpritePal(1, 1, (void*)black_mask_Pal );				
				
				PA_CreateSprite( 1, introLogoMaskSprites[0], (void*)black_mask_Sprite, OBJ_SIZE_64X32, 1, 1, 0, 0 );
				PA_CreateSprite( 1, introLogoMaskSprites[1], (void*)black_mask_Sprite, OBJ_SIZE_64X32, 1, 1, 0, 0 );
				PA_CreateSprite( 1, introLogoMaskSprites[2], (void*)black_mask_Sprite, OBJ_SIZE_64X32, 1, 1, 0, 0 );
				PA_CreateSprite( 1, introLogoMaskSprites[3], (void*)black_mask_Sprite, OBJ_SIZE_64X32, 1, 1, 0, 0 );
				
				// enable double size
				PA_SetSpriteDblsize(1, introLogoMaskSprites[0], 1);
				PA_SetSpriteDblsize(1, introLogoMaskSprites[1], 1);
				PA_SetSpriteDblsize(1, introLogoMaskSprites[2], 1);
				PA_SetSpriteDblsize(1, introLogoMaskSprites[3], 1);
								
				// Activate rotations for selected sprite
				PA_SetSpriteRotEnable( 1, introLogoMaskSprites[0], 0 );
				PA_SetSpriteRotEnable( 1, introLogoMaskSprites[1], 0 );
				PA_SetSpriteRotEnable( 1, introLogoMaskSprites[2], 0 );
				PA_SetSpriteRotEnable( 1, introLogoMaskSprites[3], 0 );
								
				// size the sprites
				PA_SetRotsetNoAngle( 1, 0, 128, 128 );

				// position the sprites
				PA_SetSpriteXY(1, introLogoMaskSprites[0], 0, 32);
				PA_SetSpriteXY(1, introLogoMaskSprites[1], 0, 96);
				PA_SetSpriteXY(1, introLogoMaskSprites[2], 128, 96);
				PA_SetSpriteXY(1, introLogoMaskSprites[3], 128, 32);
				
				// wait for screen redraw
				PA_WaitForVBL();
				
				/*******************************************************
				 *    LOAD BACKGROUND GFX, PALETTES AND SET POSITION   *
				 *******************************************************/
				PA_LoadTiledBg( 1, 3, bg_startbubble );

				
				// play draw sound:
				s8 tmpsnd;
				tmpsnd = GetFreeLimitedSoundChannel();
				if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_drawlogo );

				
				gameState = TITLE_APPEAR;
				break;
				
			}
			

			
			case TITLE_APPEAR: {
				/***********************************
				 *      HARD CODED! ANIMATION      *
				 ***********************************/
				s8 tmpMovedA = PA_GetSpriteX( 1, introLogoMaskSprites[0] );
				s8 tmpMovedB = PA_GetSpriteY( 1, introLogoMaskSprites[3] );
				u8 drawspeed = 6;
				if( tmpMovedA > -82 ) {
					PA_SetSpriteX(1, introLogoMaskSprites[0], PA_GetSpriteX( 1, introLogoMaskSprites[0] )-drawspeed );
					tmpMovedA = tmpMovedA - drawspeed;
				} else if( PA_GetSpriteY( 1, introLogoMaskSprites[1] ) < 152 ) {
					PA_SetSpriteY(1, introLogoMaskSprites[1], PA_GetSpriteY( 1, introLogoMaskSprites[1] )+drawspeed );
				
				} else if( PA_GetSpriteX( 1, introLogoMaskSprites[2] ) < 204 ) {
					PA_SetSpriteX(1, introLogoMaskSprites[2], PA_GetSpriteX( 1, introLogoMaskSprites[2] )+drawspeed );
				
				} else if( tmpMovedB > -32 ) {
					PA_SetSpriteY(1, introLogoMaskSprites[3], PA_GetSpriteY( 1, introLogoMaskSprites[3] )-drawspeed );
					tmpMovedB = tmpMovedB - drawspeed;
					anyCounter = 30;
					
				} else if( anyCounter > 0 ) {
					anyCounter--;
					
				} else {
					
					// disable double size
					PA_SetSpriteDblsize(1, introLogoMaskSprites[0], 0);
					PA_SetSpriteDblsize(1, introLogoMaskSprites[1], 0);
					PA_SetSpriteDblsize(1, introLogoMaskSprites[2], 0);
					PA_SetSpriteDblsize(1, introLogoMaskSprites[3], 0);
					
					// size the sprites
					PA_SetRotsetNoAngle( 1, 0, 256, 256 );
					
					// disable rotset for the sprites
					PA_SetSpriteRotDisable(1, introLogoMaskSprites[0]);
					PA_SetSpriteRotDisable(1, introLogoMaskSprites[1]);
					PA_SetSpriteRotDisable(1, introLogoMaskSprites[2]);
					PA_SetSpriteRotDisable(1, introLogoMaskSprites[3]);

					// delete sprites
					PA_DeleteSprite(1, introLogoMaskSprites[0]);
					PA_DeleteSprite(1, introLogoMaskSprites[1]);
					PA_DeleteSprite(1, introLogoMaskSprites[2]);
					PA_DeleteSprite(1, introLogoMaskSprites[3]);
					
					PA_LoadTiledBg( 1, 2, bg_startlogo );
					PA_LoadTiledBg( 1, 1, bg_startscreen );
					
					// load palette to bottom screen, set to palette 1
					PA_LoadSpritePal(
						0,			// screen
						0,			// palette number
						(void*)wordblocks_Pal	// palette name
					);					
										
					setupFlyingSprites();
			
					PA_PlayMod( music_intro );
					PA_SetModVolume( 127 );
					balanceSound();
					
					
					lockSpriteCreate( &lockSpriteNum );
					
					lockSpriteShow( &lockSpriteNum );

					gameState = RUN_MENU;
					// show text logo and to to next state
				}
				
				break;
			}
			

						
			case RUN_MENU: {
				// clear all text
				PA_ClearTextBg( 0 );
				
				PA_OutputText( 0, 12, 8,  "WORD UP!" );
				if( hasAward() ) PA_OutputText( 0, 12, 10, "ARCADE" );
				else PA_OutputText( 0, 12, 10, "%c8ARCADE" );
				PA_OutputText( 0, 12, 13, "OPTIONS" );
				PA_OutputText( 0, 12, 15, "CREDITS" );

				if( saveEnabled ) {
					PA_OutputText( 0, 8, 19,  "SAVING ENABLED" );
				} else {
					if( bounceCounter % 3 ) PA_OutputText( 0, 8, 19,  "%c8SAVING DISABLED" );					
					//PA_OutputText( 0, 8, 19,  "%c8SAVING DISABLED" );					
				}

				PA_OutputText( 0, 10, 21, "%c8Version 0.4" );

				if( bounceCounter < ARRAY_LEN( bounceArray )-1 ) {
					bounceCounter++;
				} else {
					bounceCounter = 0;
				}
				PA_BGScrollY( 1, 2, bounceArray[bounceCounter] );
	
				// update stylus
				// PA_UpdateStylus();
				
				runFlyingSprites();
				
				if( Stylus.Held ) {
					if( Stylus.X > 8*11 && Stylus.X < 8*22 ) {
						// check for menu items
						if( Stylus.Y > 8*7 && Stylus.Y < 8*9 ) {
							bounceCounter = 0;
							anyCounter = 30;
							gameStyle = CLASSIC;
							gameState = FADE_OUT_MENU;
						} 
						if( Stylus.Y > 8*9 && Stylus.Y < 8*11 ) {
							if( hasAward() ) {
								bounceCounter = 0;
								anyCounter = 30;
								gameStyle = ARCADE;
								gameState = FADE_OUT_MENU;	
							}						
						} 
						if( Stylus.Y > 8*12 && Stylus.Y < 8*14 ) {
							// bounceCounter = 0;
							anyCounter = 30;
							screenFade = 0;
							gameState = OPTIONS_INIT;
						}

						if( Stylus.Y > 8*14 && Stylus.Y < 8*16 ) {
							// bounceCounter = 0;
							anyCounter = 30;
							screenFade = 0;
							gameState = CREDITS;
						}
						
						
					}
				}


				
				break;
			}
			
			
			case CREDITS: {

				if( bounceCounter < ARRAY_LEN( bounceArray )-1 ) {
					bounceCounter++;
				} else {
					bounceCounter = 0;
				}

				PA_BGScrollY( 1, 2, bounceArray[bounceCounter] );

				if( anyCounter > 0 ) {
					// blink the selected menu option for half a sec
					PA_ClearTextBg( 0 );
					PA_OutputText( 0, 12, 8,  "WORD UP!" );
					if( hasAward() ) PA_OutputText( 0, 12, 10, "ARCADE" );
					else PA_OutputText( 0, 12, 10, "%c8ARCADE" );
					PA_OutputText( 0, 12, 13, "OPTIONS" );

					if( anyCounter % 5 ) PA_OutputText( 0, 12, 15, "CREDITS" );

					if( saveEnabled ) {
						PA_OutputText( 0, 8, 19,  "SAVING ENABLED" );
					} else {
						if( bounceCounter % 3 ) PA_OutputText( 0, 8, 19,  "%c8SAVING DISABLED" );					
					}

					anyCounter--;

				} else if( screenFade > -32 ) {
					screenFade--;
					PA_SetBrightness( 0, screenFade );
					
					lockSpriteHide( &lockSpriteNum );
					// PA_SetBrightness( 1, screenFade );

				} else {
					// clear all text
					PA_ClearTextBg( 0 ); 

					PA_OutputText( 0, 0,  8, "  GAME DESIGN AND PROGRAMMING   " );
					PA_OutputText( 0, 0,  9, "         BY SUPERPANIC          " );

					PA_OutputText( 0, 0, 11, "      ALL MUSIC BY SYPHUS       " );
				        PA_OutputText( 0, 0, 12, "     SYPHUS.UNTERGRUND.NET      " );
					
					PA_OutputText( 0, 0, 14, "   SPECIAL THANKS TO MOLLUSK    " );
					PA_OutputText( 0, 0, 15, "   AND EVERYONE AT PALIB.COM    " );

					PA_OutputText( 0, 0, 17, "             %c8BACK               " );
 
 					screenFade = 0;			
					PA_SetBrightness( 0, screenFade );
					gameState = RUN_CREDITS;
				}

				runFlyingSprites();

				break;
				
			}




			case RUN_CREDITS: {

				if( bounceCounter < ARRAY_LEN( bounceArray )-1 ) {
					bounceCounter++;
				} else {
					bounceCounter = 0;
				}
				PA_BGScrollY( 1, 2, bounceArray[bounceCounter] );
			
				if( Stylus.Newpress ) {
					if( Stylus.X > 8*9 && Stylus.X < 8*24 ) {
						// check for menu items
												
						u8 tmpRow = 17;
						if( Stylus.Y > 8*( tmpRow-1 ) && Stylus.Y < ( tmpRow+1 )*8 ) {
							// BACK
							screenFade = 0;
							anyCounter = 30;
							gameState = FADE_OUT_CREDITS;
						}							
						
					}

				}

				runFlyingSprites();
				
				break;

			}




			case FADE_OUT_CREDITS: {

				// bounce title logo
				if( bounceCounter < ARRAY_LEN( bounceArray )-1 ) {
					bounceCounter++;
				} else {
					bounceCounter = 0;
				}
				PA_BGScrollY( 1, 2, bounceArray[bounceCounter] );

				if( anyCounter > 0 ) {
					// blink the selected menu option for half a sec
					if( anyCounter % 5 ) PA_OutputText( 0, 0, 17, "                                " );
					else                 PA_OutputText( 0, 0, 17, "             %c8BACK               " );
					anyCounter--;
				} else if( screenFade > -32 ) {
					screenFade--;
					PA_SetBrightness( 0, screenFade );
					// PA_SetBrightness( 1, screenFade );
				} else {
					// clear all text
					PA_ClearTextBg( 0 );
					screenFade = 0;
					PA_SetBrightness( 0, screenFade );
					lockSpriteShow( &lockSpriteNum );
					gameState = RUN_MENU;
				}

				runFlyingSprites();

				break;				
			}



			
			case OPTIONS_INIT: {
			
// add: enable saving (fatlib)
// add: clear saved scores

				if( bounceCounter < ARRAY_LEN( bounceArray )-1 ) {
					bounceCounter++;
				} else {
					bounceCounter = 0;
				}
				PA_BGScrollY( 1, 2, bounceArray[bounceCounter] );

				if( anyCounter > 0 ) {
					// blink the selected menu option for half a sec
					PA_ClearTextBg( 0 );
					PA_OutputText( 0, 12, 8,  "WORD UP!" );
					
					if( hasAward() ) PA_OutputText( 0, 12, 10, "ARCADE" );
					else PA_OutputText( 0, 12, 10, "%c8ARCADE" );
					
					if( anyCounter % 5 ) PA_OutputText( 0, 12, 13, "OPTIONS" );

					PA_OutputText( 0, 12, 15, "CREDITS" );

					if( saveEnabled ) {
						PA_OutputText( 0, 8, 19,  "SAVING ENABLED" );
					} else {
						if( bounceCounter % 3 ) PA_OutputText( 0, 8, 19,  "%c8SAVING DISABLED" );					
					}

					anyCounter--;

				} else if( screenFade > -32 ) {
					screenFade--;
					PA_SetBrightness( 0, screenFade );
					// PA_SetBrightness( 1, screenFade );

				} else {
					// clear all text
 					printOptions();
					screenFade = 0;			
					PA_SetBrightness( 0, screenFade );
					lockSpriteHide( &lockSpriteNum );
					gameState = RUN_OPTIONS;
				}

				runFlyingSprites();

				break;
			
			}
			
			
			
			
			case RUN_OPTIONS: {

				if( bounceCounter < ARRAY_LEN( bounceArray )-1 ) {
					bounceCounter++;
				} else {
					bounceCounter = 0;
				}
				PA_BGScrollY( 1, 2, bounceArray[bounceCounter] );
			
				if( Stylus.Newpress ) {
					if( Stylus.X > 8*9 && Stylus.X < 8*24 ) {
						// check for menu items
						
						u8 tmpRow = 7;
						if( Stylus.Y > 8*( tmpRow-1 ) && Stylus.Y < ( tmpRow+1 )*8 ) {
							randomAlgorithm = SIMPLE;
						}
						
						tmpRow = 8;
						if( Stylus.Y > 8*( tmpRow-1 ) && Stylus.Y < ( tmpRow+1 )*8 ) {
							randomAlgorithm = SUPERPANIC;
						}
/*						
						tmpRow = 9;
						if( Stylus.Y > 8*( tmpRow ) && Stylus.Y < ( tmpRow+1 )*8 ) {
							randomAlgorithm = JAYENKAI;
						}
*/						
						tmpRow = 10;
						if( Stylus.Y > 8*( tmpRow-1 ) && Stylus.Y < ( tmpRow+1 )*8 ) {
							voiceEffects = !voiceEffects;
							// voices on/off
						}
						
						tmpRow = 12;
						if( Stylus.Y > 8*( tmpRow-1 ) && Stylus.Y < ( tmpRow+1 )*8 ) {
							tryEnableSave();
						}

						tmpRow = 17;
						if( Stylus.Y > 8*tmpRow-4 && Stylus.Y < 8*tmpRow+4 ) {
							// BACK
							difficulty = EASY;
						}						

						tmpRow = 16;
						if( Stylus.Y > 8*tmpRow-4 && Stylus.Y < 8*tmpRow+4 ) {
							// BACK
							difficulty = MEDIUM;
						}
						
						tmpRow = 15;
						if( Stylus.Y > 8*tmpRow-4 && Stylus.Y < 8*tmpRow+4 ) {
							// BACK
							difficulty = HARD;
						}


						

						
						tmpRow = 20;
						if( Stylus.Y > 8*( tmpRow-1 ) && Stylus.Y < ( tmpRow+1 )*8 ) {
							// BACK
							screenFade = 0;
							anyCounter = 30;
							gameState = FADE_OUT_OPTIONS;
						}							
						
					}
					printOptions();
				}

				runFlyingSprites();
				
				break;
			}
			
			
			
			
			case FADE_OUT_OPTIONS: {

				// bounce title logo
				if( bounceCounter < ARRAY_LEN( bounceArray )-1 ) {
					bounceCounter++;
				} else {
					bounceCounter = 0;
				}
				PA_BGScrollY( 1, 2, bounceArray[bounceCounter] );

				printOptions();
				if( anyCounter > 0 ) {
					// blink the selected menu option for half a sec
					if( anyCounter % 5 ) PA_OutputText( 0, 9, 20,  "        " );
					anyCounter--;
				} else if( screenFade > -32 ) {
					screenFade--;
					PA_SetBrightness( 0, screenFade );
					// PA_SetBrightness( 1, screenFade );
				} else {
					// clear all text
					PA_ClearTextBg( 0 );
					screenFade = 0;
					PA_SetBrightness( 0, screenFade );
					lockSpriteShow( &lockSpriteNum );
					gameState = RUN_MENU;
				}

				runFlyingSprites();

				break;				
			}
			
			
			
			
			case FADE_OUT_MENU: {
			
				runFlyingSprites();

				if( anyCounter > 0 ) {
					// blink the selected menu option for half a sec
					PA_ClearTextBg( 0 );
					if( gameStyle == CLASSIC ) {
						if( anyCounter % 5 ) PA_OutputText( 0, 12, 8,  "WORD UP!" );

						if( hasAward() ) PA_OutputText( 0, 12, 10, "ARCADE" );
						else PA_OutputText( 0, 12, 10, "%c8ARCADE" );

					} else {

						PA_OutputText( 0, 12, 8,  "WORD UP!" );
						if( anyCounter % 5 )PA_OutputText( 0, 12, 10,  "ARCADE" );						

					}
					PA_OutputText( 0, 12, 13,  "OPTIONS" );

					PA_OutputText( 0, 12, 15, "CREDITS" );

					if( saveEnabled ) {
						PA_OutputText( 0, 8, 19,  "SAVING ENABLED" );
					} else {
						if( anyCounter % 3 ) PA_OutputText( 0, 8, 19,  "%c8SAVING DISABLED" );
					}

					anyCounter--;

				} else if( screenFade > -32 ) {
					// fade both screens
				        screenFade--;
					PA_SetBrightness( 1, screenFade );
					PA_SetBrightness( 0, screenFade );
				} else {

					PA_DeleteBg( 1, 3 );
					PA_DeleteBg( 1, 2 );
					PA_DeleteBg( 1, 1 );
					// delete text background
					PA_DeleteBg( 0, 0 );
					// did not work with just delete, so added ResetBgSys, and it worked!
					PA_ResetBgSys();
					
					level = 0;
					
					lockSpriteDelete( &lockSpriteNum );
					
					cleanFlyingSprites();
					
					gameState = INIT_TUTORIAL;
//					gameState = INIT_GAME;
				}

				break;
			}



			
			case INIT_TUTORIAL: {

				tutorialSprite_G.x = 3*32;
				tutorialSprite_G.y = 3*32;
				tutorialSprite_G.counter = 0;
				tutorialSprite_G.floatXOffset = -(3*32) - 32;
				tutorialSprite_G.timer = 0;
			
				tutorialSprite_A.x = 4*32;
				tutorialSprite_A.y = 3*32;
				tutorialSprite_A.counter = 0;
				tutorialSprite_A.floatXOffset = -(4*32) - 32;
				tutorialSprite_A.timer = 10;
			
				tutorialSprite_M.x = 3*32;
				tutorialSprite_M.y = 2*32;
				tutorialSprite_M.counter = 0;
				tutorialSprite_M.floatXOffset = -(3*32) - 32;
				tutorialSprite_M.timer = 20;
			
				tutorialSprite_E.x = 4*32;
				tutorialSprite_E.y = 2*32;
				tutorialSprite_E.counter = 0;
				tutorialSprite_E.floatXOffset = -(4*32) - 32;
				tutorialSprite_E.timer = 30;



				tutorialFallingBlock1.x 		= 3*32;
				tutorialFallingBlock1.y 		= -32;
				tutorialFallingBlock1.counter 		= 0;
				tutorialFallingBlock1.floatXOffset 	= 0;
				tutorialFallingBlock1.timer 		= 0;

				tutorialFallingBlock2.x 		= 4*32;
				tutorialFallingBlock2.y 		= -32;
				tutorialFallingBlock2.counter 		= 0;
				tutorialFallingBlock2.floatXOffset 	= 0;
				tutorialFallingBlock2.timer 		= 0;


				
				stylusSprite.x = -32;
				stylusSprite.y = -32;
				stylusSprite.floatXOffset = 0;
				stylusSprite.counter = 0;
				stylusSprite.timer = 0;

				tutorialCursor.x = -32;
				tutorialCursor.y = -32;	
				tutorialCursor.floatXOffset = 0;
				tutorialCursor.floatYOffset = 0;
				tutorialCursor.counter = 0;
				tutorialCursor.timer = 0;

				tutorialButtonL.x = 3;
//				tutorialButtonL.y = 176;	
				tutorialButtonL.y = 192;	
				tutorialButtonL.floatXOffset = 0;
				tutorialButtonL.floatYOffset = 0;
				tutorialButtonL.counter = 0;
				tutorialButtonL.timer = 0;

				tutorialButtonR.x = 222;
//				tutorialButtonR.y = 176;	
				tutorialButtonR.y = 192;	
				tutorialButtonR.floatXOffset = 0;
				tutorialButtonR.floatYOffset = 0;
				tutorialButtonR.counter = 0;
				tutorialButtonR.timer = 0;


				/*************************************************
				 *    LOAD CHARACTER BLOCK SPRITE AND PALETTE    *
				 *************************************************/
				// load palette to top screen
				PA_LoadSpritePal(
					1,			// screen
					0,			// palette number
					(void*)wordblocks_Pal	// palette name
				);


				/****************************************
				 *    LOAD STYLUS SPRITE AND PALETTE    *
				 ****************************************/
				// load palette to top screen
				PA_LoadSpritePal(
					1,			// screen
					1,			// palette number
					(void*)stylus_Pal	// palette name
				);


				/****************************************
				 *    LOAD BUTTON SPRITE AND PALETTE    *
				 ****************************************/
				// load palette to top screen
				PA_LoadSpritePal(
					1,			// screen
					2,			// palette number
					(void*)tutorial_gfx_Pal	// palette name
				);


				/******************************************************
				 *    INIT TEXT FUNCTIONS ON TOP AND BOTTOM SCREEN    *
				 ******************************************************/			
				// top screen
				PA_InitText(1,0);	// init the default text functions (screen, background)
			
				PA_SetTextCol(		// set the text columns white
					1,		// screen
					31,		// Red
					31,		// Green
					31		// Blue
				);
				
				// bottom screen
				PA_InitText(0,0);	// init the default text functions (screen, background)
			
				PA_SetTextCol(		// set the text columns white
					0,		// screen
					31,		// Red
					31,		// Green
					31		// Blue
				);


				/************************
				 *    CREATE SPRITES    *
				 ************************/
				PA_CreateSprite(			// create the button sprite
					1,				// screen
					tutorialButtonL.spriteNum,	// sprite number
					(void*)tutorial_gfx_Sprite,	// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					2,				// sprite palette number
					tutorialButtonL.x,		// x pos (* 32)
					tutorialButtonL.y		// y pos
				);


				PA_SetSpriteAnim(
					1,				// screen
					tutorialButtonL.spriteNum,	// sprite number
					1				// set frame to i
				);

				/************************
				 *    CREATE SPRITES    *
				 ************************/
				PA_CreateSprite(			// create the button sprite
					1,				// screen
					tutorialButtonR.spriteNum,	// sprite number
					(void*)tutorial_gfx_Sprite,	// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					2,				// sprite palette number
					tutorialButtonR.x,		// x pos (* 32)
					tutorialButtonR.y		// y pos
				);

				PA_SetSpriteAnim(
					1,				// screen
					tutorialButtonR.spriteNum,	// sprite number
					2				// set frame to i
				);


				/************************
				 *    CREATE SPRITES    *
				 ************************/
				stylusSprite.x = -32;
				stylusSprite.y = HALF_SCREEN_HEIGHT-16;

				PA_CreateSprite(			// create the cursor (selection rectangle) sprite
					1,				// screen
					stylusSprite.spriteNum,		// sprite number
					(void*)stylus_Sprite,		// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					1,				// sprite palette number
					stylusSprite.x,			// x pos (* 32)
					stylusSprite.y			// y pos
				);
				

//				stylusSprite.floatXOffset = -HALF_SCREEN_WIDTH-32;

				stylusSprite.timer = 30;

				/************************
				 *    CREATE SPRITES    *
				 ************************/
				PA_CreateSprite(			// create the cursor (selection rectangle) sprite
					1,				// screen
					tutorialSprite_G.spriteNum,	// sprite number
					(void*)wordblocks_Sprite,		// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					0,				// sprite palette number
					-32,				// x pos (* 32)
					tutorialSprite_G.y		// y pos
				);

				PA_SetSpriteAnim(
					1,				// screen
					tutorialSprite_G.spriteNum,	// sprite number
					'G'-'A'				// set frame to i
				);

//				tutorialSprite_G.x = -32;
//				tutorialSprite_G.y = HALF_SCREEN_HEIGHT-16;

				PA_CreateSprite(			// create the cursor (selection rectangle) sprite
					1,				// screen
					tutorialSprite_A.spriteNum,	// sprite number
					(void*)wordblocks_Sprite,		// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					0,				// sprite palette number
					-32,				// x pos (* 32)
					tutorialSprite_A.y		// y pos
				);

				PA_SetSpriteAnim(
					1,				// screen
					tutorialSprite_A.spriteNum,	// sprite number
					'A'-'A'				// set frame to i
				);

//				tutorialSprite_A.x = -32;
//				tutorialSprite_A.y = HALF_SCREEN_HEIGHT-16;			
				
				PA_CreateSprite(			// create the cursor (selection rectangle) sprite
					1,				// screen
					tutorialSprite_M.spriteNum,	// sprite number
					(void*)wordblocks_Sprite,		// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					0,				// sprite palette number
					-32,				// x pos (* 32)
					tutorialSprite_M.y		// y pos
				);

				PA_SetSpriteAnim(
					1,				// screen
					tutorialSprite_M.spriteNum,	// sprite number
					'M'-'A'				// set frame to i
				);

//				tutorialSprite_M.x = -32;
//				tutorialSprite_M.y = HALF_SCREEN_HEIGHT-16;

				PA_CreateSprite(			// create the cursor (selection rectangle) sprite
					1,				// screen
					tutorialSprite_E.spriteNum,	// sprite number
					(void*)wordblocks_Sprite,		// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					0,				// sprite palette number
					-32,				// x pos (* 32)
					tutorialSprite_E.y		// y pos
				);

				PA_SetSpriteAnim(
					1,				// screen
					tutorialSprite_E.spriteNum,	// sprite number
					'E'-'A'				// set frame to i
				);



				/**************************************
				 *    CREATE FALLING BLOCK SPRITES    *
				 **************************************/
				PA_CreateSprite(			// create a falling block
					1,				// screen
					tutorialFallingBlock1.spriteNum,	// sprite number
					(void*)wordblocks_Sprite,		// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					0,				// sprite palette number
					tutorialFallingBlock1.x,	// x pos (* 32)
					tutorialFallingBlock1.y		// y pos
				);

				PA_SetSpriteAnim(
					1,				// screen
					tutorialFallingBlock1.spriteNum,	// sprite number
					'D'-'A'				// set frame to i
				);
				


				PA_CreateSprite(			// create a falling block
					1,				// screen
					tutorialFallingBlock2.spriteNum,	// sprite number
					(void*)wordblocks_Sprite,		// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					0,				// sprite palette number
					tutorialFallingBlock2.x,	// x pos (* 32)
					tutorialFallingBlock2.y		// y pos
				);

				PA_SetSpriteAnim(
					1,				// screen
					tutorialFallingBlock2.spriteNum,	// sprite number
					'S'-'A'				// set frame to i
				);


/*
	PA_OutputText( 1, 4, 4, "spritenum: %d", tutorialCursor.spriteNum );
	PA_WaitFor( (Stylus.Newpress) );
*/			
				
				// TUTORIAL CURSOR
				PA_CreateSprite(			// create the cursor (selection rectangle) sprite
					1,				// screen
					tutorialCursor.spriteNum,	// sprite number
					(void*)wordblocks_Sprite,	// sprite name
					OBJ_SIZE_32X32,			// sprite size
					1,				// 256 color mode
					0,				// sprite palette number
					tutorialCursor.x,		// x pos (* 32)
					tutorialCursor.y		// y pos
				);
					
				PA_SetSpriteAnim(
					1,			// screen
					tutorialCursor.spriteNum,	// sprite number
					27			// set frame to i
				);
			
			
				PA_StartSpriteAnim(		// Start the animation. Once started, it works on its own!
					1,			// screen
					tutorialCursor.spriteNum,	// sprite number
					27,			// first frame
					29,			// last frame
					12			// Speed, n frames per second
				);

//				tutorialSprite_E.x = -32;
//				tutorialSprite_E.y = HALF_SCREEN_HEIGHT - 16;

				PA_OutputText( 0, 16-6, 12, "HOW TO PLAY!" );
				PA_OutputText( 0, 16-11, 21, "%c8TOUCH TO SKIP TUTORIAL!" );
				gameState = FADE_IN_TUTORIAL;

				break;				
			}










			case FADE_IN_TUTORIAL: {
				if( screenFade < 0 ) {
				        screenFade++;
					PA_SetBrightness( 0, screenFade );
					PA_SetBrightness( 1, screenFade );
				} else {
					gameState = WAIT_FOR_NEXT_TUTORIAL;
					nextState = TUTORIAL_TITLE;
					stateTimer = 60;
				}
				if( Stylus.Newpress ) gameState = FADE_OUT_TUTORIAL;
				break;
			}
			
			case TUTORIAL_TITLE: {
				PA_ClearTextBg( 0 );
				gameState = WAIT_FOR_NEXT_TUTORIAL;
				nextState = RUN_TUTORIAL_A;
				stateTimer = 30;
				PA_OutputText( 0, 16-11, 21, "%c8TOUCH TO SKIP TUTORIAL!" );
				if( Stylus.Newpress ) gameState = FADE_OUT_TUTORIAL;
				break;
			}

			case RUN_TUTORIAL_A: {
				PA_ClearTextBg( 0 );

				PA_OutputText( 0, 16-11, 21, "%c8TOUCH TO SKIP TUTORIAL!" );

				PA_OutputText( 0, 16-7, 12, "USE THE STYLUS" );
				// textFader( "USE STYLUS" );

				if( stylusSprite.timer > 0 ) {
					stylusSprite.timer--;
					break;
				} 


				if( moveStylus( HALF_SCREEN_WIDTH-16, HALF_SCREEN_HEIGHT-16, &stylusSprite ) ) {
					gameState = WAIT_FOR_NEXT_TUTORIAL;
					nextState = RUN_TUTORIAL_B;
					stateTimer = 120;
				}

				// "use the stylus"
				// fly in the stylus
				if( Stylus.Newpress ) gameState = FADE_OUT_TUTORIAL;				
				break;
			}

			case RUN_TUTORIAL_B: {
//				PA_ClearTextBg( 1 );
//				textFader( "DRAG TO SELECT WORDS" );

				// animate blocks to fly in
				bool flag = true;
				for( i=0; i<ARRAY_LEN(tutorialSpriteArray); i++ ) {
					if( !moveStylus( HALF_SCREEN_WIDTH + 60, HALF_SCREEN_HEIGHT - 60, &stylusSprite ) ) {
						flag = false;
					} else if( (*tutorialSpriteArray[i]).timer > 0) {
						(*tutorialSpriteArray[i]).timer--;
						flag = false;
					} else {
						if( (*tutorialSpriteArray[i]).counter < ARRAY_LEN( smoothMotionArray ) ) {
							PA_SetSpriteX( 1, (*tutorialSpriteArray[i]).spriteNum, (*tutorialSpriteArray[i]).x + (s16)( (*tutorialSpriteArray[i]).floatXOffset * smoothMotionArray[ (*tutorialSpriteArray[i]).counter ]  ) );
							(*tutorialSpriteArray[i]).counter++;
							flag = false;
						}
					}
				}
				
				// all blocks are positioned and ready
				if( flag ) {
					anyCounter = 0;
					gameState = WAIT_FOR_NEXT_TUTORIAL;
					nextState = RUN_TUTORIAL_C;
					stateTimer = 120;
				}
				
				// select and find english words
				// select 'GAME'
				// print GAME as text and blink
				if( Stylus.Newpress ) gameState = FADE_OUT_TUTORIAL;
				break;
			}



			case RUN_TUTORIAL_C: {
				// PA_ClearTextBg( 1 );
				
//				PA_OutputText( 0, 0, 12, "         USE THE STYLUS         " );

				// textFader( "FIND WORDS" );
				// words must be longer than two character
				// select ME
				// failure sound
				switch( anyCounter ) {
					case 0: 
						PA_ClearTextBg( 1 );
						anyCounter++;
						break;

					case 40:
						PA_OutputText( 0, 0, 12, "                                " );
						anyCounter++;
						break;

					case 50:
						if(   moveStylus(   PA_GetSpriteX( 1, tutorialSprite_G.spriteNum )+16, PA_GetSpriteY( 1, tutorialSprite_G.spriteNum )-16, &stylusSprite  )   ) {
							PA_OutputText( 1, 16-2, 5, "G" );
							PA_SetSpriteXY( 1, tutorialCursor.spriteNum, PA_GetSpriteX(1, tutorialSprite_G.spriteNum), PA_GetSpriteY(1, tutorialSprite_G.spriteNum) );
							tutorialCursor.x = PA_GetSpriteX(1, tutorialSprite_G.spriteNum);
							tutorialCursor.y = PA_GetSpriteY(1, tutorialSprite_G.spriteNum);
							tutorialCursor.floatXOffset = PA_GetSpriteX(1, tutorialSprite_G.spriteNum);;
							tutorialCursor.floatYOffset = PA_GetSpriteY(1, tutorialSprite_G.spriteNum);;
							PA_OutputText( 0, 0, 12, "   SELECT THREE OR MORE BLOCKS  " );
							anyCounter++;
						}
						break;



					case 100:
						if(   moveStylus(   PA_GetSpriteX( 1, tutorialSprite_A.spriteNum )+16, PA_GetSpriteY( 1, tutorialSprite_A.spriteNum )-16, &stylusSprite  )   ) {
							// PA_OutputText( 1, 16-2, 5, "GA" );
							anyCounter++;
						}
						break;
						
					case 110:
						if(   moveStylus(   PA_GetSpriteX( 1, tutorialSprite_A.spriteNum ), PA_GetSpriteY( 1, tutorialSprite_A.spriteNum ), &tutorialCursor  )   ) {
							PA_OutputText( 1, 16-2, 5, "GA" );
							anyCounter++;
						}
						break;						



					case 150:
						if(   moveStylus(   PA_GetSpriteX( 1, tutorialSprite_M.spriteNum )+16, PA_GetSpriteY( 1, tutorialSprite_M.spriteNum )-16, &stylusSprite  )   ) {
							// PA_OutputText( 1, 16-2, 5, "GAM" );
							anyCounter++;
						}
						break;
						
					case 160:
						if(   moveStylus(   PA_GetSpriteX( 1, tutorialSprite_M.spriteNum ), PA_GetSpriteY( 1, tutorialSprite_M.spriteNum ), &tutorialCursor  )   ) {
							PA_OutputText( 1, 16-2, 5, "GAM" );
							anyCounter++;
						}
						break;											



					case 200:
						if(   moveStylus(   PA_GetSpriteX( 1, tutorialSprite_E.spriteNum )+16, PA_GetSpriteY( 1, tutorialSprite_E.spriteNum )-16, &stylusSprite  )   ) {
							// PA_OutputText( 1, 16-2, 5, "GAME" );
							anyCounter++;
						}
						break;
						
					case 210:
						if(   moveStylus(   PA_GetSpriteX( 1, tutorialSprite_E.spriteNum ), PA_GetSpriteY( 1, tutorialSprite_E.spriteNum ), &tutorialCursor  )   ) {
							PA_OutputText( 1, 16-2, 5, "GAME" );
							anyCounter++;
						}
						break;


						
					case 300:
						if(   moveStylus(   PA_GetSpriteX( 1, tutorialSprite_E.spriteNum )+128, PA_GetSpriteY( 1, tutorialSprite_E.spriteNum )-16, &stylusSprite  )   ) {
//							PA_OutputText( 1, 16-2, 5, "GAME" );
							PA_SetSpriteXY( 1, tutorialCursor.spriteNum, -32, -32 );
							PA_OutputText( 0, 0, 12, "                                " );
							anyCounter++;
						}
						break;


					case 310:

//						PA_OutputText( 0, 0, 12, "         USE THE STYLUS         " );
						PA_OutputText( 0, 0, 12, "       FIND WORDS TO SCORE      " );
						anyCounter++;
	 					break;




					case 500:
        					moveStylus(   222, 192-16, &tutorialButtonR  );
						if(  moveStylus(   3, 192-16, &tutorialButtonL  )  ) anyCounter++;
						break;

					case 540:
						PA_OutputText( 0, 0, 12, "                                " );
						anyCounter++;
						break;

					case 550:
						PA_OutputText( 0, 0, 12, " USE L-R BUTTONS TO DROP BLOCKS " );
						anyCounter++;
	 					break;

					case 600:
						if(  moveStylus(   3, 192-16+5, &tutorialButtonL  )  ) anyCounter++;
						break;

					case 601:
						if(  moveStylus(   3, 192-16, &tutorialButtonL  )  ) anyCounter++;
						break;




					case 602:
						if(  moveStylus(   32*3, 32, &tutorialFallingBlock1  )  ) anyCounter++;
						break;




					case 610:
						if(  moveStylus(   222, 192-16+5, &tutorialButtonR  )  ) anyCounter++;
						break;

					case 611:
						if(  moveStylus(   222, 192-16, &tutorialButtonR  )  ) anyCounter++;
						break;



					case 612:
						if(  moveStylus(    32*4, 32, &tutorialFallingBlock2  )  ) anyCounter++;
						break;





					default:
						if( anyCounter < 750 ) {
							
							if( anyCounter > 300 && anyCounter < 500 ) {
								if( anyCounter % 10 < 5 ) { 
									PA_OutputText( 1, 16-2, 5, "GAME" );
								} else {
									PA_OutputText( 1, 16-2, 5, "    " );	
								}
							}
							
							anyCounter++;
						} else {
							anyCounter = 0;
							gameState = FADE_OUT_TUTORIAL;
						}
						
						break;
				}
				
				if( Stylus.Newpress ) gameState = FADE_OUT_TUTORIAL;				
				// move the cursor if active
				break;
			}			


			case WAIT_FOR_NEXT_TUTORIAL: {
				if( stateTimer > 0 ) {
					stateTimer--;
				} else {
					gameState = nextState;
				}
				if( Stylus.Newpress ) gameState = FADE_OUT_TUTORIAL;				
				break;
			}


			case FADE_OUT_TUTORIAL: {

				if( screenFade > -32 ) {
				        screenFade--;
					PA_SetBrightness( 0, screenFade );
					PA_SetBrightness( 1, screenFade );
				} else {
					gameState = CLEAR_TUTORIAL;
				}
				break;
			}

			case CLEAR_TUTORIAL: {

				// clear all backgrounds and memory!
				PA_ResetBgSys();

				// clear stylus sprite
				PA_DeleteSprite( 1, stylusSprite.spriteNum );

				PA_DeleteSprite( 1, tutorialButtonL.spriteNum );
				PA_DeleteSprite( 1, tutorialButtonR.spriteNum );

				// clear falling block sprites
				PA_DeleteSprite( 1, tutorialFallingBlock1.spriteNum );
				PA_DeleteSprite( 1, tutorialFallingBlock2.spriteNum );

				// clear block sprites
				PA_DeleteSprite( 1, tutorialSprite_G.spriteNum );
				PA_DeleteSprite( 1, tutorialSprite_A.spriteNum );
				PA_DeleteSprite( 1, tutorialSprite_M.spriteNum );
				PA_DeleteSprite( 1, tutorialSprite_E.spriteNum );

				PA_StopSpriteAnim( 1, tutorialCursor.spriteNum );				
				PA_DeleteSprite( 1, tutorialCursor.spriteNum );

			PA_StopMod();
				
				gameState = INIT_GAME;

				break;
			}































































			case INIT_GAME: {
				/********************************************************
				 *    LOAD BACKGROUND GFX, PALETTES AND SET POSITION    *
				 ********************************************************/
				PA_LoadTiledBg( 1, 3, bg_starlow );
				PA_LoadTiledBg( 1, 2, bg_starhigh );
				PA_LoadTiledBg( 0, 3, bg_sky );
				PA_BGScrollY( 0, 3, 32 ); // move bottom background 32 pixels down



				/*******************************************
				 *    INIT TEXT FUNCTIONS ON TOP SCREEN    *
				 *******************************************/			
				PA_InitText(1,0);	// init the default text functions (screen, background)
			
				PA_SetTextCol(		// set the text columns white
					1,		// screen
					31,		// Red
					31,		// Green
					31		// Blue
				);



//!				/*******************************************
//!				 *    INIT TEXT FUNCTIONS ON BOTTOM SCREEN    *
//!				 *******************************************/

				PA_InitText(0,0);	
							
				PA_SetTextCol(		// set the text columns white
					0,		// screen
					31,		// Red
					31,		// Green
					31		// Blue
				);				



				/***********************************
				 *    LOAD GAME SPRITE PALETTES    *
				 ***********************************/
				// load palette to bottom screen
				if( palIsNotLoaded ) {
					PA_LoadSpritePal(
						0,			// screen
						0,			// palette number
						(void*)wordblocks_Pal	// palette name
					);
				
					
					// load palette to top screen
					PA_LoadSpritePal(
						1,			// screen
						0,			// palette number
						(void*)wordblocks_Pal	// palette name
					);
					palIsNotLoaded = false;
				}

				// create a list of random characters
				// fillBag();

				/***********************************************
				 *    CREATE GAME SPRITES FOR BOTTOM SCREEN    *
				 ***********************************************/
				for( yi=0; yi < GRIDH; yi++ ) {
					for( xi=0; xi < GRIDW; xi++ ) {

						switch( randomAlgorithm ) {
							case SIMPLE:
								blockArray[yi][xi].character = simpleRandomChar();								
								break;
							case SUPERPANIC:
								blockArray[yi][xi].character = superpanicRandomChar();								
								break;
							case JAYENKAI:
								blockArray[yi][xi].character = jayenkaiRandomChar();								
								break;
						}

						PA_CreateSprite(
							0,				// screen
							blockArray[yi][xi].spriteNum,	// sprite number
							(void*)wordblocks_Sprite,	// sprite name
							OBJ_SIZE_32X32,			// sprite size
							1,				// 256 color mode
							0,				// sprite palette number
							blockArray[yi][xi].x,		// x pos (* 32)
							blockArray[yi][xi].y		// y pos
						);
						PA_SetSpriteAnim(
							0,				// screen
							blockArray[yi][xi].spriteNum,	// sprite number
							blockArray[yi][xi].character	// set frame to i
						);
						
						// let the text background print on top of sprites
						PA_SetSpritePrio(0, blockArray[yi][xi].spriteNum, 1);						
						blockArray[yi][xi].velocity = 0;
						blockArray[yi][xi].yoffset = 0;
						blockArray[yi][xi].zoom = 256;
						// used for bomb animation
						blockArray[yi][xi].explode = false;
						blockArray[yi][xi].explosionCounter = 0;
						// used for game over animation
						blockArray[yi][xi].xFloatOffset = 0;
						blockArray[yi][xi].yFloatOffset = 0;
						blockArray[yi][xi].xMomentum = (float)(PA_RandMinMax(0,4))-2;
						blockArray[yi][xi].yMomentum = (float)(PA_RandMinMax(0,4))-2;


						float fx = (float)blockArray[yi][xi].x + 16.0;
						
						if( fx >= 144 ) {
							// right side
							blockArray[yi][xi].xSlideSpeed = (255 - fx) / 10;
						} else {
							// left side
							blockArray[yi][xi].xSlideSpeed = -fx / 10;
						}
						
						// used to animate a dissapearing block
						blockArray[yi][xi].busy = false;
						blockArray[yi][xi].rotSet = 0;
						blockArray[yi][xi].rotDelay = 0;
						blockArray[yi][xi].active = true;
					}
				}
				
				
				/************************************************
				 *    CREATE SCORE SPRITES FOR BOTTOM SCREEN    *
				 ************************************************/
				for( i = 0; i < 8; i++ ) {	
					PA_CreateSprite(
						0,				// screen
						scoreSpriteArray[i],		// sprite number
						(void*)wordblocks_Sprite,	// sprite name
						OBJ_SIZE_32X32,			// sprite size
						1,				// 256 color mode
						0,				// sprite palette number
						-64,				// x pos
						-64				// y pos
					);	
				}
				


				/***************************************
				 *    RESET AND CREATE WARNING TABS    *
				 ***************************************/
				for( i=0; i<8; i++ ) {
					warningTabs[i].active = false;
					warningTabs[i].counter = 600;
					warningTabs[i].movementCounter = ARRAY_LEN( WARNING_TAB_MOVEMENT )-1;

					// create the sprites					
					PA_CreateSprite(					// create the cursor (selection rectangle) sprite
						0,						// screen
						warningTabs[i].spriteNum,			// sprite number
						(void*)wordblocks_Sprite,			// sprite name
						OBJ_SIZE_32X32,					// sprite size
						1,						// 256 color mode
						0,						// sprite palette number
						warningTabs[i].xpos,				// x pos (* 32)
						warningTabs[i].ypos + warningTabs[i].offset	// y pos
					);
						
					PA_SetSpriteAnim(
						0,				// screen
						warningTabs[i].spriteNum,	// sprite number
						46				// set frame to 46
					);
					
					// let the text background print on top of sprites
					PA_SetSpritePrio(0, warningTabs[i].spriteNum, 1);

				}
				


			
			
				
			
				/********************
				*     GAME SPEED    *
				*********************/
				switch( difficulty ){
        				case HARD:
        					gameClockSpeed = 40;
        					break;
        				case MEDIUM:
        					gameClockSpeed = 110;
        					break;
        				case EASY:
        					gameClockSpeed = 200;
        					break;
				}

				// gameSpeed = 300;
				gameSpeed = 100;
				// gameSpeed = 10;
				blockTimer = getNewBlockTime(); // get the time for next top block to appear
				// the game speed timer increases faster by each level
				if( gameState == ARCADE ) {
					gameSpeedTimer = MAX( gameClockSpeed - level * 5, 10 );
				} else {
					gameSpeedTimer = gameClockSpeed;				
				}
				
				
				
				/********************
				*     LEVELGOAL     *
				*********************/				
				showMissionTimer = 160;
				levelGoal = levelGoals[level];
				
				/*********************
				*     RESET TOUCH    *
				**********************/				
				firstTouch = true;
				pressed = FALSE;

				levelClearFlag = false;
				
				
				/**************************
				*     GET READY VOICE!    *
				***************************/	
				s8 tmpsnd;
				tmpsnd = GetFreeLimitedSoundChannel();
				if( tmpsnd > -1 && voiceEffects ) PA_PlaySimpleSound( (u8)tmpsnd, voice_getready );


				gameState = FADE_IN_GAME;
				break;
			}


			
			case FADE_IN_GAME: {
				if( screenFade < 0 ) {
				        screenFade++;
					PA_SetBrightness( 0, screenFade );
					PA_SetBrightness( 1, screenFade );
				} else {
				
					gameState = RUN_GAME;
				}
				
				break;
			}



			case RUN_GAME: {
			        
				// clear all text
				PA_ClearTextBg( 1 );

displayTopInfo( &nSelected, currentWord );		

				PA_ClearTextBg( 0 );

				// update stylus
				// PA_UpdateStylus();
				
				if( showMissionTimer > 0 ) {
					

/*
##################################################
						PLAY MOD FILE
##################################################
*/

if( showMissionTimer == 100 && !musicIsPlaying ) {
	PA_PlayMod( music_ingame );
	PA_SetModVolume( 90 );
	balanceSound();
	musicIsPlaying = true;
}


/*
##################################################
                END PLAY MOD FILE
##################################################
*/
					
					if( gameStyle == ARCADE ) {
						PA_OutputText( 1, 9, 15,  "    LEVEL %d   ", level+1 );
						if( ( gameSpeedTimer % 10 ) < 5 ) {
							PA_OutputText( 1, 9, 17,  "CLEAR %d WORDS!", levelGoal );
						}
					} else {
						if( ( gameSpeedTimer % 10 ) < 5 ) {
							PA_OutputText( 1, 7, 17,  "BEAT HISCORE: %d", hiscoreArray[0] );
						}						
					}
					showMissionTimer--;
				}
				
			
				if( Stylus.Held && !levelClearFlag ) {
					
					// get pointer to the currently touched block, else NULL
					p_currentBlock = getPointerToTouchedCharBlock( blockArray );
					// if we touched a block print text to top screen, else clear text
					if( p_currentBlock ) {
					        
		
		
		
						// the only legal destinations
						s8 x1 = (*p_currentBlock).gridx;
						s8 y1 = (*p_currentBlock).gridy;
						s8 x2 = (*p_lastBlock).gridx;
						s8 y2 = (*p_lastBlock).gridy;
		
						bool okToAdd = false;
		
						if( nSelected == 0 ) okToAdd = true;				
						if ( x2 - x1 == -1 && y2 - y1 == -1 ) okToAdd = true;
						if ( x2 - x1 ==  0 && y2 - y1 == -1 ) okToAdd = true;
						if ( x2 - x1 ==  1 && y2 - y1 == -1 ) okToAdd = true;
						if ( x2 - x1 == -1 && y2 - y1 ==  0 ) okToAdd = true;
						if ( x2 - x1 ==  1 && y2 - y1 ==  0 ) okToAdd = true;
						if ( x2 - x1 == -1 && y2 - y1 ==  1 ) okToAdd = true;
						if ( x2 - x1 ==  0 && y2 - y1 ==  1 ) okToAdd = true;
						if ( x2 - x1 ==  1 && y2 - y1 ==  1 ) okToAdd = true;
		
		
		
						
						
								
						if( ( *p_currentBlock ).spriteNum != ( *p_lastBlock ).spriteNum && okToAdd ) {
		
		
		
		
							// is the currently selected already selected?
							u8 i;
							for( i=0; i < nSelected; i++ ) {
								if( ( *p_selectedBlocksArray[i] ).spriteNum == (*p_currentBlock).spriteNum ) {
									clearSelectionRange( p_selectedBlocksArray, &nSelected, i );
									s8 j = strlen( currentWord )-1;						
									while( j >= i ) {
										currentWord[ j ] = '\0';
										j--;
									}
									break;						
								}
							}
		
		
		
		
		
							// add current block to the word
							buildWord( p_currentBlock );
							
							
							if( nSelected < MAX_WORD_LENGTH ) {
								// set sprite mode of the current block to alpha blending
							        PA_SetSpriteMode( 0, (*p_currentBlock).spriteNum, 1 ); // screen, sprite, alphablending
								// add selected block to selectionlist
								p_selectedBlocksArray[nSelected] = p_currentBlock;
		
								
								
								
								
								
								
								// set cursor movement destination
								if( nSelected == 0 && firstTouch ) {
									firstTouch = false;
									cursor.xpos = (u8)(*p_currentBlock).x;
									cursor.ypos = (u8)(*p_currentBlock).y;
									PA_SetSpriteXY( 0, cursor.spriteNum, cursor.xpos, cursor.ypos );
								
								} else {
								        cursor.xpos = (u8)(*p_lastBlock).x;
									cursor.ypos = (u8)(*p_lastBlock).y;
									PA_SetSpriteXY( 0, cursor.spriteNum, cursor.xpos, cursor.ypos );
									// start cursor movement						
									switch( (*p_currentBlock).spriteNum - (*p_lastBlock).spriteNum ) {
										case -GRIDW:
											cursor.target=0;
										        break;
										        
										case -GRIDW+1:
											cursor.target=1;
										        break;
		
										case 1:
											cursor.target=2;
										        break;
		
										case GRIDW+1:
										        cursor.target=3;
										        break;
		
										case GRIDW+0:
											cursor.target=4;
										        break;
										        
										case GRIDW-1:
											cursor.target=5;
											break;
										        
										case -1:
											cursor.target=6;
											break;
			
										case -GRIDW-1:
											cursor.target=7;
											break;
											
									}
									cursor.idle = false;
									cursor.stepCounter = 0;
								}
								
								
								
								
								
								
								
								// increase number of selected blocks
								nSelected++;
								// set last block to current block
								p_lastBlock = p_currentBlock;
								
														
		
		
							}
							
							
							
							
							
							
						}
					}
					if(!pressed) { 
						// reset pressed to true
						pressed = TRUE;
						// reset realWord
						realWord = false;
					}			
		
		
				
				
				} else if ( pressed ) {  // stylus is released
					firstTouch = true;
					hideCursor( &cursor );
					
					/*****************************************************
					*  find the current word in the lexicon              *
					*  if found, add score and let the selected blocks   *
					*  dissapear.                                        *
					******************************************************/
					
					if( starSelected( currentWord ) ) {
		
						u8 n;
						for( n = 0; n < nSelected; n++ ) {
							// if the block is not exploding, then activate zoom function:
							(*p_selectedBlocksArray[n]).explode = true;
							(*p_selectedBlocksArray[n]).busy = true;
							(*p_selectedBlocksArray[n]).rotDelay = n * 10;				
						}
						realWord = true;
						
					} else if( searchLexicon( currentWord ) ) {

						if( nSelected >= 5 ) {
							addBomb = true;
						}

						// save statistics
						if( longestWord < nSelected ) {
						        longestWord = nSelected;

						        if(longestWord == 7) award_c = true;
//						        if(longestWord > 1) award_c = true;


							strcpy( longestWordStr, currentWord );
						}

						if( longestLevelWord < nSelected ) {
							longestLevelWord = nSelected;
							strcpy( longestLevelWordStr, currentWord );
						}

						charsCollected = charsCollected + nSelected;
						
						// increase size of allwords array if too small
						if( numberOfWords < MAX_NUMBER_OF_WORDS ) {							
							// add word to allwords array	
							strcpy( allWordsArray[numberOfWords], currentWord );
							numberOfWords++;

							if( !award_b && numberOfWords >= 100 ) award_b = true;
							if( !award_a && numberOfWords >= 200 ) award_a = true;

//							if( !award_b && numberOfWords >= 2 ) award_b = true;
//							if( !award_a && numberOfWords >= 3 ) award_a = true;

						}

						numberOfLevelWords++;

						if( gameStyle == ARCADE ) { 
							if( numberOfLevelWords >= levelGoal ) levelClearFlag = true;
						}

/*
						if( levelClearFlag ) {
							// play word complete sound	
							s8 tmpsnd;
							tmpsnd = GetFreeLimitedSoundChannel();
							if( tmpsnd > -1 && voiceEffects ) PA_PlaySimpleSound( (u8)tmpsnd, voice_levelclear );
						}
*/

						if( gameStyle == ARCADE && levelClearFlag == true ) {
							// play word complete sound	
							s8 tmpsnd;
							tmpsnd = GetFreeLimitedSoundChannel();
							if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_levelclear );
						} else {
							// play word complete sound	
							s8 tmpsnd;
							tmpsnd = GetFreeLimitedSoundChannel();
							if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, voice_wordup );        
						}						
						
						
						if( showScoreFlag ) {
							scoreDisplayCounter = 0;
							clearScoreDisplay( scoreSpriteArray, &showScoreFlag, &scoreDisplayMotionCounter );
						}
						
						addAndDisplayWordScore( currentWord, nSelected, scoreSpriteArray, &showScoreFlag, (*p_selectedBlocksArray[nSelected-1]).x, (*p_selectedBlocksArray[nSelected-1]).y, &scoreYPos );

						// hide the mission brief
						// if( showMissionTimer > 0 ) showMissionTimer = 0;

						realWord = true;
						u8 n;
						for( n = 0; n < nSelected; n++ ) {
							// Activate rotations for selected sprite
							PA_SetSpriteRotEnable(
								0,	// screen
								(*p_selectedBlocksArray[n]).spriteNum,	// sprite number
								n	// rotset number.
							);
							(*p_selectedBlocksArray[n]).rotSet = n;
							(*p_selectedBlocksArray[n]).busy = true;
							(*p_selectedBlocksArray[n]).rotDelay = n * 10;
							PA_SetSpriteDblsize(0, (*p_selectedBlocksArray[n]).spriteNum, 1 );
							PA_SetSpriteXY(0, (*p_selectedBlocksArray[n]).spriteNum, (*p_selectedBlocksArray[n]).x-16, (*p_selectedBlocksArray[n]).y-16 );
						}



// GAME SPEED						
						if( gameSpeed + 1 * nSelected > 100 ) {
							gameSpeed = 100;
						} else {
							gameSpeed = gameSpeed + 1 * nSelected;
						}


						

					} else {

						realWord = false;

					}				
		
		
					// clear everything that was selected
					p_lastBlock = NULL;
					memset(currentWord, '\0', sizeof(currentWord));
					clearSelection( p_selectedBlocksArray, nSelected );
					nSelected = 0;
					pressed = FALSE;
					
					
					
				}
		
		
		
		
		
		
		
		
		
				// move the cursor if active
				runCursor( &cursor );
		
		
		
		
		
		
		
		
		/*
				// print current word to top screen
				if( realWord ) { 
					PA_OutputText( 1, 2, 2, "WORD OK!", currentWord );
				}
				PA_OutputText( 1, 2, 1, "CURRENT WORD: %s", currentWord );
		*/
		
		
		
				
				
				
		
				// calculate new alpha
				alpha = getAlpha( alpha );
				// set alpha
				PA_SetSFXAlpha(0, alpha, 15); // screen, alpha value (0-15), set to 15
		
		
		
		
		
		
		
				// zoom and hide all sprites marked as 'busy'
				for( yi=0; yi < GRIDH; yi++ ) {
					for( xi=0; xi < GRIDW; xi++ ) {
						if( blockArray[yi][xi].busy == false ) continue;				
						if( blockArray[yi][xi].rotDelay > 0 ) {
							blockArray[yi][xi].rotDelay--;
							continue;
						}
						
						if( blockArray[yi][xi].explode ) {
							
							if( blockArray[yi][xi].explosionCounter < EXPLOSION_FRAMES ) {
								if( blockArray[yi][xi].explosionCounter == 0 ) {
									// play block explosion sound	
									s8 tmpsnd;
									tmpsnd = GetFreeLimitedSoundChannel();
									if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_bomb );
								}
								// animate explosion
								PA_SetSpriteAnim(
									0,				// screen
									blockArray[yi][xi].spriteNum,	// sprite number
									explosionAnimationArray[ blockArray[yi][xi].explosionCounter ]	// set frame to i
								);
								blockArray[yi][xi].explosionCounter++;
							} else {
								PA_DeleteSprite( 0, blockArray[yi][xi].spriteNum );
								blockArray[yi][xi].explosionCounter = 0;
								blockArray[yi][xi].explode = false;
								resetCHARBLOCK( &blockArray[yi][xi] );
							}
							
						} else {
							
							// limit to max 512				
							if( blockArray[yi][xi].zoom > 128 ) {


/*
								if( blockArray[yi][xi].zoom == 256 ) {
									// play block explosion sound	
									s8 tmpsnd;
									tmpsnd = GetFreeLimitedSoundChannel();
									if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_zoom );
								}
*/


								// zoom out sprite a bit more
								blockArray[yi][xi].zoom = blockArray[yi][xi].zoom - 16;
								PA_SetRotsetNoAngle(
									0,				// screen
									blockArray[yi][xi].rotSet,	// rotset
									blockArray[yi][xi].zoom,	// horizontal zoom
									blockArray[yi][xi].zoom		// vertical zoom.
								);
								
							} else {
							        // reset zoom and hide current sprite
								blockArray[yi][xi].zoom = 256;
								//PA_SetSpriteX( 0, blockArray[yi][xi].spriteNum, -64 );
								//PA_SetSpriteY( 0, blockArray[yi][xi].spriteNum, -64 );
								PA_SetSpriteRotDisable( 0, blockArray[yi][xi].spriteNum );
								// before deleting the sprite, save the sprites number for later reuse
								// recycledSpriteNums[ nRecycledSprites ] = blockArray[yi][xi].spriteNum;
								// nRecycledSprites++;
								PA_DeleteSprite( 0, blockArray[yi][xi].spriteNum );				
								PA_SetRotsetNoAngle(
									0,				// screen
									blockArray[yi][xi].rotSet,	// rotset
									blockArray[yi][xi].zoom,	// horizontal zoom
									blockArray[yi][xi].zoom		// vertical zoom.
								);
								
								PA_SetSpriteDblsize(0, blockArray[yi][xi].spriteNum,0 );
								resetCHARBLOCK( &blockArray[yi][xi] );
							}
						
						}
						
					}
				}
		
		
		
		
		
		
		
		
		
		
		
				// BOTTOM SCREEN BLOCKS
		
				// move new blocks that are positioned outside of screen with negative yoffset
				yi = 0;
				for( xi=0; xi < GRIDW; xi++ ) {
					if( blockArray[yi][xi].yoffset < 0 ) {
						blockArray[yi][xi].velocity = MIN( 8, blockArray[yi][xi].velocity + 1 );
						blockArray[yi][xi].yoffset += blockArray[yi][xi].velocity;
						if( blockArray[yi][xi].yoffset >= 0 ) {
							PA_SetSpriteY( 0, blockArray[yi][xi].spriteNum, (u8)blockArray[yi][xi].y );
							blockArray[yi][xi].yoffset = 0;
							blockArray[yi][xi].busy = false;
							
							if( blockArray[1][xi].active ) {
								// play block touch sound	
								s8 tmpsnd;
								tmpsnd = GetFreeLimitedSoundChannel();
								if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_blockhit );
							}
							
						} else {
							PA_SetSpriteY( 0, blockArray[yi][xi].spriteNum, (s8)( blockArray[yi][xi].y + blockArray[yi][xi].yoffset ) );
						}
					}
				}
		
				// let all blocks fall if there is an empty space under the block
				for( yi=0; yi < GRIDH-1; yi++ ) { // -1 because we should not check the last line
					for( xi=0; xi < GRIDW; xi++ ) {
						// if the block is busy dissapearing OR not active (empty), continue to next block
						if( blockArray[yi][xi].busy || !blockArray[yi][xi].active ) continue;
						// let current block fall if the block below is NOT active
						if( !blockArray[yi+1][xi].active ) {					
							// move the current block using velocity and offset
							blockArray[yi][xi].velocity = MIN( 8, blockArray[yi][xi].velocity + 1 );
							blockArray[yi][xi].yoffset += blockArray[yi][xi].velocity;
							if( blockArray[yi][xi].yoffset >= 32 ) {
								// if block has moved a whole step (32 pix), swap all block attributes in grid and reset current block						
								PA_DeleteSprite( 0, blockArray[yi][xi].spriteNum );
								blockArray[yi+1][xi].character = blockArray[yi][xi].character;
								PA_CreateSprite( 0, blockArray[yi+1][xi].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[yi+1][xi].x, blockArray[yi+1][xi].y );			
								PA_SetSpriteAnim( 0, blockArray[yi+1][xi].spriteNum, blockArray[yi+1][xi].character );
								PA_SetSpritePrio( 0, blockArray[yi+1][xi].spriteNum, 1);
								blockArray[yi+1][xi].active = true;
								blockArray[yi+1][xi].yoffset = 0;

								if( yi+2 < GRIDH ) {
									if( blockArray[yi+2][xi].active ) {
										// play block touch sound	
										s8 tmpsnd;
										tmpsnd = GetFreeLimitedSoundChannel();
										if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_blockhit );
									}
								} else {
									// play block touch sound	
									s8 tmpsnd;
									tmpsnd = GetFreeLimitedSoundChannel();
									if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_blockhit );
								}

								resetCHARBLOCK( &blockArray[yi][xi] );
								
							} else {
								PA_SetSpriteY( 0, blockArray[yi][xi].spriteNum, (u8)(blockArray[yi][xi].y + blockArray[yi][xi].yoffset) );
							}
						}
		
		
		
					}
				}
		
		
		
		
		
		
		
		
				// TOP SCREEN BLOCKS
		
				// let all top blocks fall, and move them over to bottom screen and game area if there is room.
				// don't move or check the bottom row
				for( yi=0; yi < GRIDH-1; yi++ ) { // -1 because we don't should not check the last line (done later in special case loop below)
					for( xi=0; xi < GRIDW; xi++ ) {
						// if block is not active continue to next step in loop
						if( !topBlockArray[yi][xi].active ) continue;
						
						
						
						
						
						
						
						
						
						// check if a column should bounce as a warning to the player that the column is full!
						if( yi==0 && topBlockArray[1][xi].active && topBlockArray[2][xi].active && topBlockArray[3][xi].active && topBlockArray[4][xi].active && topBlockArray[5][xi].active ) {
							columnsBouncing[ xi ] = true;
						} else if( yi==0 && columnsBouncing[ xi ] ) {





							warningTabs[xi].counter = 600;
							warningTabs[xi].movementCounter = ARRAY_LEN( WARNING_TAB_MOVEMENT )-1;
							warningTabs[xi].offset = WARNING_TAB_MOVEMENT[ warningTabs[xi].movementCounter ];

							PA_SetSpriteY( 0, warningTabs[xi].spriteNum, warningTabs[xi].ypos + warningTabs[xi].offset );

							warningTabs[xi].active = false;


							columnsBouncing[ xi ] = false;
							u8 ti;
							for( ti=0; ti < GRIDH; ti++ ) {
								// reset position of the sprite
								PA_SetSpriteY( 1, topBlockArray[ti][xi].spriteNum, (u8)topBlockArray[ti][xi].y );
								// also reset position of the sprites on the lower screen
								PA_SetSpriteY( 0, blockArray[ti][xi].spriteNum, (u8)blockArray[ti][xi].y );
							}
						}
						
		
						
						
						
						
						
						
						
						// check if block under current block is NOT active
						if( !topBlockArray[yi+1][xi].active ) {
							// if there is room below, increase velocity
							topBlockArray[yi][xi].velocity = MIN( 8, topBlockArray[yi][xi].velocity + 1 );
							// topBlockArray[yi][xi].velocity++;
							// offset position is increased using velocity
							topBlockArray[yi][xi].yoffset += topBlockArray[yi][xi].velocity;
							// check if offset is more than a whole block (32 pixels)
							if( topBlockArray[yi][xi].yoffset >= 32 ) {
								// if block has moved a whole step (32 pix), swap all block attributes in grid and reset current block						
								PA_DeleteSprite( 1, topBlockArray[yi][xi].spriteNum );
								topBlockArray[yi+1][xi].character = topBlockArray[yi][xi].character;
								PA_CreateSprite( 1, topBlockArray[yi+1][xi].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, topBlockArray[yi+1][xi].x, topBlockArray[yi+1][xi].y );			
								PA_SetSpriteAnim( 1, topBlockArray[yi+1][xi].spriteNum, topBlockArray[yi+1][xi].character );

								PA_SetSpritePrio(1, topBlockArray[yi+1][xi].spriteNum, 1);
								
								topBlockArray[yi+1][xi].active = true;
								topBlockArray[yi+1][xi].yoffset = 0;

								if( yi+2 < GRIDH ) {
									if( topBlockArray[yi+2][xi].active ) {
										// play block touch sound	
										s8 tmpsnd;
										tmpsnd = GetFreeLimitedSoundChannel();
										if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_blockhit );
									}
								} else {
									if( blockArray[0][xi].active ) {
										// play block touch sound	
										s8 tmpsnd;
										tmpsnd = GetFreeLimitedSoundChannel();
										if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_blockhit );
									}
								}

								resetCHARBLOCK( &topBlockArray[yi][xi] );
							} else {
								PA_SetSpriteY( 1, topBlockArray[yi][xi].spriteNum, (u8)( topBlockArray[yi][xi].y + topBlockArray[yi][xi].yoffset ) );
							}
						}
					}
				}
				
				
				
				
				
				
				
				
				
				
				// bottom row special case
				
				/************************************************************************************************** 
				*
				*	1. check if empty in first row on bottom screen
				*	2. move block out of screen
				*	3. when block is out of screen swap all info with first block in row on bottom screen
				*	4. delete the sprite on top screen
				*	5. offset the block on bottom screen to -32
				*	6. let the block on bottom screen fall into place
				*
				***************************************************************************************************/
				
				yi = GRIDH-1;
				for( xi = 0; xi<GRIDW; xi++ ) {
					if( !topBlockArray[yi][xi].active ) continue;
					if( blockArray[0][xi].active ) continue;
					// ok, there is a block on top screen and an empty space on bottom screen
					topBlockArray[yi][xi].velocity = MIN( 8, topBlockArray[yi][xi].velocity + 1 );
					topBlockArray[yi][xi].yoffset += topBlockArray[yi][xi].velocity;
					if( topBlockArray[yi][xi].yoffset >= 32 ) {
						// if block has moved a whole step (32 pix), swap all block attributes in grid and reset current block						
						PA_DeleteSprite( 1, topBlockArray[yi][xi].spriteNum );
						blockArray[0][xi].character = topBlockArray[yi][xi].character;
						blockArray[0][xi].yoffset = -32;
						PA_CreateSprite( 0, blockArray[0][xi].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[0][xi].x, blockArray[0][xi].y + blockArray[0][xi].yoffset );			
						PA_SetSpriteAnim( 0, blockArray[0][xi].spriteNum, blockArray[0][xi].character );
						
						// let the text background print on top of sprites
						PA_SetSpritePrio( 0, blockArray[0][xi].spriteNum, 1 );
												
						blockArray[0][xi].active = true;
						blockArray[0][xi].busy = true;
						blockArray[0][xi].velocity = 0;				

						// resetCHARBLOCK( &blockArray[yi+1][xi] );						
						resetCHARBLOCK( &topBlockArray[yi][xi] );
					} else {
						PA_SetSpriteY( 1, topBlockArray[yi][xi].spriteNum, (u8)( topBlockArray[yi][xi].y + topBlockArray[yi][xi].yoffset ) );
					}		
				}
		
		
		
		
		

		
		
				// check if it's time for new block to enter game?
				if( Pad.Newpress.L || Pad.Newpress.R ) {
					if( !levelClearFlag ) {
						playerAddedBlockCounter++;
						if( playerAddedBlockCounter >= 10 ) {
							addBomb = true;
							playerAddedBlockCounter = 0;
						}
						addNewBlock( topBlockArray, blockArray );
					}
					blockTimer = getNewBlockTime();					
				} else {
					if( blockTimer <= 0 ) {
						if( !levelClearFlag ) addNewBlock( topBlockArray, blockArray );
						blockTimer = getNewBlockTime();
					} else {
						blockTimer--;        
					}
				}
		
		
		
		
		
		
				// bounce all columns that are supposed to bounce
				if( bounceCounter < ARRAY_LEN( bounceArray )-1 ) {
					bounceCounter++;
				} else {
					bounceCounter = 0;
				}
				


				
				for( xi=0; xi < GRIDW; xi++ ) {
					if( columnsBouncing[ xi ] ) {
						for( yi = 0; yi < GRIDH; yi++ ) {
							PA_SetSpriteY( 1, topBlockArray[yi][xi].spriteNum, (u8)topBlockArray[yi][xi].y + bounceArray[ bounceCounter ] );
							PA_SetSpriteY( 0, blockArray[yi][xi].spriteNum, (u8)blockArray[yi][xi].y + bounceArray[ bounceCounter ] );
						}
						
						if( warningTabs[xi].active ) {

							if( warningTabs[xi].counter > 0 ) {

								if( ( warningTabs[xi].counter / 60 ) + 1 < 10 ){ 
									PA_OutputText( 0, warningTabs[xi].textColumn, 23, "0%d", ( warningTabs[xi].counter / 60 ) + 1 );
								} else {
									PA_OutputText( 0, warningTabs[xi].textColumn, 23, "%d", ( warningTabs[xi].counter / 60 ) + 1 );
								}
								warningTabs[xi].counter--;

							} else {

								// time has run out, game over
								displayScore = score;
								gameState = GAME_OVER;
								gameOver = true;
								
							}

						} else {
							// move sprite and then turn on the counter
							if( !gameOver ) {
								if( warningTabs[xi].movementCounter >  0 ) {
									warningTabs[xi].movementCounter--;
									warningTabs[xi].offset = WARNING_TAB_MOVEMENT[ warningTabs[xi].movementCounter ];
									PA_SetSpriteY( 0, warningTabs[xi].spriteNum, warningTabs[xi].ypos + warningTabs[xi].offset );
								} else {
									warningTabs[xi].active = true;
								}
							}
						}
						
					}
				}














		
				// scroll background
				scrollSky( &skyScroll );

		
//displayTopInfo( &nSelected, currentWord );		
				
				
				
				if( showScoreFlag ) {
					if( showScoreDisplay( &scoreDisplayCounter, &scoreDisplayFade ) ) {
						clearScoreDisplay( scoreSpriteArray, &showScoreFlag, &scoreDisplayMotionCounter );
						// after showing score, level is clear!
						if( levelClearFlag ) {
						        gameState = LEVEL_CLEAR;
						        gameSpeedTimer = 0;
						}
					} else {
						scrollScoreDisplay( scoreSpriteArray, &SCORE_DISPLAY_OFFSET, &scoreDisplayMotionCounter, &scoreYPos );
					}
				}

		
				if(!levelClearFlag) {
					if( gameSpeedTimer > 0 ) {
						gameSpeedTimer--;
					} else {


						if( gameSpeed > 1 ) {
							gameSpeed = gameSpeed - 1;
						}
	
						if( gameState == ARCADE ) {
							gameSpeedTimer = MAX( gameClockSpeed - level * 5, 10 );
						} else {
							gameSpeedTimer = gameClockSpeed;				
						}


					}
				} 


				if( Pad.Newpress.Select ) {
        				//PA_SetBrightness( 0, -32 );
					//PA_SetBrightness( 1, -5 );
					PA_OutputText( 1, 4, 9, "      GAME PAUSED" );
					PA_OutputText( 1, 4, 11, "PRESS %c1START%c0 TO QUIT GAME" );
					PA_OutputText( 1, 4, 13, " PRESS %c1SELECT%c0 TO RESUME" );
					PA_SetModVolume( 20 );
					screenFade = 0;
					gameState = PAUSE_FADE;
				}

	
				break;
			}

			
			case PAUSE_FADE: {
					if( screenFade > -32 ) {
					        screenFade--;
						PA_SetBrightness( 0, screenFade );
						PA_SetBrightness( 1, MAX( screenFade, -5 ) );
					} else {
						gameState = PAUSE;
					}
		        		break;
				}



			case PAUSE: {
				if( Pad.Newpress.Select ) {
        				PA_SetBrightness( 0, 0 );
					PA_SetBrightness( 1, 0 );
					PA_OutputText( 1, 4, 9, "                               " );
					PA_OutputText( 1, 4, 11, "                               " );
					PA_OutputText( 1, 4, 13, "                               " );
					PA_SetModVolume( 90 );
					
					gameState = RUN_GAME;
				}     


				if( Pad.Newpress.Start ) {
        				PA_SetBrightness( 0, -32 );
					PA_SetBrightness( 1, -32 );

					PA_StopMod();

					gameState = QUIT_GAME;
				}     				
				   			
        			break;
			}

			case QUIT_GAME: {
				PA_ClearTextBg( 1 );
				PA_ClearTextBg( 0 );

				// delete all blocksprites
				for( yi=0; yi < GRIDH; yi++ ) {
					for( xi=0; xi < GRIDW; xi++ ) {
						if( topBlockArray[yi][xi].active ) {
							PA_DeleteSprite( 1, topBlockArray[yi][xi].spriteNum );				
							resetCHARBLOCK( &topBlockArray[yi][xi] );
						}
						if( blockArray[yi][xi].active ) {
							PA_DeleteSprite( 0, blockArray[yi][xi].spriteNum );
							resetCHARBLOCK( &blockArray[yi][xi] );
						}
					}
				}

				gameOver = true;


				clearScoreDisplay( scoreSpriteArray, &showScoreFlag, &scoreDisplayMotionCounter );
				deleteWarningTabs( warningTabs );
			        hideCursor( &cursor );
			        
				// clear game backgrounds
				PA_DeleteBg( 1, 3 );
				PA_DeleteBg( 1, 2 );
				// text bg
				PA_DeleteBg( 1, 0 );
				// sky bg
				PA_DeleteBg( 0, 3 );
				// text bg
				PA_DeleteBg( 0, 0 );

				// clear background system
				PA_ResetBgSys();
				
				// reset any bouncing columns
				bounceCounter = 0;
				for( i=0; i < ARRAY_LEN( columnsBouncing ); i++ ) columnsBouncing[ i ] = false;

				if( nSelected > 0 ) {
				        clearSelection( p_selectedBlocksArray, nSelected );
					nSelected = 0;
				}
				
				// delete all the score sprites
				for( i = 0; i < 8; i++ ) {
				        PA_DeleteSprite( 0, scoreSpriteArray[i] );
				}
				
				playerAddedBlockCounter = 0;
				
				score = 0;
				displayScore = 0;
				highestScore = 0;
				longestWord = 0;
				memset( longestWordStr, '\0', sizeof( longestWordStr ) );
				charsCollected = 0;				
				longestLevelWord = 0;
				memset( longestLevelWordStr, '\0', sizeof( longestLevelWordStr ) );
				
				for( i=0; i < numberOfWords; i++ ) {
					memset( allWordsArray[i], '\0', sizeof( allWordsArray[i] ));
				}
				
				numberOfWords = 0;
				numberOfLevelWords = 0;
				
			        musicIsPlaying = false;

				PA_SetBrightness( 0, 0 );
				PA_SetBrightness( 1, 0 );

				nextState = MENU;
				stateTimer = 60;
				gameState = WAIT_FOR_NEXT_STATE;

   				break;
			}




			case LEVEL_CLEAR: {
				PA_ClearTextBg( 1 );
				PA_ClearTextBg( 0 );


				// play level clear sound	
				s8 tmpsnd;
				tmpsnd = GetFreeLimitedSoundChannel();
				if( tmpsnd > -1 && voiceEffects ) PA_PlaySimpleSound( (u8)tmpsnd, voice_levelclear );
				
				displayTopInfo( &nSelected, currentWord );

			        clearScoreDisplay( scoreSpriteArray, &showScoreFlag, &scoreDisplayMotionCounter );
				deleteWarningTabs( warningTabs );
				hideCursor( &cursor );					

				gameState = LEVEL_CLEAR_ANIMATION;
				scrollSky( &skyScroll );
				break;
			}



			
			case LEVEL_CLEAR_ANIMATION: {

//				PA_ClearTextBg( 1 );
				PA_ClearTextBg( 0 );

				gameSpeedTimer++;
				if( levelClearFlag && ( gameSpeedTimer % 10 ) < 5 ) PA_OutputText( 0, 10, 12, "LEVEL CLEAR!" );

				bool allOutOfBoundsFlag = true;

				// move all existing sprites out of screen, as if they were falling
				for( yi=0; yi < GRIDH; yi++ ) {
					for( xi=0; xi < GRIDW; xi++ ) {
						if( topBlockArray[yi][xi].active ) {
							if( topBlockArray[yi][xi].x + topBlockArray[yi][xi].xFloatOffset > -64 && topBlockArray[yi][xi].x + topBlockArray[yi][xi].xFloatOffset < 256+64 ) {
								topBlockArray[yi][xi].xFloatOffset = topBlockArray[yi][xi].xFloatOffset + topBlockArray[yi][xi].xSlideSpeed;
								PA_SetSpriteX( 1, topBlockArray[yi][xi].spriteNum, (s16)(topBlockArray[yi][xi].x + topBlockArray[yi][xi].xFloatOffset) );
								allOutOfBoundsFlag = false;
							}
						}

						if( blockArray[yi][xi].active ) {						
						        if( blockArray[yi][xi].x + blockArray[yi][xi].xFloatOffset > -64 && blockArray[yi][xi].x + blockArray[yi][xi].xFloatOffset < 256 + 64 ) {															
								blockArray[yi][xi].xFloatOffset = blockArray[yi][xi].xFloatOffset + blockArray[yi][xi].xSlideSpeed;
								PA_SetSpriteX( 0, blockArray[yi][xi].spriteNum, (s16)(blockArray[yi][xi].x + blockArray[yi][xi].xFloatOffset) );								
								allOutOfBoundsFlag = false;
							}
						}						
					}
				}

				if( allOutOfBoundsFlag && gameSpeedTimer > 240 ) {
					// delete all blocksprites
					for( yi=0; yi < GRIDH; yi++ ) {
						for( xi=0; xi < GRIDW; xi++ ) {
							if( topBlockArray[yi][xi].active ) {
								PA_DeleteSprite( 1, topBlockArray[yi][xi].spriteNum );				
								resetCHARBLOCK( &topBlockArray[yi][xi] );
							}
							if( blockArray[yi][xi].active ) {
								PA_DeleteSprite( 0, blockArray[yi][xi].spriteNum );
								resetCHARBLOCK( &blockArray[yi][xi] );
							}
						}
					}



					gameState = LEVEL_CLEAR_FADE_OUT;

				}
				scrollSky( &skyScroll );
				break;
			}




			case LEVEL_CLEAR_FADE_OUT: {

				PA_ClearTextBg( 0 );

				if( screenFade > -32 ) {
				        screenFade--;
					PA_SetBrightness( 0, screenFade );
					PA_SetBrightness( 1, screenFade );

					gameSpeedTimer++;
					if( levelClearFlag && ( gameSpeedTimer % 10 ) < 5 ) PA_OutputText( 0, 10, 12, "LEVEL CLEAR!" );

				} else {

					// fade down music

					if( fadeOutMod( 30 ) ) {

						longestWordBonus = getLongestWordBonus();
						timeBonus = getTimeBonus();
						charsBonus = getCharsBonus();
						
						// print current score
						PA_OutputText( 1, 4, 7,  "LONG WORD BONUS...%d", longestWordBonus );
						PA_OutputText( 1, 4, 9,  "CHARS BONUS.......%d", charsBonus );
						PA_OutputText( 1, 4, 11, "SPEED BONUS.......%d", timeBonus );
						PA_OutputText( 1, 4, 13, "SCORE.............%d", score );
						
	
						
						gameSpeedTimer = 0;				
	
						gameState = LEVEL_CLEAR_FADE_IN;
					}
										
				}
				scrollSky( &skyScroll );
				break;
			}

		
		
			case LEVEL_CLEAR_FADE_IN: {

				if( screenFade < 0 ) {
				        screenFade++;
					PA_SetBrightness( 0, screenFade );
					PA_SetBrightness( 1, screenFade );
				} else {
					PA_SetBrightness( 0, 0 );
					PA_SetBrightness( 1, 0 );
					gameState = WAIT_FOR_NEXT_STATE_SCROLL;
					nextState = LEVEL_SCORE;
					stateTimer = 30;

					// music
//					PA_PlayMod( music_break );
					
				}
				scrollSky( &skyScroll );
				break;
			}		
		
		
			case LEVEL_SCORE: {
				
				// clear all text
				PA_ClearTextBg( 1 );
				
				if( longestWordBonus > 0 ) {
					if( longestWordBonus > 9 ) {
						longestWordBonus = longestWordBonus - 10;
						score = score + 10;
						
						if( !PA_SoundChannelIsBusy( 7 ) ) PA_PlaySimpleSound( 7, sound_blockhit );
	
						if( longestWordBonus == 0 ) {
							gameState = WAIT_FOR_NEXT_STATE_SCROLL;
							nextState = LEVEL_SCORE;
							stateTimer = 30;
						}
					} else {
						score = score + longestWordBonus;
						longestWordBonus = 0;
						gameState = WAIT_FOR_NEXT_STATE_SCROLL;
						nextState = LEVEL_SCORE;
						stateTimer = 30;
					}
				} else if( charsBonus > 0 ) {
					if( charsBonus > 9 ) {
						charsBonus = charsBonus - 10;
						score = score + 10;

						if( !PA_SoundChannelIsBusy( 7 ) ) PA_PlaySimpleSound( 7, sound_blockhit );

						if( charsBonus == 0 ) {
							gameState = WAIT_FOR_NEXT_STATE_SCROLL;
							nextState = LEVEL_SCORE;
							stateTimer = 30;
						}
					} else {
						score = charsBonus;
						charsBonus = 0;
						gameState = WAIT_FOR_NEXT_STATE_SCROLL;
						nextState = LEVEL_SCORE;
						stateTimer = 30;
					}
				} else if( timeBonus > 0 ) {
					if( timeBonus > 9 ) {
						timeBonus = timeBonus - 10;
						score = score + 10;

						if( !PA_SoundChannelIsBusy( 7 ) ) PA_PlaySimpleSound( 7, sound_blockhit );

						if( timeBonus == 0 ) {
							gameState = WAIT_FOR_NEXT_STATE_SCROLL;
							nextState = NEXT_LEVEL_GOAL;
							stateTimer = 180;
						}
						
					} else {
						score = timeBonus;
						timeBonus = 0;
						gameState = WAIT_FOR_NEXT_STATE_SCROLL;
						nextState = NEXT_LEVEL_GOAL;
						stateTimer = 180;
					}
				}
				
				displayScore = score;
				displayTopInfo( &nSelected, currentWord );
				
				// print current score
				PA_OutputText( 1, 4, 7,  "LONG WORD BONUS...%d", longestWordBonus );
				PA_OutputText( 1, 4, 9,  "CHARS BONUS.......%d", charsBonus );
				PA_OutputText( 1, 4, 11, "SPEED BONUS.......%d", timeBonus );
				PA_OutputText( 1, 4, 13, "SCORE.............%d", score );
				
				scrollSky( &skyScroll );
				break;
			}
		
		
		
			case NEXT_LEVEL_GOAL: {

				if( !fadeUpMod( 90 ) ) break;

				// clear all text
				PA_ClearTextBg( 1 );
				PA_ClearTextBg( 0 );
				level++;
				if( level < maxLevels ) {
//					levelGoal = levelGoals[level];
					gameState = RESET_GAME;
				} else {
					PA_OutputText( 1, 8, 10, "CONGRATULATIONS!", level );
					PA_OutputText( 1,11, 12, "GAME CLEAR", level );
					nextState = GAME_CLEAR;
					stateTimer = 180;
					gameState = WAIT_FOR_NEXT_STATE_SCROLL;
				}
				
				break;
			}
			
			
			
			case GAME_CLEAR: {
				PA_ClearTextBg( 1 );
				PA_ClearTextBg( 0 );
				PA_OutputText( 1, 4, 7,  "SCORE.............%d", score );
				PA_OutputText( 1, 4, 9,  "LONGEST WORD......%s", longestWordStr );
				PA_OutputText( 1, 4, 11, "BEST SCORE........%d", highestScore );
				PA_OutputText( 1, 4, 13, "WORDS FOUND.......%d", numberOfWords );
				PA_OutputText( 1, 4, 15, "MOST USED WORD....%s", allWordsArray[ getMostUsedWord( numberOfWords, allWordsArray ) ] );
				PA_OutputText( 1, 4, 16, "(USED %d TIMES)", getMostUsedWordCount( numberOfWords, allWordsArray ) );
			PA_StopMod();
				nextState = INIT_PRINT_NAME;
				stateTimer = 600;
				gameState = WAIT_FOR_NEXT_STATE_SCROLL;
				break;
			}

			
				
			case GAME_OVER: {
				PA_ClearTextBg( 0 );
			        
				clearScoreDisplay( scoreSpriteArray, &showScoreFlag, &scoreDisplayMotionCounter );
				deleteWarningTabs( warningTabs );
			        hideCursor( &cursor );
			        
			        // play game over sound:
				s8 tmpsnd;
				tmpsnd = GetFreeLimitedSoundChannel();
				if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_gameover );
				
				PA_StopMod();
				
				gameState = GAME_OVER_ANIMATION;
				break;
			}
		
		
			
			case GAME_OVER_ANIMATION: {
				bool allOutOfBoundsFlag = true;
				// move all existing sprites out of screen, as if they were falling
				for( yi=0; yi < GRIDH; yi++ ) {
					for( xi=0; xi < GRIDW; xi++ ) {
						if( topBlockArray[yi][xi].active ) {
						        if( topBlockArray[yi][xi].x + topBlockArray[yi][xi].xFloatOffset > -64 && topBlockArray[yi][xi].x + topBlockArray[yi][xi].xFloatOffset < 256+64 ) {
								if( topBlockArray[yi][xi].y + topBlockArray[yi][xi].yFloatOffset > 0-32 ) {
	
									topBlockArray[yi][xi].xFloatOffset = topBlockArray[yi][xi].xFloatOffset + topBlockArray[yi][xi].xMomentum;
									topBlockArray[yi][xi].yFloatOffset = topBlockArray[yi][xi].yFloatOffset + topBlockArray[yi][xi].yMomentum;
	
									PA_SetSpriteXY( 1, topBlockArray[yi][xi].spriteNum, topBlockArray[yi][xi].x + topBlockArray[yi][xi].xFloatOffset, topBlockArray[yi][xi].y + topBlockArray[yi][xi].yFloatOffset );
									// PA_SetSpriteXY( 0, blockArray[yi][xi].spriteNum, blockArray[yi][xi].x, blockArray[yi][xi].y );
									
									// modify friction and gravity
									topBlockArray[yi][xi].xMomentum = topBlockArray[yi][xi].xMomentum * blockFriction;
									topBlockArray[yi][xi].yMomentum = topBlockArray[yi][xi].yMomentum - blockGravity;
									// debugVal = topBlockArray[yi][xi].yMomentum;
									allOutOfBoundsFlag = false;
								}
							}
						}

						if( blockArray[yi][xi].active ) {						
						        if( blockArray[yi][xi].x + blockArray[yi][xi].xFloatOffset > -64 && blockArray[yi][xi].x + blockArray[yi][xi].xFloatOffset < 256+64 ) {
								if( blockArray[yi][xi].y + blockArray[yi][xi].yFloatOffset < 192 ) {
	
									blockArray[yi][xi].xFloatOffset = blockArray[yi][xi].xFloatOffset + blockArray[yi][xi].xMomentum;
									blockArray[yi][xi].yFloatOffset = blockArray[yi][xi].yFloatOffset + blockArray[yi][xi].yMomentum;
	
									PA_SetSpriteXY( 0, blockArray[yi][xi].spriteNum, blockArray[yi][xi].x + blockArray[yi][xi].xFloatOffset, blockArray[yi][xi].y + blockArray[yi][xi].yFloatOffset );
									// PA_SetSpriteXY( 0, blockArray[yi][xi].spriteNum, blockArray[yi][xi].x, blockArray[yi][xi].y );
									
									// modify friction and gravity
									blockArray[yi][xi].xMomentum = blockArray[yi][xi].xMomentum * blockFriction;
									blockArray[yi][xi].yMomentum = blockArray[yi][xi].yMomentum + blockGravity;
									// debugVal = blockArray[yi][xi].yMomentum;
									allOutOfBoundsFlag = false;
								}
							}
						}						
					}
				}
				
				if( allOutOfBoundsFlag ) {
					// all sprites are out of screen, delete them all
					for( yi=0; yi < GRIDH; yi++ ) {
						for( xi=0; xi < GRIDW; xi++ ) {
							if( topBlockArray[yi][xi].active ) { 
							        PA_DeleteSprite( 1, topBlockArray[yi][xi].spriteNum );
								resetCHARBLOCK( &topBlockArray[yi][xi] );
							}							
							if( blockArray[yi][xi].active ) {
							        PA_DeleteSprite( 0, blockArray[yi][xi].spriteNum );							
								resetCHARBLOCK( &blockArray[yi][xi] );
							}
						

						}
					}
					
					// G
					PA_CreateSprite( 0, blockArray[2][2].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[2][2].x, blockArray[2][2].y );
					PA_SetSpriteAnim( 0, blockArray[2][2].spriteNum, 6 );
					
					// A
					PA_CreateSprite( 0, blockArray[2][3].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[2][3].x, blockArray[2][3].y );
					PA_SetSpriteAnim( 0, blockArray[2][3].spriteNum, 0 );

					// M
					PA_CreateSprite( 0, blockArray[2][4].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[2][4].x, blockArray[2][4].y );
					PA_SetSpriteAnim( 0, blockArray[2][4].spriteNum, 12 );

					// E
					PA_CreateSprite( 0, blockArray[2][5].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[2][5].x, blockArray[2][5].y );
					PA_SetSpriteAnim( 0, blockArray[2][5].spriteNum, 4 );

					// O
					PA_CreateSprite( 0, blockArray[3][2].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[3][2].x, blockArray[3][2].y );
					PA_SetSpriteAnim( 0, blockArray[3][2].spriteNum, 14 );

					// V
					PA_CreateSprite( 0, blockArray[3][3].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[3][3].x, blockArray[3][3].y );
					PA_SetSpriteAnim( 0, blockArray[3][3].spriteNum, 21 );

					// E
					PA_CreateSprite( 0, blockArray[3][4].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[3][4].x, blockArray[3][4].y );
					PA_SetSpriteAnim( 0, blockArray[3][4].spriteNum, 4 );

					// R
					PA_CreateSprite( 0, blockArray[3][5].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[3][5].x, blockArray[3][5].y );
					PA_SetSpriteAnim( 0, blockArray[3][5].spriteNum, 17 );

					PA_OutputText( 1, 4, 7,  "SCORE.............%d", score );
					PA_OutputText( 1, 4, 9,  "LONGEST WORD......%s", longestWordStr );
					PA_OutputText( 1, 4, 11, "BEST SCORE........%d", highestScore );
					PA_OutputText( 1, 4, 13, "WORDS FOUND.......%d", numberOfWords );
					PA_OutputText( 1, 4, 15, "MOST USED WORD....%s", allWordsArray[ getMostUsedWord( numberOfWords, allWordsArray ) ] );
					PA_OutputText( 1, 4, 16, "(USED %d TIMES)", getMostUsedWordCount( numberOfWords, allWordsArray ) );

					gameState = GAME_OVER_WAIT;


					PA_PlayMod( music_break );
					balanceSound();
					PA_SetModVolume( 127 );
						
				}
				scrollSky( &skyScroll );
				break;
			}  
			
			
			      
			case GAME_OVER_WAIT: {
			 					
			        //PA_UpdateStylus();
				if( Stylus.Held ) {
					// delete the sprites used for printing GAME OVER on bottom screen
					PA_DeleteSprite( 0, blockArray[2][2].spriteNum );					
					PA_DeleteSprite( 0, blockArray[2][3].spriteNum );
					PA_DeleteSprite( 0, blockArray[2][4].spriteNum );
					PA_DeleteSprite( 0, blockArray[2][5].spriteNum );
					PA_DeleteSprite( 0, blockArray[3][2].spriteNum );
					PA_DeleteSprite( 0, blockArray[3][3].spriteNum );
					PA_DeleteSprite( 0, blockArray[3][4].spriteNum );
					PA_DeleteSprite( 0, blockArray[3][5].spriteNum );
				        

					
					PA_ClearTextBg( 1 );
					
					if( award_a || award_b || award_c ) {
						gameState = AWARD_C;
						anyCounter = 0;
					} else {
						// play thank you sound	
						s8 tmpsnd;
						tmpsnd = GetFreeLimitedSoundChannel();
						if( tmpsnd > -1 && voiceEffects ) PA_PlaySimpleSound( (u8)tmpsnd, voice_thankyouforplaying );

						gameState = INIT_PRINT_NAME;
					}
	
				}
				scrollSky( &skyScroll );
				break;
			}
			
			
			
			
			
			
			
			
			case AWARD_C: {

				PA_ClearTextBg( 1 );

				scrollSky( &skyScroll );

				if(!award_c) {
					gameState = AWARD_B;
					break;
				}
				
				if( anyCounter == 0 ) {

					PA_CreateSprite( 	1, awardTopSprites[0], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 128 - 32, 96 - 32 );
					PA_SetSpriteAnim( 	1, awardTopSprites[0], 47 );
					PA_SetSpriteDblsize( 	1, awardTopSprites[0],  1 );
					// scaling does not work on emu's!
					PA_SetSpriteRotEnable( 	1, awardTopSprites[0], 31 );

				}

				if( anyCounter == 21 ) {
					// play award sound	
					s8 tmpsnd;
					tmpsnd = GetFreeLimitedSoundChannel();
					if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_award );
				}

				if( anyCounter > 27 ) {
					PA_OutputText( 1, 0,  5,  "        CONGRATULATIONS!        " );
					PA_OutputText( 1, 0, 18,  "   THE C AWARD FOR COMPLETING   " );
					PA_OutputText( 1, 0, 20,  "       A SEVEN LETTER WORD!     " );
				}
				
				// 	 ###			
				// 512 - 384 = 128;
				//       ###				

				u16 tmpZoom = 0;
				if( anyCounter < ARRAY_LEN( smoothMotionArrayWithBounce ) ) {
					
					tmpZoom = (u16)( 128.0 + ( 384.0 * smoothMotionArrayWithBounce[anyCounter] ) );

					// run animation
					PA_SetRotsetNoAngle(	
						1, 		// screen
						31, 		// rotset
						tmpZoom,	// vertical zoom
						tmpZoom		// horizontal zoom
					);

				}

				anyCounter++;				

				// wait for time out
				if( Stylus.Newpress || anyCounter > 500 ) {
					PA_SetSpriteDblsize( 	1, awardTopSprites[0],  0 );
					PA_SetSpriteRotDisable( 1, awardTopSprites[0] );
					PA_DeleteSprite( 1, awardTopSprites[0] );
					anyCounter = 0;
					gameState = AWARD_B;
				}
				

				break;
			}			

				
			case AWARD_B: {

				PA_ClearTextBg( 1 );

				scrollSky( &skyScroll );

				if(!award_b) {
					gameState = AWARD_A;
					break;
				}
				
				if( anyCounter == 0 ) {

					PA_CreateSprite( 	1, awardTopSprites[1], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 128 - 32, 96 - 32 );
					PA_SetSpriteAnim( 	1, awardTopSprites[1], 48 );
					PA_SetSpriteDblsize( 	1, awardTopSprites[1],  1 );
					// scaling does not work on emu's!
					PA_SetSpriteRotEnable( 	1, awardTopSprites[1], 31 );

				}

				if( anyCounter == 21 ) {
					// play award sound	
					s8 tmpsnd;
					tmpsnd = GetFreeLimitedSoundChannel();
					if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_award );
				}

				if( anyCounter > 27 ) {
					PA_OutputText( 1, 0,  5,  "        CONGRATULATIONS!        " );
					PA_OutputText( 1, 0, 18,  "   THE B AWARD FOR COMPLETING   " );
					PA_OutputText( 1, 0, 20,  "           100 WORDS!           " );
				}
				
				// 	 ###			
				// 512 - 384 = 128;
				//       ###				

				u16 tmpZoom = 0;
				if( anyCounter < ARRAY_LEN( smoothMotionArrayWithBounce ) ) {
					
					tmpZoom = (u16)( 128.0 + ( 384.0 * smoothMotionArrayWithBounce[anyCounter] ) );

					// run animation
					PA_SetRotsetNoAngle(	
						1, 		// screen
						31, 		// rotset
						tmpZoom,	// vertical zoom
						tmpZoom		// horizontal zoom
					);

				}

				anyCounter++;				

				// wait for time out
				if( Stylus.Newpress || anyCounter > 500 ) {
					PA_SetSpriteDblsize( 	1, awardTopSprites[1],  0 );
					PA_SetSpriteRotDisable( 1, awardTopSprites[1] );
					PA_DeleteSprite( 1, awardTopSprites[1] );
					anyCounter = 0;
					gameState = AWARD_A;
				}
				

				break;

			}	
			
			case AWARD_A: {

				PA_ClearTextBg( 1 );

				scrollSky( &skyScroll );

				if(!award_a) {
				
					// play thank you sound	
					s8 tmpsnd;
					tmpsnd = GetFreeLimitedSoundChannel();
					if( tmpsnd > -1 && voiceEffects ) PA_PlaySimpleSound( (u8)tmpsnd, voice_thankyouforplaying );
				
					gameState = INIT_PRINT_NAME;
					break;
				}
				
				if( anyCounter == 0 ) {

					PA_CreateSprite( 	1, awardTopSprites[2], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 128 - 32, 96 - 32 );
					PA_SetSpriteAnim( 	1, awardTopSprites[2], 49 );
					PA_SetSpriteDblsize( 	1, awardTopSprites[2],  1 );
					// scaling does not work on emu's!
					PA_SetSpriteRotEnable( 	1, awardTopSprites[2], 31 );

				}

				if( anyCounter == 21 ) {
					// play award sound	
					s8 tmpsnd;
					tmpsnd = GetFreeLimitedSoundChannel();
					if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_award );
				}

				if( anyCounter > 27 ) {
					PA_OutputText( 1, 0,  5,  "         YOU ARE GREAT!         " );
					PA_OutputText( 1, 0, 18,  "   THE A AWARD FOR COMPLETING   " );
					PA_OutputText( 1, 0, 20,  "           200 WORDS!           " );
				}
				
				// 	 ###			
				// 512 - 384 = 128;
				//       ###				

				u16 tmpZoom = 0;
				if( anyCounter < ARRAY_LEN( smoothMotionArrayWithBounce ) ) {
					
					tmpZoom = (u16)( 128.0 + ( 384.0 * smoothMotionArrayWithBounce[anyCounter] ) );

					// run animation
					PA_SetRotsetNoAngle(	
						1, 		// screen
						31, 		// rotset
						tmpZoom,	// vertical zoom
						tmpZoom		// horizontal zoom
					);

				}

				anyCounter++;				

				// wait for time out
				if( Stylus.Newpress || anyCounter > 500 ) {
					PA_SetSpriteDblsize( 	1, awardTopSprites[2],  0 );
					PA_SetSpriteRotDisable( 1, awardTopSprites[2] );
					PA_DeleteSprite( 1, awardTopSprites[2] );
					anyCounter = 0;

					// play thank you sound	
					s8 tmpsnd;
					tmpsnd = GetFreeLimitedSoundChannel();
					if( tmpsnd > -1 && voiceEffects ) PA_PlaySimpleSound( (u8)tmpsnd, voice_thankyouforplaying );
					
					gameState = INIT_PRINT_NAME;	

				}
				

				break;

			}				
			
			

/*


	// Activate rotations for that sprite
	PA_SetSpriteRotEnable(0,// screen
						0,// sprite number
						0);// rotset number. You have 32 rotsets (0-31) per screen. 2 sprites with the same rotset will
							// be zoomed/rotated the same way...
				PA_SetSpriteRotEnable(0, 1, 0); // Same rotset as the other sprite... it'll be zoomed the same way
	
				u16 zoom = 256; // Zoom. 256 means no zoom, 512 is twice as small, 128 is twice as big....
	
				// Fast function for zoom without rotations...
				PA_SetRotsetNoAngle(	0, 		//screen
					0, 		// rotset
					zoom, zoom	// Horizontal zoom, vertical zoom. You can have a sprite streched out if you want,
				);

				// top sprites
				PA_CreateSprite(  1, awardTopSprites[0], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );
				PA_SetSpriteAnim( 1, awardTopSprites[0], 49 );
				PA_CreateSprite(  1, awardTopSprites[1], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );
				PA_SetSpriteAnim( 1, awardTopSprites[1], 48 );
				PA_CreateSprite(  1, awardTopSprites[2], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );
				PA_SetSpriteAnim( 1, awardTopSprites[2], 47 );

				// delete award sprites top
				PA_DeleteSprite( 1, awardTopSprites[0] );
				PA_DeleteSprite( 1, awardTopSprites[1] );
				PA_DeleteSprite( 1, awardTopSprites[2] );			
*/			
			
			
			
			
			case INIT_PRINT_NAME: {
				//PA_UpdateStylus();
				// wait for stylus release ...
				if( Stylus.Held ) break;
				
				memset( currentNameString, '\0', sizeof( currentNameString ) );
				currentNameStringLen = 0;
				
				pressed = true;
				p_currentBlock = NULL;

				char tmpChr = 0;
				for( yi=1; yi < 4; yi++ ) {
					for( xi=0; xi < GRIDW; xi++ ) {				
						blockArray[yi][xi].character = tmpChr;
						PA_CreateSprite( 0, blockArray[yi][xi].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[yi][xi].x, blockArray[yi][xi].y );
						PA_SetSpriteAnim( 0, blockArray[yi][xi].spriteNum, blockArray[yi][xi].character );						
						PA_SetSpritePrio( 0, blockArray[yi][xi].spriteNum, 1 );
						tmpChr++;
					}
				}

				
				// y
				blockArray[4][2].character = 24;
				PA_CreateSprite( 0, blockArray[4][2].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[4][2].x, blockArray[4][2].y );
				PA_SetSpriteAnim( 0, blockArray[4][2].spriteNum, blockArray[4][2].character );	
				PA_SetSpritePrio( 0, blockArray[4][2].spriteNum, 1 );
				
				// z
				blockArray[4][3].character = 25;
				PA_CreateSprite( 0, blockArray[4][3].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[4][3].x, blockArray[4][3].y );
				PA_SetSpriteAnim( 0, blockArray[4][3].spriteNum, blockArray[4][3].character );										
				PA_SetSpritePrio( 0, blockArray[4][3].spriteNum, 1 );				

				// bomb block
				blockArray[4][4].character = 26;
				PA_CreateSprite( 0, blockArray[4][4].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[4][4].x, blockArray[4][4].y );
				PA_SetSpriteAnim( 0, blockArray[4][4].spriteNum, blockArray[4][4].character );										
				PA_SetSpritePrio( 0, blockArray[4][4].spriteNum, 1 );

				// ok! block
				blockArray[4][5].character = 44;
				PA_CreateSprite( 0, blockArray[4][5].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, blockArray[4][5].x, blockArray[4][5].y );
				PA_SetSpriteAnim( 0, blockArray[4][5].spriteNum, blockArray[4][5].character );										
				PA_SetSpritePrio( 0, blockArray[4][5].spriteNum, 1 );
				
				// name blocks on topscreen
				yi = 2;
				for ( xi=2; xi < 6; xi++ ) {
					topBlockArray[yi][xi].character = 45;
					PA_CreateSprite( 1, topBlockArray[yi][xi].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, topBlockArray[yi][xi].x, topBlockArray[yi][xi].y );
					PA_SetSpriteAnim( 1, topBlockArray[yi][xi].spriteNum, topBlockArray[yi][xi].character );
					topBlockArray[yi][xi].explode = false;
				}
				
				scrollSky( &skyScroll );
				gameState = PRINT_NAME;
				
				break;
				
			}




			case PRINT_NAME:  {
			
				//PA_UpdateStylus();
				// clear all text
				PA_ClearTextBg( 1 );
				
				if( Stylus.Held ) {

					if(!pressed) {
						// get pointer to the currently touched block, else NULL
						p_currentBlock = getPointerToTouchedCharBlock_noBoundCheck( blockArray );
	
						// if a block was selected, place and show cursor
						if( p_currentBlock ) {
							cursor.xpos = (u8)(*p_currentBlock).x;
							cursor.ypos = (u8)(*p_currentBlock).y;
							PA_SetSpriteXY( 0, cursor.spriteNum, cursor.xpos, cursor.ypos );
							// set sprite mode of the current block to alpha blending
							PA_SetSpriteMode( 0, (*p_currentBlock).spriteNum, 1 ); // screen, sprite, alphablending
						} else {
							hideCursor( &cursor );
						}

						// reset pressed to true
						pressed = true;
					}
					stylusIsOnCursor = PA_SpriteTouched( cursor.spriteNum );
				} else { // Stylus is released!

					if( pressed ) {
						// is the stylus still in focus on the character
						if( stylusIsOnCursor ) {
						
							if( (*p_currentBlock).character == STAR ) {
							
								if( currentNameStringLen > 0 ) {
									// delete character!
									currentNameString[currentNameStringLen-1] = '\0';
									yi = 2;
									xi = 2 + currentNameStringLen-1;
									topBlockArray[yi][xi].explode = true;
									topBlockArray[yi][xi].explosionCounter = EXPLOSION_FRAMES;
									
									// play block explosion sound	
									s8 tmpsnd;
									tmpsnd = GetFreeLimitedSoundChannel();
									if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_bomb );
									
									topBlockArray[yi][xi].character = 45;
									currentNameStringLen--;
								}
							
							} else if ( (*p_currentBlock).character == 44 ) { // ok! use this name ...
							
								// if no name is typed, set name to "none"
								if( currentNameStringLen == 0 ) {
									strcpy( currentNameString, "NONE" );
									currentNameStringLen = 4;
									topBlockArray[2][2].character = 'N' - 'A';
									topBlockArray[2][3].character = 'O' - 'A';
									topBlockArray[2][4].character = 'N' - 'A';
									topBlockArray[2][5].character = 'E' - 'A';
									PA_SetSpriteAnim( 1, topBlockArray[2][2].spriteNum, topBlockArray[2][2].character );
									PA_SetSpriteAnim( 1, topBlockArray[2][3].spriteNum, topBlockArray[2][3].character );
									PA_SetSpriteAnim( 1, topBlockArray[2][4].spriteNum, topBlockArray[2][4].character );
									PA_SetSpriteAnim( 1, topBlockArray[2][5].spriteNum, topBlockArray[2][5].character );
								}
								
								// delete the keyboard
								for( yi=1; yi < 4; yi++ ) {
									for( xi=0; xi < GRIDW; xi++ ) {				
										PA_DeleteSprite( 0, blockArray[yi][xi].spriteNum );
										resetCHARBLOCK( &blockArray[yi][xi] );
									}
								}
								
								// delete the odd final keys of the keyboard
								// y
								PA_DeleteSprite( 0, blockArray[4][2].spriteNum );
								resetCHARBLOCK( &blockArray[4][2] );
								
								// z
								PA_DeleteSprite( 0, blockArray[4][3].spriteNum );
								resetCHARBLOCK( &blockArray[4][3] );
								
								// bomb block
								PA_DeleteSprite( 0, blockArray[4][4].spriteNum );
								resetCHARBLOCK( &blockArray[4][4] );

								// ok! block
								PA_DeleteSprite( 0, blockArray[4][5].spriteNum );
								resetCHARBLOCK( &blockArray[4][5] );


								// set alphablending to blocks on topscreen
								yi = 2;
								for ( xi=2; xi < 6; xi++ ) {
									if( topBlockArray[yi][xi].explode ) {
										PA_SetSpriteAnim( 1, topBlockArray[yi][xi].spriteNum, topBlockArray[yi][xi].character );
										topBlockArray[yi][xi].explode = false;
									}
									PA_SetSpriteMode( 1, topBlockArray[yi][xi].spriteNum, 1 ); // screen, sprite, alphablending
								}


								
								if( currentNameStringLen < 4 ) {
									PA_SetSpriteXY( 1, topBlockArray[2][5].spriteNum, -32, -32 );
									resetCHARBLOCK( &topBlockArray[2][5] );
								}
								if( currentNameStringLen < 3 ) {
									PA_SetSpriteXY( 1, topBlockArray[2][4].spriteNum, -32, -32 );
									resetCHARBLOCK( &topBlockArray[2][4] );
								}
								if( currentNameStringLen < 2 ) {
									PA_SetSpriteXY( 1, topBlockArray[2][3].spriteNum, -32, -32 );
									resetCHARBLOCK( &topBlockArray[2][3] );
								}

								// set and save hiscore
								setHiscore();

								// top sprites
								PA_CreateSprite(  1, awardTopSprites[0], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );
								PA_CreateSprite(  1, awardTopSprites[1], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );
								PA_CreateSprite(  1, awardTopSprites[2], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );

								PA_SetSpriteAnim( 1, awardTopSprites[0], 49 );
								PA_SetSpriteAnim( 1, awardTopSprites[1], 48 );
								PA_SetSpriteAnim( 1, awardTopSprites[2], 47 );

								// bottom sprites
								PA_CreateSprite(  0, awardBottomSprites[0], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );
								PA_CreateSprite(  0, awardBottomSprites[1], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );
								PA_CreateSprite(  0, awardBottomSprites[2], (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, 256, 192 );

								PA_SetSpriteAnim( 0, awardBottomSprites[0], 49 );
								PA_SetSpriteAnim( 0, awardBottomSprites[1], 48 );
								PA_SetSpriteAnim( 0, awardBottomSprites[2], 47 );

								if( award_a ) {
									PA_SetSpriteXY( 1, awardTopSprites[0], 80, 144 );	
								}

								if( award_b ) {
									PA_SetSpriteXY( 1, awardTopSprites[1], 112, 144 );
								}
								
								if( award_c ) {
									PA_SetSpriteXY( 1, awardTopSprites[2], 144, 144 );
								}

								// awardTopSprites			
								// awardBottomSprites
								
								// prepare for scrolling hiscore names								
								initScrollHiscore( blockArray );
								
								gameState = SET_NAME;

							} else if( currentNameStringLen < MAX_NAME_LENGTH ) { // add new character to name
						
								buildName( p_currentBlock );
								yi = 2;
								xi = 2 + currentNameStringLen;
								topBlockArray[yi][xi].character = (*p_currentBlock).character;
								PA_SetSpriteAnim( 1, topBlockArray[yi][xi].spriteNum, topBlockArray[yi][xi].character );
								currentNameStringLen++;
						
							}
						}
						// reset and hide current selection
						PA_SetSpriteMode( 0, (*p_currentBlock).spriteNum, 0 ); // screen, sprite, alphablending
						hideCursor( &cursor );
						p_currentBlock = NULL;
						stylusIsOnCursor = false;	
						pressed = false;
					}

				}
				
				// animate explosion for deleted characters
				yi = 2;
				for ( xi=2; xi < 6; xi++ ) {
					if( topBlockArray[yi][xi].explode ) {
						if( topBlockArray[yi][xi].explosionCounter > 0 ) {
							topBlockArray[yi][xi].explosionCounter--;
							PA_SetSpriteAnim( 1, topBlockArray[yi][xi].spriteNum, explosionAnimationArray[ topBlockArray[yi][xi].explosionCounter ] );
						} else {
							PA_SetSpriteAnim( 1, topBlockArray[yi][xi].spriteNum, topBlockArray[yi][xi].character );
							topBlockArray[yi][xi].explode = false;
						}
					}
				}
				
				
				// calculate new alpha
				alpha = getAlpha( alpha );
				// set alpha
				PA_SetSFXAlpha(1, alpha, 15); // screen, alpha value (0-15), set to 15
				

				// PA_OutputText( 1, 9, 17,  "%s", currentNameString );

				PA_OutputText( 1, 5, 14,  "PLEASE TYPE YOUR NAME!" );


				// anyCounter++;
				// gameState = RESET_GAME;
				scrollSky( &skyScroll );				
				break;
			}
			
			
			
			case SET_NAME: {

				// clear all text
				PA_ClearTextBg( 1 );
				
				// name blocks on topscreen

				// move name to center
				yi = 2;
				u8 tmp = 2 + currentNameStringLen;
				for ( xi=2; xi < tmp; xi++ ) {
					if(  PA_GetSpriteX( 1, topBlockArray[yi][xi].spriteNum ) < ( topBlockArray[yi][xi].x + ( 4 - currentNameStringLen ) * 16 )  ) {
						PA_SetSpriteX( 1, topBlockArray[yi][xi].spriteNum, PA_GetSpriteX( 1, topBlockArray[yi][xi].spriteNum ) + 4 ); // topBlockArray[yi][xi].xFloatOffset );
					}
				}

				PA_OutputText( 1, 11, 14,  "SCORE %d", score );
				
				// calculate new alpha
				alpha = getAlpha( alpha );
				// set alpha
				PA_SetSFXAlpha(1, alpha, 15); // screen, alpha value (0-15), set to 15				

				// show hiscore names one by one on the bottom screen
				scrollHiscore( blockArray, awardBottomSprites );
				
				//PA_UpdateStylus();
								
				if( Stylus.Held ) {
					if( !pressed ) {
						// delete and clear name sprites on top screen
						yi = 2;
						for ( xi=2; xi < 6; xi++ ) {
							PA_SetSpriteMode( 1, topBlockArray[yi][xi].spriteNum, 0 ); // screen, sprite, alphablending
							PA_DeleteSprite( 1, topBlockArray[yi][xi].spriteNum );
							resetCHARBLOCK( &topBlockArray[yi][xi] );	
						}				
						// clear hiscore scroller
						clearScrollHiscore( blockArray );
						// text bg
						PA_DeleteBg( 0, 0 );
						gameState = RESET_GAME;
					}
				} else {
					if( pressed ) pressed = false;
				}
				
				scrollSky( &skyScroll );				
				break;
			}
			
			
			
			case SAVE_SCORE: {
				break;
			}
			
			
			
			case SHOW_HISCORES: {
				break;
			}
						
			
			case RESET_GAME: { // reset game before returning to menu
			        
				// clear game backgrounds
				PA_DeleteBg( 1, 3 );
				PA_DeleteBg( 1, 2 );
				// text bg
				PA_DeleteBg( 1, 0 );
				// sky bg
				PA_DeleteBg( 0, 3 );
				// text bg
				PA_DeleteBg( 0, 0 );

				// clear background system
				PA_ResetBgSys();
				
				// reset any bouncing columns
				bounceCounter = 0;
				for( i=0; i < ARRAY_LEN( columnsBouncing ); i++ ) columnsBouncing[ i ] = false;

				if( nSelected > 0 ) {
				        clearSelection( p_selectedBlocksArray, nSelected );
					nSelected = 0;
				}
				
				// delete all the score sprites
				for( i = 0; i < 8; i++ ) {
				        PA_DeleteSprite( 0, scoreSpriteArray[i] );
				}
				
				playerAddedBlockCounter = 0;
				
				if( gameOver ) {
	
					// delete award sprites top
					PA_DeleteSprite( 1, awardTopSprites[0] );
					PA_DeleteSprite( 1, awardTopSprites[1] );
					PA_DeleteSprite( 1, awardTopSprites[2] );
	
					// delete award sprites bottom
					PA_DeleteSprite( 0, awardBottomSprites[0] );
					PA_DeleteSprite( 0, awardBottomSprites[1] );
					PA_DeleteSprite( 0, awardBottomSprites[2] );
	
					// reset the awards
					award_a = false;
					award_b = false;
					award_c = false;

					score = 0;
					displayScore = 0;
					highestScore = 0;
					longestWord = 0;
					memset( longestWordStr, '\0', sizeof( longestWordStr ) );
				} else {
					displayScore = score;
					charsCollected = 0;
				}
											
				longestLevelWord = 0;
				memset( longestLevelWordStr, '\0', sizeof( longestLevelWordStr ) );
				
				if( gameOver ) {
					for( i=0; i < numberOfWords; i++ ) {
						memset( allWordsArray[i], '\0', sizeof( allWordsArray[i] ));
					}
					numberOfWords = 0;
				}
				
				numberOfLevelWords = 0;
				
				if( gameOver ) {
				        PA_StopMod();
				        musicIsPlaying = false;
					nextState = MENU;
					stateTimer = 60;
					gameState = WAIT_FOR_NEXT_STATE;
				} else {
					gameState = INIT_GAME;
				}
				
				break;
			}
			
			
			
			case WAIT_FOR_NEXT_STATE_SCROLL: {
				//PA_UpdateStylus();

				// scroll background
				scrollSky( &skyScroll );
/*				
				if( Stylus.Held ) {
					gameState = nextState;
				}
*/
				if( stateTimer > 0 ) {
					stateTimer--;
				} else {
					gameState = nextState;
				}
				
				break;				
			}
			
			
			
			case WAIT_FOR_NEXT_STATE: {
				
				//PA_UpdateStylus();
/*
				if( Stylus.Held ) {
					gameState = nextState;
				}
*/
				if( stateTimer > 0 ) {
					stateTimer--;
				} else {
					gameState = nextState;
				}
				
				break;
			}
			
		
		} // end of main case switch




/*
		// press 'up' to activate slow mode
		if( Pad.Held.Up ) {
		        u32 tmp = 10;
			while( tmp > 0 ) {
				tmp--;
				PA_WaitForVBL();
			}
		}
*/



		if( Pad.Held.Up ) {
			if( gameSpeedTimer % 2 ) PA_OutputText( 0, 1, 1, "DEBUG MODE" );			
			if( gameState == RUN_GAME ) {
		        	for( i=0; i<6; i++ ){

					char tmps[0];
					tmps[0] = (char)(blockArray[i][6].character + 'A');

					if( blockArray[i][6].active ) {
						PA_OutputText( 1, 1, 3+i, "           , actve [x], [%s]", tmps );
					} else {
						PA_OutputText( 1, 1, 3+i, "           , actve [ ], [ ]" );
					}

					if( blockArray[i][6].busy ) {
						PA_OutputText( 1, 1, 3+i, "%d busy [x]", i);
					} else {
						PA_OutputText( 1, 1, 3+i, "%d busy [ ]", i);
					}

				}

				PA_OutputText( 1, 1, 11, "pos [%d, %d]", PA_GetSpriteX(0, blockArray[1][6].spriteNum ), PA_GetSpriteY(0, blockArray[1][6].spriteNum ) );				
				PA_OutputText( 1, 1, 12, "velocity [%d]", blockArray[1][6].velocity );
				PA_OutputText( 1, 1, 13, "yoffset [%d]", blockArray[1][6].yoffset );

			        u32 tmp = 10;
				while( tmp > 0 ) {
					tmp--;
					PA_WaitForVBL();
				}

			}
		}






		// pause if lid is closed
		if( PA_CheckLid() ) {
			// play let's play! sound when lid is opened
			s8 tmpsnd;
			tmpsnd = GetFreeLimitedSoundChannel();
			if( tmpsnd > -1 && voiceEffects ) PA_PlaySimpleSound( (u8)tmpsnd, voice_letsplay );
		}



		// ### wait for screen to redraw ###
		PA_WaitForVBL();

		
	} // end of main while
	
	return 0;
} // End of main()



bool hasAward() {
	u8 i;
	for( i=0; i<10; i++ ) {
 		if( hiscoreAwards_a[i] ) return true;
 		if( hiscoreAwards_b[i] ) return true;
 		if( hiscoreAwards_c[i] ) return true;
	}	
	return false;
}

void lockSpriteCreate( u8 *lockSpriteNum ) {
	/* CREATE AND DISPLAY THE LOCK SPRITE */
	// load palette
	PA_LoadSpritePal( 0, 2, (void*)tutorial_gfx_Pal );	
	// create sprite
	PA_CreateSprite( 0, *lockSpriteNum, (void*)tutorial_gfx_Sprite, OBJ_SIZE_32X32, 1, 2, 256, 192 );
	// set frame
	PA_SetSpriteAnim( 0, *lockSpriteNum, 3);        
}

void lockSpriteHide( u8 *lockSpriteNum ) {
	PA_SetSpriteXY( 0, *lockSpriteNum, 256, 192 );
}

void lockSpriteShow( u8 *lockSpriteNum ) {
	if( !hasAward() ) PA_SetSpriteXY( 0, *lockSpriteNum, 84, 78 );
}

void lockSpriteDelete( u8 *lockSpriteNum ) {
	PA_DeleteSprite( 0, *lockSpriteNum );
}



void balanceSound( ) {      

	PA_SetModChanPan(0, 127);

	PA_SetModChanPan(1, 0);

	PA_SetModChanPan(2, 0);

	PA_SetModChanPan(3, 127);

}




bool fadeUpMod( u8 vol ) {
	if( PA_GetModVolume() < vol ) {
		PA_SetModVolume( PA_GetModVolume() + 1 );
	} else {
		PA_SetModVolume( vol );
		return true;
	}
	return false;
}





bool fadeOutMod( u8 vol ) {
	if( PA_GetModVolume() > vol ) {
		PA_SetModVolume( PA_GetModVolume() - 1 );
	} else {
		PA_SetModVolume( vol );
		return true;
	}
	return false;
}











// use only channel 4 to 7 for sound effects
s8 GetFreeLimitedSoundChannel() {

/*
	if( !PA_SoundChannelIsBusy(  4 ) ) return  4;
	if( !PA_SoundChannelIsBusy(  5 ) ) return  5;
	if( !PA_SoundChannelIsBusy(  6 ) ) return  6;
	if( !PA_SoundChannelIsBusy(  7 ) ) return  7;
*/
	
	if( !PA_SoundChannelIsBusy( 12 ) ) return  12;
	if( !PA_SoundChannelIsBusy( 13 ) ) return  13;
	if( !PA_SoundChannelIsBusy( 14 ) ) return  14;
	if( !PA_SoundChannelIsBusy( 15 ) ) return  15;

	return -1;
}








void deleteWarningTabs( WARNING_TAB warningTabs[8] ) {
	u8 i;
	for(i=0; i<8; i++) {
		warningTabs[i].movementCounter = ARRAY_LEN( WARNING_TAB_MOVEMENT )-1;
		warningTabs[i].offset = WARNING_TAB_MOVEMENT[ warningTabs[i].movementCounter ];
		// PA_SetSpriteY( 0, warningTabs[i].spriteNum, warningTabs[i].ypos + warningTabs[i].offset );
		warningTabs[i].counter = 600;
		warningTabs[i].active = false;
		// deleting warningtabs, make sure they are also deleted when level is clear
		PA_DeleteSprite( 0, warningTabs[i].spriteNum );
	}
}









// does file exist? ->
// tryEnableSave -> 



bool saveExist() {
	// returns 0 if file exists
	// returns -1 if error
	FILE *file;
	file = fopen( fileName, "rb" );
	if( file == NULL ) { 
		return false;
	} else {
		fclose( file );
		return true;
	}
}

bool createSave() {
	// returns 0 if file exists
	// returns -1 if error
	FILE *file;
	file = fopen( fileName, "wb" );
	if( file == NULL ) {
		return false;
	} else {
		fclose( file );
		return true;
	}
}

bool tryEnableSave() {
	// if fatlib already is active, return true
	if( saveEnabled ) return true;
	// try to init fat lib
	if( !fatInitDefault() ) return false;
	// saving is now enabled!	
	saveEnabled = true;
	// check if file already exist, if not create a new file
	if( !saveExist() ) {
		createSave();
		// save default hiscore list
		saveHiscore();
	} else {
		loadHiscore();
	}
	// all is good
	return true;
}

bool saveHiscore() {
	if( !saveEnabled ) return false;

	FILE *file;
	file = fopen( fileName, "wb" );

	if( file == NULL ) return false; // open file failed!

	u8 i;	
	for( i=0; i<ARRAY_LEN( hiscoreArray ); i++ ) {
		fwrite( &(hiscoreArray[i]), sizeof( hiscoreArray[0] ), 1, file );
		// string is already a pointer, no need to cast
		fwrite( hiscoreNameArray[i], sizeof( char ), 4+1, file );
		
		fwrite( &(hiscoreWordCount[i]), sizeof( hiscoreWordCount[0] ), 1, file );
		fwrite( hiscoreLongWord[i], sizeof( char ), 7+1, file );
		fwrite( &(hiscoreAwards_a[i]), sizeof( bool ), 1, file );
		fwrite( &(hiscoreAwards_b[i]), sizeof( bool ), 1, file );
		fwrite( &(hiscoreAwards_c[i]), sizeof( bool ), 1, file );		
		
		// ... hiscoreWordCount
		// ... hiscoreLongWord
		// ... hiscoreAwards_a
		// ... hiscoreAwards_b
		// ... hiscoreAwards_c
		
	}
	strcpy( debugStr, "saved" );
	fclose( file );
	return true;
}



bool loadHiscore() {
	if( !saveEnabled ) return false;

	FILE *file;
	file = fopen( fileName, "rb" );
	if( file == NULL ) return false; // open file failed!

	// tmp buffers
//	u16 tmpVal[10];
//	char tmpStr[10][4+1];

	u8 i;

	for(i=0; i<ARRAY_LEN( hiscoreArray ); i++) {
		if( feof(file) == 0 ) {
			fread( &(hiscoreArray[i]), sizeof( hiscoreArray[0] ), 1, file );
			// string! no need to cast to pointer
			fread( hiscoreNameArray[i], sizeof( char ), 4+1, file );

			fread( &(hiscoreWordCount[i]), sizeof( hiscoreWordCount[0] ), 1, file );
			fread( hiscoreLongWord[i], sizeof( char ), 7+1, file );
			fread( &(hiscoreAwards_a[i]), sizeof( bool ), 1, file );
			fread( &(hiscoreAwards_b[i]), sizeof( bool ), 1, file );
			fread( &(hiscoreAwards_c[i]), sizeof( bool ), 1, file );
					
			// ... hiscoreWordCount
			// ... hiscoreLongWord
			// ... hiscoreAwards_a
			// ... hiscoreAwards_b
			// ... hiscoreAwards_c

		}
	}

	strcpy( debugStr, "loaded" );			

	// finished reading
	fclose( file );

	return true;
}















void printOptions() {
	
	PA_ClearTextBg( 0 );
// 	%c7 = light gray, %c8 = dark gray

	u8 tmpCol = 10;

		PA_OutputText( 0, tmpCol, 6,   "RANDOM TYPE:" );


	if( randomAlgorithm == SIMPLE ) {
		PA_OutputText( 0, tmpCol, 7,   "   UNBALANCED" );
	} else {
		PA_OutputText( 0, tmpCol, 7,   "   %c8UNBALANCED" );					
	}
	
	if( randomAlgorithm == SUPERPANIC ) {
		PA_OutputText( 0, tmpCol, 8,   "   BALANCED" );
	} else {
		PA_OutputText( 0, tmpCol, 8,   "   %c8BALANCED" );					
	}
/*	
	if( randomAlgorithm == JAYENKAI ) {
		PA_OutputText( 0, tmpCol, 9,   "   JAYENKAI" );
	} else {
		PA_OutputText( 0, tmpCol, 9,   "   %c8JAYENKAI" );						
	}
*/
	if( voiceEffects ) {
		PA_OutputText( 0, tmpCol, 10,  "VOICE ON" );
	} else {
		PA_OutputText( 0, tmpCol, 10,  "%c8VOICE OFF" );
	}

	if( saveEnabled ) {
		PA_OutputText( 0, tmpCol, 12,  "SAVING ON" );
	} else {
		PA_OutputText( 0, tmpCol, 12,  "%c8SAVING OFF" );
	}

	PA_OutputText( 0, tmpCol, 14,  "DIFFICULTY" );
	switch( difficulty ) {
		case HARD:
			PA_OutputText( 0, tmpCol, 15,  "   HARD" );
			PA_OutputText( 0, tmpCol, 16,  "   %c8MEDIUM" );
			PA_OutputText( 0, tmpCol, 17,  "   %c8EASY" );
			break;
		case MEDIUM:
			PA_OutputText( 0, tmpCol, 15,  "   %c8HARD" );
			PA_OutputText( 0, tmpCol, 16,  "   MEDIUM" );
			PA_OutputText( 0, tmpCol, 17,  "   %c8EASY" );
			break;
		case EASY:
			PA_OutputText( 0, tmpCol, 15,  "   %c8HARD" );
			PA_OutputText( 0, tmpCol, 16,  "   %c8MEDIUM" );
			PA_OutputText( 0, tmpCol, 17,  "   EASY" );
			break;
	}

	PA_OutputText( 0, tmpCol, 20,  "BACK" );

}










bool moveStylus( s16 tx, s16 ty, SPRITE_WRAPPER *p_stylus ) {
	if ( tx == (*p_stylus).x && ty == (*p_stylus).y ) {
		// target has NOT changed
		if( !targetReached ) {
			if( (*p_stylus).counter < ARRAY_LEN( smoothMotionArray ) ) {				
				PA_SetSpriteX( 1, (*p_stylus).spriteNum, (*p_stylus).x + (s16)( (*p_stylus).floatXOffset * smoothMotionArray[ (*p_stylus).counter ] ) );
				PA_SetSpriteY( 1, (*p_stylus).spriteNum, (*p_stylus).y + (s16)( (*p_stylus).floatYOffset * smoothMotionArray[ (*p_stylus).counter ] ) );
				(*p_stylus).counter++;
			} else {
				(*p_stylus).floatXOffset = 0;
				(*p_stylus).floatYOffset = 0;
				(*p_stylus).counter = 0;
				targetReached = true;
			}
		} else {
			// target reached, return true
			return true;
		}
	} else {
		// target has changed, prepare for movement
		targetReached = false;
		(*p_stylus).counter = 0;

		(*p_stylus).floatXOffset = (double)( (*p_stylus).x - tx ); // difference between current real x pos and tx;
		(*p_stylus).floatYOffset = (double)( (*p_stylus).y - ty ); // difference between current real y pos and ty;

		(*p_stylus).x = tx;	// goal x
		(*p_stylus).y = ty;	// goal y

// debug:
// PA_OutputText( 1, 2, 2, "%d %d %d",  PA_GetSpriteX( 1, (*p_stylus).spriteNum ), tx, PA_GetSpriteX( 1, (*p_stylus).spriteNum ) - tx );
// PA_WaitFor( Stylus.Newpress );


	};
	// stylus is moving, return false
	return false;
}







// check if current score is high enough to be put on the top ten list
s8 setHiscore( void ) {

	// are score among the top ten?

	if( score < hiscoreArray[9] ) return -1;

	s8 i;

	// count backwards from 9 to -1

	for( i=9; i > -1; i-- ) {

		// if score is same or higher than current hiscore copy hiscore down.
		// AND if we are not at the bottom of the list (top score).

		if( score >= hiscoreArray[i] && i > 0 ) {

			// copy next score to current position
			hiscoreArray[i] = hiscoreArray[i-1];
			strcpy( hiscoreNameArray[i], hiscoreNameArray[i-1] );
			
			hiscoreWordCount[i] = hiscoreWordCount[i-1];
			strcpy( hiscoreLongWord[i], hiscoreLongWord[i-1] ); 

			hiscoreAwards_a[i] = hiscoreAwards_a[i-1];
			hiscoreAwards_b[i] = hiscoreAwards_b[i-1];
			hiscoreAwards_c[i] = hiscoreAwards_c[i-1];			

/*
			hiscoreAwards_a[i]
			hiscoreAwards_b[i]
			hiscoreAwards_c[i]
*/			
			
			continue;

		} else {

			// we are at the bottom of the list OR score is less than current position
			if( score < hiscoreArray[i] && i < 9 ) {
				hiscoreArray[i+1] = score;
				strcpy( hiscoreNameArray[i+1], currentNameString );
	
				hiscoreWordCount[i+1] = numberOfWords;
				strcpy( hiscoreLongWord[i+1], longestWordStr );
				
				hiscoreAwards_a[i+1] = award_a;
				hiscoreAwards_b[i+1] = award_b;
				hiscoreAwards_c[i+1] = award_c;
				
			} else {
				hiscoreArray[i] = score;
				strcpy( hiscoreNameArray[i], currentNameString );
	
				hiscoreWordCount[i] = numberOfWords;
				strcpy( hiscoreLongWord[i], longestWordStr );

				hiscoreAwards_a[i] = award_a;
				hiscoreAwards_b[i] = award_b;
				hiscoreAwards_c[i] = award_c;

			}			
			// save hiscore!
			if( saveEnabled ) saveHiscore();
			
			return i;

		}

	}

	return -1;	

}









// prepares for animating hiscore names on lower screen
void initScrollHiscore( CHARBLOCK blockArray[GRIDH][GRIDW] ) {

	PA_InitText(0,0);	// init the default text functions (screen, background)
	
	PA_SetTextCol(		// set the text columns white
		0,		// screen
		31,		// Red
		31,		// Green
		31		// Blue
	);

	u8 yi;
	u8 xi;

	// set alphablending to blocks on topscreen
	yi = 2;
	for ( xi=2; xi < 6; xi++ ) {
		// place the sprites outside of screen
		blockArray[yi][xi].motionCounter = 0;
		blockArray[yi][xi].yFloatOffset = ( (float)SCREEN_HEIGHT - (float)blockArray[yi][xi].y );
		PA_CreateSprite( 0, blockArray[yi][xi].spriteNum, (void*)wordblocks_Sprite, OBJ_SIZE_32X32, 1, 0, (s16)blockArray[yi][xi].x  + ( 4 - (s16)strlen( hiscoreNameArray[0] ) ) * 16 , (s16)blockArray[yi][xi].y + (s16)blockArray[yi][xi].yFloatOffset );

		if( xi < strlen( hiscoreNameArray[ hiscoreShowCounter ] ) + 2 ) {
				blockArray[yi][xi].character = hiscoreNameArray[0][xi-2] - 'A';
				PA_SetSpriteAnim( 0, blockArray[yi][xi].spriteNum, blockArray[yi][xi].character );
		}
		
		blockArray[yi][xi].rotDelay = 10 * (xi-2);
// 		PA_SetSpriteXY(0, blockArray[yi][xi].spriteNum, blockArray[yi][xi].x, blockArray[yi][xi].y + blockArray[yi][xi].yFloatOffset );
	}

	hiscoreShowCounter = 0;
	showHiscoreDelayCounter = 120;
}


// continues to animate in hiscore names one by one on lower screen
void scrollHiscore( CHARBLOCK blockArray[GRIDH][GRIDW], u8 awardBottomSprites[3] ) {
	
	// clear all text on lower screen
	PA_ClearTextBg( 0 );

	// check if the last hiscore is shown
	// if( hiscoreShowCounter < ARRAY_LEN( hiscoreArray ) ) {
	// go throught all the blocks of the name
	
	u8 yi;
	u8 xi;
	
	yi = 2;
	
	bool ready = true;
	
	for ( xi=2; xi < strlen( hiscoreNameArray[ hiscoreShowCounter ] )+2; xi++ ) {
		// wait for delay
		if( blockArray[yi][xi].rotDelay > 0 ) {
			blockArray[yi][xi].rotDelay--;
			continue;
		}
		// move
		if( blockArray[yi][xi].motionCounter < ARRAY_LEN( hiscoreMotionArray ) ) {
			PA_SetSpriteAnim( 0, blockArray[yi][xi].spriteNum, blockArray[yi][xi].character );
			PA_SetSpriteY( 0, blockArray[yi][xi].spriteNum, (s16)blockArray[yi][xi].y + (s16)( blockArray[yi][xi].yFloatOffset * hiscoreMotionArray[ blockArray[yi][xi].motionCounter ] ) );
			blockArray[yi][xi].motionCounter++;
			ready = false;
		}
	}
	
	
	
	if( ready ) {

		if( --showHiscoreDelayCounter > 0 ) {

			PA_OutputText( 0, 6, 6,  "     POSITION %d", hiscoreShowCounter + 1 );

			PA_OutputText( 0, 4, 14, "SCORE.............%d", hiscoreArray[ hiscoreShowCounter ] );

			PA_OutputText( 0, 4, 16, "WORDS.............%d", hiscoreWordCount[ hiscoreShowCounter ] );

			PA_OutputText( 0, 4, 18, "LONGEST WORD......%s", hiscoreLongWord[ hiscoreShowCounter ] );


			// show the award medals
			if( hiscoreAwards_a[ hiscoreShowCounter ] ) {
				PA_SetSpriteXY( 0, awardBottomSprites[0], 80, 156 );	
			} 

			if( hiscoreAwards_b[ hiscoreShowCounter ] ) {
				PA_SetSpriteXY( 0, awardBottomSprites[1], 112, 156 );
			} 
			
			if( hiscoreAwards_c[ hiscoreShowCounter ] ) {
				PA_SetSpriteXY( 0, awardBottomSprites[2], 144, 156 );
			}


/*
	hiscoreAwards_a[i] = award_a;
	hiscoreAwards_b[i] = award_b;
	hiscoreAwards_c[i] = award_c;

	// ... hiscoreWordCount
	// ... hiscoreLongWord
	// ... hiscoreAwards_a
	// ... hiscoreAwards_b
	// ... hiscoreAwards_c
*/




		} else {	
			
	
			if( hiscoreShowCounter < ARRAY_LEN( hiscoreArray )-1 ) {
				hiscoreShowCounter++;
			} else {
				hiscoreShowCounter = 0;
			}
			// reset and start with next name
			for ( xi=2; xi < 6; xi++ ) {
				if( xi < strlen( hiscoreNameArray[ hiscoreShowCounter ] )+2 ) {
					blockArray[yi][xi].motionCounter = 0;
					PA_SetSpriteY( 0, blockArray[yi][xi].spriteNum, (s16)blockArray[yi][xi].y + (s16)( blockArray[yi][xi].yFloatOffset * hiscoreMotionArray[ blockArray[yi][xi].motionCounter ] ) );
					PA_SetSpriteX( 0, blockArray[yi][xi].spriteNum, (s16)blockArray[yi][xi].x + ( 4 - strlen( hiscoreNameArray[ hiscoreShowCounter ] ) ) * 16 );
					blockArray[yi][xi].character = hiscoreNameArray[hiscoreShowCounter][xi-2] - 'A';
					blockArray[yi][xi].rotDelay = 10*(xi-2);
					showHiscoreDelayCounter = 120;
				} else {
					// blockArray[yi][xi].motionCounter = ARRAY_LEN( hiscoreMotionArray );
					PA_SetSpriteY( 0, blockArray[yi][xi].spriteNum, (s16)blockArray[yi][xi].y + (s16)( blockArray[yi][xi].yFloatOffset ) );
				}
			}

			// hide the award medals
			PA_SetSpriteXY( 0, awardBottomSprites[0], 256, 192 );
			PA_SetSpriteXY( 0, awardBottomSprites[1], 256, 192 );
			PA_SetSpriteXY( 0, awardBottomSprites[2], 256, 192 );						
			
		}
	
	}

}



void clearScrollHiscore( CHARBLOCK blockArray[GRIDH][GRIDW] ) {
	u8 yi;
	u8 xi;
	yi = 2;
	
	// delete text background
	PA_DeleteBg( 0, 0 );
	
	for ( xi=2; xi < 6; xi++ ) {
		PA_DeleteSprite( 0, blockArray[yi][xi].spriteNum );
		resetCHARBLOCK( &(blockArray[yi][xi]) );
	}
}































































void displayTopInfo( u8 *nSelected, char currentWord[8] ) {

	if( displayScore+10 < score ) {
	        displayScore = displayScore +10;
	} else {
		displayScore = score;			
	}

	PA_OutputText( 1, 1, 1,  "SCORE" );
	PA_OutputText( 1, 1, 2,  "%d", displayScore );

	if( gameStyle == ARCADE ) {	
		PA_OutputText( 1, 8, 1, "GOAL" );
		PA_OutputText( 1, 8, 2, "%d", levelGoal - numberOfLevelWords );
	} else {
		PA_OutputText( 1, 8, 1, "FOUND" );
		PA_OutputText( 1, 8, 2, "%d", numberOfLevelWords );	
	}

	PA_OutputText( 1, 16, 1, "WORD" );
	if( *nSelected ) {
		if( starSelected( currentWord ) ) {
			if( gameSpeedTimer%10 < 5 ) PA_OutputText( 1, 16, 2, "EXPLODE!" );
		} else {
			if( gameSpeedTimer%10 < 5 ) PA_OutputText( 1, 16, 2, "%s", currentWord );
		}
	}
	// PA_OutputText( 1, 14, 2, "%s", levelGoal - numberOfLevelWords );

	PA_OutputText( 1, 26, 1, "SPEED" );
	PA_OutputText( 1, 26, 2, "%d", 100-gameSpeed );


}





u32 getLongestWordBonus( ) {
	return (u32)strlen( longestLevelWordStr ) * 100;
}

u32 getTimeBonus( ) {
	return gameSpeed * 10;
}

u32 getCharsBonus( ) {
	return charsCollected * 10;
}











void scrollSky( float *skyScroll ) {
	(*skyScroll) = (*skyScroll) + 0.1;
	PA_BGScrollX( 1, 3, (u32)(*skyScroll) );
	PA_BGScrollX( 1, 2, (u32)((*skyScroll) * 2) );
}



void addScore( char s[8], u8 n ) {
	u8 i=0;
	u32 tmp = BASE_VALUE * n;
	while( s[i] != '\0' ) {
		// tmp = tmp + BASE_VALUE;
		tmp = tmp * SCORE_ARRAY[ (u8)(s[i]-'A') ];  
		i++;
	}
	score = score + tmp;
}


u32 getScore( char s[8], u8 n ) {
	u8 i=0;
	u32 tmp = BASE_VALUE * n;
	while( s[i] != '\0' ) {
		// tmp = tmp + BASE_VALUE;
		tmp = tmp * SCORE_ARRAY[ (u8)(s[i]-'A') ];  
		i++;
	}
	return tmp;
}

void addAndDisplayWordScore( char s[8], u8 n, u8 arr[8], bool *showScoreFlag, s16 xpos, s16 ypos, s16 *scoreYPos ) {

	u32 tmp = BASE_VALUE * n;
	u8 i=0;
	
	while( s[i] != '\0' ) {
		tmp = tmp + ( 10 * (u8)( s[i]-'A' ) );
		i++;
	}

	*scoreYPos = ypos-8;

	if( tmp < 100000000 ) {
		u8 strLen;
		char scoreString[8];

		if( gameStyle == ARCADE ) {
		        // display number of words left
			strLen = sprintf( scoreString, "%d", levelGoal - numberOfLevelWords );
		} else {
		        // display score
			strLen = sprintf( scoreString, "%d", tmp );
		}
		i=0;
		while( scoreString[i] != '\0' ) {
			// PA_SetSpriteXY( 0, arr[ i ], ( xpos + 16 ) - 6*( strLen-1 ) + i*12 -8, ypos-8 );
			
			if( xpos == 224 ) { 
				PA_SetSpriteX( 0, arr[ i ], ( xpos + 16 ) - 6*( strLen-1 ) + i*12 -8 -12 );
			} else if( xpos == 0 ) {
				PA_SetSpriteX( 0, arr[ i ], ( xpos + 16 ) - 6*( strLen-1 ) + i*12 -8 +12 );
			} else {
				PA_SetSpriteX( 0, arr[ i ], ( xpos + 16 ) - 6*( strLen-1 ) + i*12 -8 );			
			}
			
			PA_SetSpriteY( 0, arr[ i ], ypos-8 );
			
			PA_SetSpriteAnim( 0, arr[ i ], scoreString[i] - '0' + 30 );
			PA_SetSpriteMode( 0, arr[ i ], 1 ); // screen, sprite, alphablending
			i++;
		}
	}
	// alpha
	// save highest score
	if( tmp > highestScore ) highestScore = tmp;	
	// score is global
	*showScoreFlag = true;
	score = score + tmp;
}

void clearScoreDisplay( u8 arr[8], bool *showScoreFlag, u8 *scoreDisplayMotionCounter ) {
        // debugVal = 666;
	u8 i;
	for( i=0; i<8; i++ ) {
		PA_SetSpriteXY( 0, arr[i], 256, 192);
		PA_SetSpriteMode( 0, arr[ i ], 0 ); // screen, sprite, alphablending
	}
	*scoreDisplayMotionCounter = 0;
	*showScoreFlag = false;
}


void scrollScoreDisplay( u8 scoreSprites[8], const double *SCORE_DISPLAY_OFFSET, u8 *scoreDisplayMotionCounter, s16 *scoreYpos ) {
	if( *scoreDisplayMotionCounter >= ARRAY_LEN( smoothMotionArray ) -1 ) return;
	u8 i;
	for( i=0; i<8; i++ ) {
		if( *scoreYpos + (s16)( *SCORE_DISPLAY_OFFSET * smoothMotionArray[ *scoreDisplayMotionCounter ] ) > 2 ) PA_SetSpriteY( 0, scoreSprites[i], *scoreYpos + (s16)( *SCORE_DISPLAY_OFFSET * smoothMotionArray[ *scoreDisplayMotionCounter ] ) );
	}
	*scoreDisplayMotionCounter = *scoreDisplayMotionCounter+1;
}


bool showScoreDisplay( u16 *scoreDisplayCounter, float *scoreDisplayFade ) {
	if( *scoreDisplayFade + 0.5 < 7 ) {	
		*scoreDisplayFade = *scoreDisplayFade + 0.2;

		PA_SetSFXAlpha( 0, (u16)*scoreDisplayFade, 15); // screen, alpha value (0-15), set to 15
	} else {
		if( *scoreDisplayCounter == 0 ) {
		        PA_SetSFXAlpha( 0, 7, 15); // screen, alpha value (0-15), set to 15       
		}
		*scoreDisplayCounter = *scoreDisplayCounter+1;				
	}

	if( *scoreDisplayCounter < 60 ) return false;

	*scoreDisplayCounter = 0;
	*scoreDisplayFade = 0;

	return true;		
}




u32 getMostUsedWordCount( u32 numberOfWords, char allWordsArray[ MAX_NUMBER_OF_WORDS ][8] ) {
	u32 i, j;
	u32 counter = 0;
	u32 tmpCounter = 0;
	// go through the list
	for( i=0; i<numberOfWords; i++ ) {
		tmpCounter = 0;
		// compare current word wich all words in the list
		for( j=0; j<numberOfWords; j++ ) {
			if( strcmp( allWordsArray[i], allWordsArray[j] ) == 0 ) {
				tmpCounter++;
			}
		}
		// was this word was more frequent? if so set returnVal to current word
		if( tmpCounter > counter ) {
			counter = tmpCounter;
		}
	}
	return counter;
}


u32 getMostUsedWord( u32 numberOfWords, char allWordsArray[ MAX_NUMBER_OF_WORDS ][8] ) {
	u32 i, j;
	u32 counter = 0;
	u32 tmpCounter = 0;
	u32 returnVal = 0;
	// go through the list
	for( i=0; i<numberOfWords; i++ ) {
		tmpCounter = 0;
		// compare current word wich all words in the list
		for( j=0; j<numberOfWords; j++ ) {
			if( strcmp( allWordsArray[i], allWordsArray[j] ) == 0 ) {
				tmpCounter++;
			}
		}
		// was this word was more frequent? if so set returnVal to current word
		if( tmpCounter > counter ) {
			counter = tmpCounter;
			returnVal = i;
		}
	}
	return returnVal;
}











void addNewBlock( CHARBLOCK topBlockArray[GRIDH][GRIDW], CHARBLOCK blockArray[GRIDH][GRIDW] ) {

	// first find the column with most missing blocks
	u8 n=0;
	u8 i;
	u8 colArray[ GRIDW ];
	for( i = 0; i < GRIDW; i++ ){
		if( !blockArray[ 0 ][ i ].active ) {
			colArray[ n ] = i;
			n++;
		}
	}

	// if all rows are full on bottom screen, then select a random row 	
	u8 tmpRow;
	if(n>0) {
		tmpRow = colArray[ PA_RandMinMax(0,n-1) ];
	} else {
		tmpRow = PA_RandMinMax(0,GRIDW-1);
	}
	
	// is the selected row (tmpRow) already full on top screen, then GAME OVER
	/******************************************************************/
	/* add check if the current position is occupied (then game over) */
	/******************************************************************/
	
	if( topBlockArray[0][tmpRow].active && topBlockArray[1][tmpRow].active && topBlockArray[2][tmpRow].active && topBlockArray[3][tmpRow].active && topBlockArray[4][tmpRow].active && topBlockArray[5][tmpRow].active ) {
		// GAME OVER!
		return;
//		displayScore = score;
//		gameState = GAME_OVER;
//		gameOver = true;
	} else if ( topBlockArray[0][tmpRow].active ) {
		// the first row is busy, no new block can enter at this time.
		return;
	}
	
	// create a block
	topBlockArray[0][tmpRow].active = true;



	if( addBomb ) {
		topBlockArray[0][tmpRow].character = STAR;
		addBomb = false;
	} else {
		switch( randomAlgorithm ) {
			case SIMPLE:
				topBlockArray[0][tmpRow].character = simpleRandomChar();								
				break;
			case SUPERPANIC:
				topBlockArray[0][tmpRow].character = superpanicRandomChar();								
				break;
			case JAYENKAI:
				topBlockArray[0][tmpRow].character = jayenkaiRandomChar();								
				break;
		}
	}

	
	// create sprite
	PA_CreateSprite(
		1,					// screen
		topBlockArray[0][tmpRow].spriteNum,	// sprite number (from array)
		(void*)wordblocks_Sprite,		// sprite name
		OBJ_SIZE_32X32,				// sprite size
		1,					// 256 color mode
		0,					// sprite palette number
		topBlockArray[0][tmpRow].x,		// x pos (* 32)
		topBlockArray[0][tmpRow].y		// y pos
	);
				
	// start sprite anim
	PA_SetSpriteAnim(
		1,					// screen
		topBlockArray[0][tmpRow].spriteNum,	// sprite number
		topBlockArray[0][tmpRow].character	// set frame to i
	);	


/*	
	// play new block sound	
	s8 tmpsnd;
	tmpsnd = GetFreeLimitedSoundChannel();
	if( tmpsnd > -1 ) PA_PlaySimpleSound( (u8)tmpsnd, sound_newblock );
*/

	
	// let the text background print on top of sprites
	PA_SetSpritePrio(1, topBlockArray[0][tmpRow].spriteNum, 1);
	
}



u32 getNewBlockTime() {
        // return a new delay timer
	return ( gameSpeed + PA_RandMinMax( 0, gameSpeed ) );
}











void hideCursor( CURSOR *cursor ) {
	(*cursor).xpos = -32;
	(*cursor).ypos = -32;
	(*cursor).idle = true;
	PA_SetSpriteXY(0, (*cursor).spriteNum, (*cursor).xpos, (*cursor).ypos );
}


void runCursor( CURSOR *cursor ) {
        /**********************************************
        	use a lut for a more exact and smooth movement
        	there are two possible ways of movement.
        	traight lines and diagonal lines
		a diagonal line is straight x and y at the same time.
	***********************************************/
	if( (*cursor).idle ) return;

	switch( (*cursor).target ) { // numbered clockwise
		case 0: // up
			if( (*cursor).stepCounter < ARRAY_LEN( CURSOR_MOVE_LUT ) ) {
				(*cursor).ypos = (*cursor).ypos - CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).stepCounter++;
			} else {
				(*cursor).stepCounter = 0;
				(*cursor).idle = true;
			}

		        break;
		        
		        
		case 1: // up right
			if( (*cursor).stepCounter < ARRAY_LEN( CURSOR_MOVE_LUT ) ) {
				(*cursor).ypos = (*cursor).ypos - CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).xpos = (*cursor).xpos + CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).stepCounter++;
			} else {
				(*cursor).stepCounter = 0;
				(*cursor).idle = true;
			}
			break;
		        
		        
		case 2: // right
			if( (*cursor).stepCounter < ARRAY_LEN( CURSOR_MOVE_LUT ) ) {
				(*cursor).xpos = (*cursor).xpos + CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).stepCounter++;
			} else {
				(*cursor).stepCounter = 0;
				(*cursor).idle = true;
			}
		        break;
		        
		        
		case 3: // right down
			if( (*cursor).stepCounter < ARRAY_LEN( CURSOR_MOVE_LUT ) ) {
				(*cursor).ypos = (*cursor).ypos + CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).xpos = (*cursor).xpos + CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).stepCounter++;
			} else {
				(*cursor).stepCounter = 0;
				(*cursor).idle = true;
			}
		        break;
		        
		        
		case 4: // down
			if( (*cursor).stepCounter < ARRAY_LEN( CURSOR_MOVE_LUT ) ) {
				(*cursor).ypos = (*cursor).ypos + CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).stepCounter++;
			} else {
				(*cursor).stepCounter = 0;
				(*cursor).idle = true;
			}
		        break;
		        
		        
		case 5: // down left
			if( (*cursor).stepCounter < ARRAY_LEN( CURSOR_MOVE_LUT ) ) {
				(*cursor).ypos = (*cursor).ypos + CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).xpos = (*cursor).xpos - CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).stepCounter++;
			} else {
				(*cursor).stepCounter = 0;
				(*cursor).idle = true;
			}
		        break;
		        
		        
		case 6: // left
			if( (*cursor).stepCounter < ARRAY_LEN( CURSOR_MOVE_LUT ) ) {
				(*cursor).xpos = (*cursor).xpos - CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).stepCounter++;
			} else {
				(*cursor).stepCounter = 0;
				(*cursor).idle = true;
			}
		        break;
		        
		        
		case 7: // left up
			if( (*cursor).stepCounter < ARRAY_LEN( CURSOR_MOVE_LUT ) ) {
				(*cursor).ypos = (*cursor).ypos - CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).xpos = (*cursor).xpos - CURSOR_MOVE_LUT[ (*cursor).stepCounter ];
				(*cursor).stepCounter++;
			} else {
				(*cursor).stepCounter = 0;
				(*cursor).idle = true;
			}
			break;
			
			
	}
	
	PA_SetSpriteXY( 0, (*cursor).spriteNum, (*cursor).xpos, (*cursor).ypos );
	
}


















/*********************************************
 *   Simple non weighted random algorithm    *
 *********************************************/


char simpleRandomChar() {
        // if lucky return star
        // if( PA_RandMinMax( 0, 60 ) == 0 ) return STAR;
        // return random char
	return PA_RandMinMax( 0, 25 );	
}

/******************************************************
 *   END OF "Simple non weighted random algorithm"    *
 ******************************************************/






/***************************************************************************
 *                                                                         *
 *  The Superpanic algorithm for finding well balanced random characters   *
 *  This method extracts one word at a time from the dictionary and uses   *
 *  up the characters of this word one at a time. When the characters are  *
 *  used up a new random word is extracted from the dictionary.            *
 *                                                                         *
 ***************************************************************************/

char randWord[8];
char superpanicRandomChar() {
        char returnChar;

	u8 i = 0;
	u8 j = 0;
        
        // if lucky return star
        // if( PA_RandMinMax( 0, 60 ) == 0 ) return STAR;
	
	// not lucky, return a random character from the current word
	u8 index;
	
	// if the string is empty, get a new random word
	if( strlen( randWord ) == 0 ) {
		
		u16 tmp0;
				
		// get a random index
		u32 tmp1 = PA_RandMinMax( 0, ARRAY_LEN( DICTIONARY ) -1 );

		
		
		for( tmp0=1; tmp0 < ARRAY_LEN(DICTIONARY_INDEX_ARRAY); tmp0++ ) {
			if( DICTIONARY_INDEX_ARRAY[tmp0] > tmp1 ) break;
		}
		
		u16 tmp2 = tmp0-1;

		// copy the first two characters
		randWord[0] = LEADING_CHARS_ARRAY[tmp2][0];
		randWord[1] = LEADING_CHARS_ARRAY[tmp2][1];
		// copy the rest of the word
		randWord[2] = DICTIONARY[tmp1][0];
		randWord[3] = DICTIONARY[tmp1][1];
		randWord[4] = DICTIONARY[tmp1][2];
		randWord[5] = DICTIONARY[tmp1][3];
		randWord[6] = DICTIONARY[tmp1][4];						
		randWord[7] = DICTIONARY[tmp1][5];

		// randWord now contains a new random word

	}



	// extract a random char from the word
	index = PA_RandMinMax( 0, strlen(randWord)-1 );
	returnChar = randWord[ index ];
	
	// strip the extracted character from the word
	char newWord[8];

	for( i=0; i < strlen( randWord ); i++ ) {
		if( i == index ) { 
		        continue;		// jump over the selected char
		}
		newWord[j] = randWord[i];
		j++; 
	}
	newWord[j] = '\0';
	strcpy(randWord, newWord);
	
	return returnChar-'A';
}

/************************************************
 *                                              *
 *  END of "Superpanics word based algorithm"   *
 *                                              *
 ************************************************/





/**********************************************************
 *                                                        *
 *  The Jayenkai algorithm for random characters          *
 *  Thanks for your help Jayenkai!                        *
 *                                                        *
 *  Replaced bombs with S as S is used more               *
 *  extensively compared to scrabbles as words            *
 *  in plural is perfectly legal.                         *
 *                                                        *
 **********************************************************/

// 1=A, 2=B, 26=Z, 27="bomb"
int perc[]= {
	0,								// nothing!
	18,5,5,8,24,4,6,4,17,2,2,8,4,13,14,4,2,12,8,12,8,4,4,2,4,2,	// A-Z (total 196)
	50								// the bomb (only 4 are used)
}; 

int Bag[250];
int TileGet;

//Use to fill "Bag"
void fillBag() {
	int i, n, x, y;
	i=0;
	
	for( n=0; n<250; n++ ){
		Bag[n]=0;
	}
	
	//Shove tiles into bag
	for( n=0; n<28; n++ ) {
		for( x=0; x < perc[n]; x++ ) {
		        // replace bomb with 'S'
		        if( n == 27 ) n = 19;
			// put in bag
			Bag[i]=n;
			i++;
		}
	}
	
	//Shuffle Bag	
	for( n=0; n<200; n++ ) {
		x=PA_RandMinMax(0,199);
		y=Bag[n];
		Bag[n]=Bag[x];
		Bag[x]=y;		
	}
        
	//Shuffle again for good luck
	for( n=0; n<200; n++ ) {
		x=PA_RandMinMax(0,199);
		y=Bag[n];
		Bag[n]=Bag[x];
		Bag[x]=y;		
	}
	
	TileGet=0;
}

char jayenkaiRandomChar() {
	int i;
	i = Bag[ TileGet ];
	TileGet++;
	if ( TileGet == 200 ) { 
		fillBag(); 
	}
	return i-1;
}


/*****************************************
 *                                       *
 *  END of "Jayenkai Bag of many words"  *
 *                                       *
 *****************************************/































void resetCHARBLOCK( CHARBLOCK *p_block ) {
	//(*p_block).x = -64;	
	//(*p_block).y = -64;		
	//(*p_block).spriteNum = 0;
	(*p_block).yoffset = 0;
	(*p_block).velocity = 0;
	(*p_block).character = 0;
	(*p_block).angle = 0;
	(*p_block).zoom = 256;
	(*p_block).rotSet = 0;
	(*p_block).rotDelay = 0;
	(*p_block).busy = false;
	(*p_block).active = false;
	(*p_block).xFloatOffset = 0;
	(*p_block).yFloatOffset = 0;
}



// ### SELECTION OF BLOCKS

	// clear current array of blocks
void clearSelection( CHARBLOCK *p_selectedBlocksArray[MAX_WORD_LENGTH], u8 nSelected ) {
	// clear list, set all values to NULL
	u8 i;
	for( i=0; i<nSelected; i++ ) {
		// reset the sprite alpha effect before erasing the pointer from the array
        	// reset the alpha
        	PA_SetSpriteMode( 0, (*p_selectedBlocksArray[i]).spriteNum, 0 ); // screen, sprite, alphablending
		p_selectedBlocksArray[i] = NULL;
	}
	// memset( selectedBlocksArray, NULL, sizeof( selectedBlocksArray ) );
}


void clearSelectionRange( CHARBLOCK *p_selectedBlocksArray[MAX_WORD_LENGTH], u8 *nSelected, u8 s ) {
	u8 i;
	for( i=s; i<*nSelected; i++ ) {
		PA_SetSpriteMode( 0, (*p_selectedBlocksArray[i]).spriteNum, 0 ); // screen, sprite, alphablending
		p_selectedBlocksArray[i] = NULL;
	}
	// if s == 0 set nSelected to 0 (not 1) as it will increase later!
	*nSelected = s;
}


// return a pointer to currently touched block
CHARBLOCK *getPointerToTouchedCharBlock( CHARBLOCK ba[GRIDH][GRIDW] ) {
	// only one block can be touched at a time, return block when found.
	u8 yi, xi;
	for( yi=0; yi<GRIDH; yi++ ) {
		for( xi=0; xi<GRIDW; xi++ ) {
			// block is busy, continue
			if( ba[yi][xi].busy ) continue;
			if( PA_SpriteTouched( ba[yi][xi].spriteNum ) ) {
			        
			        // before returning the sprite, make a boundary 
				// check as only the center area of the sprite is valid
			        s8 x = ba[yi][xi].x + 16;
			        s8 y = ba[yi][xi].y + 16;			        
			        if( ( ABS( (s8)Stylus.X - x ) <= 8 ) && ( ABS( (s8)Stylus.Y - y ) <= 8 ) ) {
					return &ba[yi][xi];
				}
								
			}
		}
	}
	return NULL;
}




// return a pointer to currently touched block
CHARBLOCK *getPointerToTouchedCharBlock_noBoundCheck( CHARBLOCK ba[GRIDH][GRIDW] ) {
	// only one block can be touched at a time, return block when found.
	u8 yi, xi;
	for( yi=0; yi<GRIDH; yi++ ) {
		for( xi=0; xi<GRIDW; xi++ ) {
			// block is busy, continue
			if( ba[yi][xi].busy ) continue;
			if( PA_SpriteTouched( ba[yi][xi].spriteNum ) ) {
			        return &ba[yi][xi];								
			}
		}
	}
	return NULL;
}









// ### WORD STRING COMPARATION

	// make a string of characters from touched blocks
void buildWord( CHARBLOCK *p_currentBlock ) {
	if( strlen( currentWord ) < MAX_WORD_LENGTH ) {
	        if( (*p_currentBlock).character == STAR ) {
			currentWord[strlen( currentWord )] = STAR + 'A';
			return;
		}		
		currentWord[strlen( currentWord )] = (*p_currentBlock).character + 'A';
		// wordLength = 1;
	}
}



	// make a string of characters from touched blocks
void buildName( CHARBLOCK *p_currentBlock ) {
	if( strlen( currentNameString ) < MAX_NAME_LENGTH ) {
		currentNameString[strlen( currentNameString )] = (*p_currentBlock).character + 'A';
	}
}



bool starSelected( char s[8] ) {
	u8 i=0;
        while( s[i] != '\0' ) {
	        if( s[i] == STAR + 'A' ) return true;
		i++;
	}
	return false;
}






	// find an array in LEXICON_ARRAY starting with words of same first character
bool searchLexicon( char s[8] ) {

	//if word is found in array then return true	
	//if word is not found in array then return false

	// first create the two strings
	// one with two chars
	// one with the rest

	char	firstChars[3];	// two chars + '\0'
	char	lastChars[6];	// five chars + '\0'

	u32	startSearch = 0;
	u32	endSearch = 0;
	
	// copy the first chars to firstChars:
	firstChars[0] = s[0];
	firstChars[1] = s[1];
	firstChars[2] = '\0';
	
	//debugVal = strlen( firstChars );
	
	// copy the remaining chars to lastChars:
	u32 i = 2;	
	memset( lastChars, '\0', sizeof( lastChars ));
	while( s[i] != '\0' ) {
	        lastChars[(i-2)] = s[i];
		i++;
	}
	
//	memset( debugStr, '\0', sizeof( debugStr ));
//	strcpy( debugStr, lastChars );

	// run first test
	bool failed = true;

	for( i=0; i < ARRAY_LEN( LEADING_CHARS_ARRAY ); i++ ) {
		if( strcmp( LEADING_CHARS_ARRAY[i], firstChars) == 0 ) {
			// get index and exit this loop
			startSearch = DICTIONARY_INDEX_ARRAY[i];
			if( i < ARRAY_LEN( LEADING_CHARS_ARRAY )-1 ) {
				endSearch = DICTIONARY_INDEX_ARRAY[i+1]-1;
			} else {
				endSearch = ARRAY_LEN( DICTIONARY )-1;
			}
			failed = false;
		}
	}

	// debugVal = endSearch;

	// if test failed return, no need to run final search!
	if( failed ) return false;

	// second and final search
	for( i=startSearch; i <= endSearch; i++ ) {
		if( strcmp( DICTIONARY[i], lastChars) == 0 ) {
			return true;
		}
	}
	return false;	

}	




bool flyingSpritesActive = false;


void setupFlyingSprites() {

	// zOffsetBottomScreen;

	if( flyingSpritesActive ) return;

	u8 i;
	for( i=0; i<6; i++ ) {
		
		flyingSprites[i].spriteNum = 12-i;
		flyingSprites[i].x = 0;
		flyingSprites[i].y = 0;
		

		flyingSprites[i].floatXOffset = 256;
        	flyingSprites[i].floatYOffset = 192;		
		
		flyingSprites[i].xVelocity = 0;
		flyingSprites[i].yVelocity = 0;


		PA_CreateSprite(
			0,				// screen
			flyingSprites[i].spriteNum,	// sprite number
			(void*)wordblocks_Sprite,	// sprite name
			OBJ_SIZE_32X32,			// sprite size
			1,				// 256 color mode
			0,				// sprite palette number
			(s16)flyingSprites[i].floatXOffset,		// x pos (* 32)
			(s16)flyingSprites[i].floatYOffset		// y pos
		);

		PA_SetSpriteAnim(
			0,				// screen
			flyingSprites[i].spriteNum,	// sprite number
			superpanicRandomChar()		// set frame to i
		);

		// let the text background print on top of sprites
		PA_SetSpritePrio(0, flyingSprites[i].spriteNum, 1);	
	}
	
	flyingSpritesActive = true;
	
}


void runFlyingSprites() {
	u8 i;
	
	for( i=0; i<6; i++ ) {
		// move
		// if moved outsite
		// get new position
		// get new direction

		// move the sprites
		flyingSprites[i].floatXOffset = flyingSprites[i].floatXOffset + flyingSprites[i].xVelocity;
		flyingSprites[i].floatYOffset = flyingSprites[i].floatYOffset + flyingSprites[i].yVelocity;
		PA_SetSpriteXY( 0, flyingSprites[i].spriteNum, (s16)flyingSprites[i].floatXOffset, (s16)flyingSprites[i].floatYOffset );
		
		// check boundaries
		if( flyingSprites[i].floatXOffset < 0-32 || flyingSprites[i].floatXOffset > 255 || flyingSprites[i].floatYOffset < 0-32 || flyingSprites[i].floatYOffset > 191 ) {
			bool tmpx = (bool)PA_RandMax( 1 );
			bool tmpy = (bool)PA_RandMax( 1 );
			
			bool xdir;
			bool ydir;
			
			// set new random x position
			if( tmpx ) {
				flyingSprites[i].floatXOffset = (double)PA_RandMinMax(0,255)-31;
				if( tmpy ) {
					flyingSprites[i].floatYOffset = -31;
				} else {
					flyingSprites[i].floatYOffset = 191;
				}
			} else {
				flyingSprites[i].floatYOffset = (double)PA_RandMinMax(0,255+32)-31;
				if( tmpy ) {
					flyingSprites[i].floatXOffset = -31;
				} else {
					flyingSprites[i].floatXOffset = 255;
				}	
			}
			
			if( flyingSprites[i].floatXOffset < 128 ) {
				xdir = true;
			} else {
				xdir = false;
			}

			if( flyingSprites[i].floatYOffset < 96 ) {
				ydir = true;
			} else {
				ydir = false;
			}

			
			setFlyingSpriteVelocity( &(flyingSprites[i]), xdir, ydir );
			PA_SetSpriteAnim( 0, flyingSprites[i].spriteNum, superpanicRandomChar() );
		}
	
	}
}

void setFlyingSpriteVelocity( SPRITE_WRAPPER *fs, bool xdir, bool ydir ) {
	(*fs).xVelocity = ( (double)PA_RandMinMax( 1, 20 ) / 10. );
	(*fs).yVelocity = ( (double)PA_RandMinMax( 1, 20 ) / 10. );
	if( !xdir ) (*fs).xVelocity = (*fs).xVelocity * -1.;
	if( !ydir ) (*fs).yVelocity = (*fs).yVelocity * -1.;
}

void cleanFlyingSprites() {
	u8 i;
	for( i=0; i<6; i++ ) {
		PA_DeleteSprite( 0, flyingSprites[i].spriteNum );
	}

	flyingSpritesActive = false;

}


void stop() {
	while( true ) PA_WaitForVBL();
}

// ### SPECIAL EFFECTS

// return a new alpha value, used to animate a fast fade of a selected block
u16 getAlpha( u16 _alpha ) {
	if( _alpha > 7 ) {
		return --_alpha;
	} 
	return 15;
}


