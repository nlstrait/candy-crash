#ifndef _GAMEDEF_H
#define _GAMEDEF_H

extern "C" {
  #include <jansson.h>
  #include "array2d.h"
}

// definition:
//
// A GameDef_st struct represents the basic information we get
// from JSON file
//
// Fields:
// -extensionboard: two-dimensional array representing the backup board
// -boardstate: the total number of fires each candy needs
// -gameid: a unique id representing the game
// -colors: how many kinds of candies the game has
typedef struct GameDef_st {
  array2d extensionboard;
  array2d boardstate;
  json_t* gamedef;
  int gameid;
  int colors;
} * GameDef;

// Creates a GameDef from the given JSON pointer
//
// Arguments:
// -json: the given JSON pointer
//
// Returns:
// A GameDef pointer
GameDef GameDef_new(json_t* json);


json_t* GameDef_serialize(GameDef d);
// Destroy the given GameDef and free the memory
//
// Arguments:
// -g: the given GameDef to be destroyed
//
// Returns:
// 0 if destroy is successful. other number otherwise
int GameDef_destroy(GameDef g);


#endif  // _GAMEDEF_H
