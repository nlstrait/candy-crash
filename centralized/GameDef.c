extern "C" {
  #include "GameDef.h"
}
#include <stdlib.h>

void PayloadFreeFn(payload_t ptr) {
  if(ptr != NULL) {
    free(ptr);
    ptr = NULL;
  }
} 

GameDef GameDef_new(json_t* json) {
  // json
  // create GameDef
  GameDef g = (GameDef)malloc(sizeof(struct GameDef_st));
  if(g == NULL) {
    return NULL;
  }

  g->gamedef = json;
  // get the fields ready
  json_t* extension;
  json_t* board;
  int json_gameid;
  int json_colors;
  
  json_unpack(json, "{s:i, s:o, s:o, s:i}", "gameid", &json_gameid, "extensioncolor", &extension, "boardstate", &board, "colors", &json_colors);
  
  // set fields
  g->gameid = json_gameid;
  g->colors = json_colors;


  // deserialize the arrays
  payload_t value;
  // extension
  json_t* extensionArr;
  array2d_deserialize(&(g->extensionboard), extension, &extensionArr); 
  int arrayCols = array2d_getColumnSize(g->extensionboard);
  for(unsigned int i = 0; i < json_array_size(extensionArr); i++) {
    // value
    int j = json_integer_value(json_array_get(extensionArr, i));
    value = (payload_t) malloc(sizeof(int));
    *(int*)value = j;
    array2d_update(g->extensionboard, value, i / arrayCols, i % arrayCols);
  }
  
  // boardstate
  json_t *boardArr;
  array2d_deserialize(&(g->boardstate), board, &boardArr); 
  //arrayCols = array2d_getColumnSize(g->boardstate);
  arrayCols = g->boardstate->numCols;
  for(unsigned int i = 0; i < json_array_size(boardArr); i++) { 
    int j = json_integer_value(json_array_get(boardArr, i));
    value = (payload_t) malloc(sizeof(int));
    *(int*)value = j;
    array2d_update(g->boardstate, value, i / arrayCols, i % arrayCols);
  }

  return g;
}

json_t* GameDef_serialize(GameDef d) {
  return d->gamedef;
}
int GameDef_destroy(GameDef d) {
  if(d == NULL) {return -1;}  
  // free array
  int code = 0;
  code = array2d_destroy(d->extensionboard, &PayloadFreeFn);
  d->extensionboard = NULL;
  code &= array2d_destroy(d->boardstate, &PayloadFreeFn);
  d->boardstate = NULL;
  // decref
  json_decref(d->gamedef);
  // free the rest
  free(d);
  d = NULL;
  return code;
}
