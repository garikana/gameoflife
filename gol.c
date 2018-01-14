#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* This program uses the hardware rendering API display primitive
   shapes on the screen. We will also use viewports to segment the screen
   into mini rectangles. Each viewport will be of size d_VIEW_PORT*d_VIEW_PORT 
   Each viewport is surrounded by 8 viewports
   The x,y co-ordinates for each of them are
   - top----------->(x, y-4)
   - top lef------->(x-4,y-4)
   - top rig------->(x+4,y-4)
   - lef----------->(x-4,y)
   - rig----------->(x+4,y)
   - bot lef------->(x-4,y+4)
   - bot rig------->(x+4,y+4)
   - bot----------->(x,y+4)
   
   Check if anytime the x and y values are beyond the limits of SCREEN_WIDTH & SCREEN_HEIGHT
   d_VIEW_PORT ----> View port height and width(square)
*/
// define bool data type
#define bool int
#define true 1
#define false 0
bool init(void);
bool loadMedia();
void sdlclose(void);

// screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// viewport dimension(square)
#define d_VIEW_PORT 8

//Image path
char *path = NULL;

  //The window we'll be rendering to
  SDL_Window *window = NULL;

// Loads individual image as a texture
SDL_Texture *loadTexture(char *);

// The renderer attached to the window
SDL_Renderer *gRenderer = NULL;

//draw the gol map
void drawMap(SDL_Renderer *);

//gol map representation
int a[SCREEN_WIDTH][SCREEN_HEIGHT] = {{0 }};

/*************************************/
/*The below definitions are related to evaluateMap() function*/

// copy 2 dim arr at src to dest                                                                                                
// assumes arrays are of same dimensions                                                                                        
void arrcpy(int a[SCREEN_WIDTH][SCREEN_HEIGHT], int b[SCREEN_WIDTH][SCREEN_HEIGHT]);
int c[SCREEN_WIDTH][SCREEN_HEIGHT] = {{0 }}; /* buffer world */
void evaluateMap(void);
/************************************/


int main(int argc, char *argv[])
{

  /* Initial Beacon pattern 
a[1][1] = 1; a[2][1] = 1;
a[1][2] = 1; a[2][2] = 1;

a[3][3] = 1; a[4][3] = 1;
a[3][4] = 1; a[4][4] = 1;
  */

  /* Initial Toad pattern 
  a[2][2] = 1; a[3][2] = 1; a[4][2] = 1;
  a[1][3] = 1; a[2][3] = 1; a[3][3] = 1;
  */

  /* glider pattern */
  a[1][1] = 1; a[2][2] = 1;
  a[0][3] = 1; a[1][3] = 1; a[2][3] = 1;

  //Start up SDL and create Window
  if(init() == false){
      printf("Failed to initialize SDL!\n");
    }
  else
    {
      // Load Media
      if( !loadMedia() )
	printf("Failed to load media\n");
      else{
	
      
	bool quit = false;
	SDL_Event e;

	// Main game loop should start here
	while( quit != true){

	  //Check the event queue
	  while(SDL_PollEvent(&e) != 0){
	    //User requests quit
	    if(e.type == SDL_QUIT)
	      quit = true;
	  }

	  // Clear the screen
	  SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	  SDL_RenderClear(gRenderer);
	  
	  // Render red filled quad; fill the SDL_Rect struct
	  /*
	  SDL_Rect fillRect = { 0, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
	  SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF);
	  SDL_RenderFillRect(gRenderer, &fillRect);
	  */
	  /*
	  if( SDL_RenderSetScale(gRenderer, 2, 2) != 0){
	    printf("Unable to set Render scale! %s\n", SDL_GetError());
	    sdlclose();
	    return 0;
	  }
	  */

	  SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
	  // evaluate the gol map
	  evaluateMap();
	  
	  // draw the gol map
	  drawMap(gRenderer);

	//Update the screen
	  SDL_RenderPresent(gRenderer);

	  SDL_Delay(500);
   
	}
      }
    }

  sdlclose();

  return 0;

}

void evaluateMap()
{
  int nrating, x, y;
  arrcpy(c, a);
  int i,j;

    for(i=0; i<SCREEN_WIDTH; i++)
      for(j=0; j<SCREEN_HEIGHT; j++){
	//calc neighbour rating for each cell                                                                                     
	//neighbour rating = sum of values of each of the 8 neighbour cells                                                       
	nrating = 0;
	for(x=-1; x<2; x++)
	  for(y=-1; y<2; y++){
	    if(x==0 && y==0){  /* skip the centre cell */
	      continue;
	    }else{
	      if(i+x > -1 && i+x < SCREEN_WIDTH && j+y > -1 && j+y < SCREEN_HEIGHT)
		nrating += a[i+x][j+y];
	    }

	  }
	/* if cell is dead & nrating == 3, make it alive */
	if(a[i][j] == 0 && nrating == 3)
	  c[i][j] = 1;

	/* if cell is alive & nrating < 2, make it dead */
	if(a[i][j] == 1 && nrating < 2)
	  c[i][j] = 0;

	/* if cell is alive & nrating > 3, make it dead */
	if(a[i][j] == 1 && nrating > 3)
	  c[i][j] = 0;

      }
    arrcpy(a, c);

}

void arrcpy(int dest[SCREEN_WIDTH][SCREEN_HEIGHT], int src[SCREEN_WIDTH][SCREEN_HEIGHT])
{
  for( int i=0; i < SCREEN_WIDTH; i++)
    for(int j=0; j < SCREEN_HEIGHT; j++)
      //      *(*(dest+i)+j) = *(*(src+i)+j);                                                                                   
      dest[i][j] = src[i][j];

}

void drawMap(SDL_Renderer *gRenderer)
{

  int i,j;

	  SDL_Rect *viewp;
	  
	  for(i=0; i<SCREEN_WIDTH; i++)
	    for(j=0; j<SCREEN_HEIGHT; j++)
	      // if 1 then only allocate a viewport
	      if(a[i][j]){	      
		 if((viewp = (SDL_Rect *)malloc(sizeof(SDL_Rect))) != NULL){
		   viewp->x = i*d_VIEW_PORT;
		   viewp->y = j*d_VIEW_PORT;
		   viewp->w = d_VIEW_PORT;
		   viewp->h = d_VIEW_PORT;
		   SDL_RenderSetViewport(gRenderer, viewp);

		   SDL_Rect fillRect = { 0, 0, d_VIEW_PORT, d_VIEW_PORT};
		   SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF);
		   SDL_RenderFillRect(gRenderer, &fillRect);
		   free(viewp);
		 }		

	      }

}
bool init()
{

  bool success = true;

  if( SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      printf(" SDL could not initialise! SDL_Error: %s\n",SDL_GetError());
      success = false;
    }
  else
    {
      //Set texture filtering to linear
      if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1") )
	printf("Warning: Linear texture filtering not enabled\n");

      //Create Window
      window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, 
				SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
      if(window == NULL){
	printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
	success = false;
      }else{
	//Create renderer for the created window
	gRenderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
	if(gRenderer == NULL){
	  printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
	  success = false;
	}else{
	  //Initialise renderer color
	  SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	  // Initialise PNG loading
	  int imgFlags = IMG_INIT_PNG;
	  if( !( IMG_Init( imgFlags ) & imgFlags) ){
	    printf("SDL_Image could not initialise! SDL_image Error: %s\n", IMG_GetError());
	    success = false;
	  }

	}

      }
	
    }

  return success;
}

bool loadMedia()
{
  //Loading success flag
  bool success = true;
  
  return success;
}

void sdlclose()
{
  //Deallocate the renderer
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(window);
  window = NULL;
  gRenderer = NULL;
  
  //Quit the subsystems
  IMG_Quit();
  SDL_Quit();

}

SDL_Texture *loadTexture(char *path)
{
  SDL_Texture *newTexture = NULL;
  
  //Load image at the specified path
  SDL_Surface *loadedSurface = IMG_Load(path);
  if (loadedSurface == NULL){
    printf("Unable to load image %s! SDL_image_error: %s\n",path, SDL_GetError());
  }else{
    // Create the texture from the loaded surface
    newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
    if( newTexture == NULL)
      printf(" Unable to create texture from %s ! SDL Error: %s\n", path, SDL_GetError());
    
    // Deallocate the loaded Surface; we only need the texture
    SDL_FreeSurface(loadedSurface);

  }
  return newTexture;
}
