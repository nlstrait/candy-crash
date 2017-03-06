#ifndef __GAMESTATE_H
#define __GAMESTATE_H

extern "C" {
  #include <stdbool.h>
  #include "GameDef.h"
}

#include "Block.h"
#include "Candy.h"




// ------------------------------STRUCTS------------------------------


// IndexPair struct wraps a row index and a column index
//
// Components:
// -row: the row number 
// -col: the column number 
typedef struct IndexPair_st {
  int row;
  int col;
} * IndexPair;


// Game struct represents essential information about the game
//
// Components:
// -candies: a 2d array holding Candy objects
// -states: a 2d array holding the current state of the candies
// -index_pair_arr: a 2d array holding the IndexPairs of the candies
// -fires: the fired positions of the candy board
// -extensionOffset: the offset on each column of GameDef board
// -movesMode: the number of moves made by the player
// -score: the score obtained by the player
// -numColsFired: the number of candies fired
typedef struct GameState_st {
  array2d candies;
  array2d states;
  array2d index_pair_arr;

  Block** fires;
  int* extensionOffset;
  
  int movesMade;
  int score;
  int numFired;
} * GameState;




// ------------------------------CONSTRUCTORS AND DESTRUCTORS------------------------------


// IndexPair struct represents a pair of index (row, col)
//
// Components:
// -i: the row number
// -j: the column number
IndexPair IndexPair_new(int i, int j);


// Start the Candy Crush game. This method contains the deserialization
// of the JSON file to create the initial candy crush board
//
// Arguments:
// -json: pointer to json object to constructs GameState from
//
// Returns:
// A pointer to the GameState struct; NULL if run out of memory
GameState GameState_new(json_t* json);


// Start the Candy Crush game. This method is used at the very beginning
// of the game when the JSON does not include the gamestate
//
// Arguments: 
// -gamedef: the GameDef we have to initialze the gamestate from
GameState GameState_new_from_GameDef(GameDef gamedef);


// Free the memory used by the GameState struct. Intend to be used
// at the delete-event of the window
//
// Arguments:
// -g: the GameState struct
//
// Returns:
// 0 if executed successfully. -1 otherwise
int GameState_destroy(GameState g);


// Free the array where Block objects are stored
// 
// -g: the current game state
void GameState_freeBlocks(GameState g);




// ------------------------------TEMPLATE FUNCTIONS------------------------------


// If there are template found from the current board
//
// Arguments:
// -g: the GameState g
//
// Returns:
// true if there are templates made; false otherwise
bool GameState_templateFound(GameState g);


// Scan through the current candy board and find matching templates
//
// Arguments:
// -g: the current game state
int GameState_checkTemplates(GameState g);


// Checks for the presence of a vertical template
//
// Arguments
// -g: the GameState to check for templates
// -height: the height of the vertical template to check
//
// Returns the number of fires detected
int GameState_checkVertical(GameState g, int height);


// Checks for the presence of a horizontal template
//
// Arguments
// -g: the GameState to check for templates
// -width: the width of the horizontal template to check
//
// Returns the number of fires detected
int GameState_checkHorizontal(GameState g, int width);




// ------------------------------BOARD ACTION FUNCTIONS------------------------------


// Swap the candy the two given indices
//
// Arguments:
// -g: the GameState struct
// -d: the GameDef struct
// -selected: the index of the candy currently selected
// -swapWith: the index of the candy to swap with
//
// Returns:
// the number of candies fired from this swap; -1 if no templates
// were made
int GameState_swap(GameState g, GameDef d, IndexPair selected, IndexPair swapWith);


// Inflict the gravity on the board after the firing 
//
// Arguments:
// -gamestate: the current game state
// -gamedef: the current game defintion
void GameState_inflictGravity(GameState gamestate, GameDef gamedef);


// Check if the position is already in one of our firing positions
//
// Arguments:
// -g: the current game state
// -row: the row index
// -col: the column index
bool GameState_alreadyFired(GameState g, int row, int col);

// Add the FireInfo to the array
//
// Arguments: 
// -g: the current game state
// -block: the FireInfo object to be added
void GameState_addFire(GameState g, Block *block);




// ------------------------------SERIALIZATION------------------------------


// Create and return a json object from GameState g
json_t* GameState_serialize(GameState g);


#endif // __GAMESTATE_H
