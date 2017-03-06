extern "C" {
  #include "GameState.h"
}
#include "model.h"


void StateFreeFn(payload_t ptr) {
  int* state = (int*) ptr;
  if (state != NULL) {
    free(state);
  }
}

void CandyFreeFn(payload_t ptr) {
  Candy *candy = (Candy*) ptr;
  if (candy != NULL) {
    delete candy;
  }
}


void IndexPairFreeFn(payload_t ptr) {
  IndexPair pair = (IndexPair)ptr;
  if(pair != NULL) {
    free(pair);
  }
}

void GameState_freeBlocks(GameState g) {
  for(int i = 0; i < g->numFired; i++) {
    if(g->fires[i] != NULL) {
      delete g->fires[i];
    } else {
      break;
    }
  }
}

GameState GameState_new(json_t* json) {

  GameState g = (GameState) malloc(sizeof(struct GameState_st));
  if(g == NULL) {
    // out of memory!
    return NULL;
  }

  // deal with json objects
  json_t* boardCandies;
  json_t* boardState;
  int movesMade;
  int currentScore;
  json_t* extensionOffset;

  json_unpack(json, "{s:o, s:o, s:i, s:i, s:o}", "boardcandies", &boardCandies, "boardstate",
    &boardState, "movesmade", &movesMade, "currentscore", &currentScore, "extensionoffset",
    &extensionOffset);

  json_t* boardCandiesArr;
  array2d_deserialize(&(g->candies), boardCandies, &boardCandiesArr);
  
  json_t* boardStateArr;
  array2d_deserialize(&(g->states), boardState, &boardStateArr);
   
  // get array dimensions
  int numRows = g->candies->numRows;
  int numCols = g->candies->numCols;
  int numCandies = numRows * numCols;

  // Intialize GameState arrays
  g->candies = array2d_new(numRows, numCols);
  g->states = array2d_new(numRows, numCols);
  g->index_pair_arr = array2d_new(numRows, numCols);

  // unpack and place candies and states in GameState's 2D arrays
  for (int i = 0; i < numCandies; i++) {
    // fetch Candy color and type from json
    json_t* thisCandyJson = json_array_get(boardCandiesArr, i);
    int thisColor;
    int thisType;
    json_unpack(thisCandyJson, "{s:i, s:i}", "color", &thisColor, "type", &thisType);
    array2d_update(g->candies, new Candy(thisColor, thisType), i / numCols, i % numCols);

    // fetch state from json
    int* thisState = (int*) malloc(sizeof(int));
    if (thisState == NULL) {
      printf("%s", "malloc failure\n");
      return NULL;
    }
    *thisState = json_integer_value(json_array_get(boardStateArr, i));
    array2d_update(g->states, thisState, i / numCols, i % numCols);
    
    // add index pair
    IndexPair index_pair = IndexPair_new(i / numCols, i % numCols);
    array2d_update(g->index_pair_arr, index_pair, i / numCols,  i % numCols); 
  }

  // deal with moves, score, and extension offset
  g->movesMade = movesMade;
  g->score = currentScore;
  g->extensionOffset = (int*) malloc(numCols * sizeof(int));
  if (extensionOffset == NULL) {
    printf("%s", "malloc failure\n");
    return NULL;
  }
  for (int i = 0; i < numCols; i++) {
    int thisEF = json_integer_value(json_array_get(extensionOffset, i));
    g->extensionOffset[i] = thisEF;
  }
  
  // an IndexPair to hold firePositions
  g->selected = NULL;
  g->fires = (Block**)malloc(sizeof(Block*) * numRows * numCols);
  g->numFired = 0;

  //decref
  json_decref(boardStateArr);
  json_decref(boardCandiesArr);
  // return!
  return g;
}


GameState GameState_new_from_GameDef(GameDef gamedef){


  GameState g = (GameState) malloc(sizeof(struct GameState_st));
  if(g == NULL) {
    // out of memory!
    return NULL;
  }
  
  int numRows = gamedef->boardstate->numRows; 
  int numCols = gamedef->boardstate->numCols; 
  g->candies = array2d_new(numRows, numCols);
  g->states = array2d_new(numRows, numCols);
  g->index_pair_arr = array2d_new(numRows, numCols);
  for(int i = 0; i < numRows; i++) {
    for(int j = 0; j < numCols; j++) {
      payload_t payload;
      array2d_get(gamedef->extensionboard, &payload, i, j);
      Candy *c = new Candy(*(int*)payload, 0);
      array2d_update(g->candies, c, i, j);
      // get boardstate 
      array2d_get(gamedef->boardstate, &payload, i, j);
      int* state = (int*)malloc(sizeof(int));
      *state = *(int*)payload;
      array2d_update(g->states, state, i, j);
      IndexPair ip = IndexPair_new(i, j);
      array2d_update(g->index_pair_arr, ip, i, j);
    }
  }
  g->selected = NULL;
  g->score = 0;
  g->movesMade = 0;
  g->extensionOffset = (int*) malloc(numCols * sizeof(int));
  for (int i = 0; i < numCols; i++) {
    g->extensionOffset[i] = gamedef->extensionboard->numRows - numRows;
  }
  g->fires = (Block**)malloc(sizeof(Block*) * numRows * numCols);
  g->numFired = 0;
  // find templates and inflict gravity
  int temp = GameState_checkTemplates(g);
  while(GameState_templateFound(g)) {
    g->score += temp;
    GameState_inflictGravity(g, gamedef);
    temp = GameState_checkTemplates(g);
  }  
  return g;
}


IndexPair IndexPair_new(int i, int j) {
  IndexPair idx = (IndexPair)malloc(sizeof(struct IndexPair_st));
  idx->row = i;
  idx->col = j;
  return idx;
}


int GameState_selected(GameState g, IndexPair p) {
  if(g == NULL) {
    return -1;
  }  
  
  if(g->selected == NULL) {
    g->selected = (IndexPair)malloc(sizeof(struct IndexPair_st));
  }
  g->selected->row = p->row;
  g->selected->col = p->col;
  return 0;
}

bool GameState_templateFound(GameState g) {
  return g->numFired != 0;
}

int GameState_swap(GameState g, GameDef d, IndexPair selected, IndexPair swapWith) {
  array2d_swap(g->candies, selected->row, selected->col, swapWith->row, swapWith->col);
  int count = GameState_checkTemplates(g);
  // if no fire Positions
  if(!GameState_templateFound(g)) {
    count = -1;
    // swap it back
    array2d_swap(g->candies, swapWith->row, swapWith->col, selected->row, selected->col);
  } else {
    GameState_inflictGravity(g, d);
  }
  return count;
}

void GameState_inflictGravity(GameState gamestate, GameDef gamedef) {
  if (!GameState_templateFound(gamestate)) {
    return;
  }
  array2d candies = gamestate->candies;
  array2d extension = gamedef->extensionboard;
  for (int i = 0; i < gamestate->numFired; i++) {
    Block* blk = gamestate->fires[i];
    int col = blk->col;
    int gapRowsIndex = 0;
    int curRow = blk->row;
    // drop down gap Candies
    while (gapRowsIndex < blk->numGapRows) {
      Candy* toMove;
      array2d_get(gamestate->candies, (payload_t*) &toMove, blk->gapRows[gapRowsIndex++], col);
      Candy* old;
      array2d_get(gamestate->candies, (payload_t*) &old, curRow++, col);
      old->color = toMove->color;
      old->type = toMove->type;
    }
    // iterate over rows
    for (int row = curRow; row < candies->numRows; row++) {
      int imgRow = row + blk->height - blk->numGapRows;
      int newColor;
      int newType = 0;
      payload_t payload;
      
      if (imgRow >= candies->numRows) {
        array2d_get(extension, &payload, gamestate->extensionOffset[col], col);
        newColor = *(int*)payload;
        // increment extension offset
        gamestate->extensionOffset[col] = (gamestate->extensionOffset[col] + 1) % extension->numRows; 
      } else {
        array2d_get(candies, &payload, imgRow, col);
        Candy* toMove = (Candy*)payload;
        newColor = toMove->color;
        newType = toMove->type;
      }
      // update color and type
      array2d_get(candies, &payload, row, col);
      Candy* thisCandy = (Candy*)payload;
      thisCandy->color = newColor;
      thisCandy->type = newType;
    }
  }  
}

bool GameState_alreadyFired(GameState g, int row, int col) {  
  for(int i = 0; i < g->numFired; i++) {
     Block* blk = g->fires[i];
     if (col == blk-> col && row >= blk->row && row < blk->row + blk->height && blk->inGaps(row) == -1) {
       return true;
     }
  }
  return false;
}

void GameState_addFire(GameState g, Block* blk) {
  // loop through 
  for (int i = 0; i < g->numFired; i++) {
    if (blk->col == g->fires[i]->col) {
      g->fires[i]->merge(blk);
      // free
      delete blk;
      return;
    }
  }
  // if we made it here, this Block has not been found, so we add it!
  g->fires[g->numFired++] = blk;
}

int GameState_checkTemplates(GameState g) {
  GameState_freeBlocks(g);
  int numRows = g->candies->numRows;
  int numCols = g->candies->numCols;
  int countFires = 0;
  // new templates
  g->numFired = 0;
  bool flag = false;
  // check vFour
  for(int i = 0; i < numRows - 3; i++) {
    for (int j = 0; j < numCols; j++) {
      flag = false;
      // check all four positions aren't fired already
      for(int x = 0; x < 4; x++) {
        if(GameState_alreadyFired(g, i, j + x)) {
          flag = true;
          break;
        }
      }
      if(flag) {
        continue;
      }
      payload_t candy;
      array2d_get(g->candies, &candy, i, j);
      int this_color = ((Candy*)candy)->color;
      bool match = true;
      for (int k = 1; k < 4; k++) {
        array2d_get(g->candies, &candy, i + k, j);
        int next_color = ((Candy*)candy)->color;
        if (next_color != this_color) {
          match = false;
          break;
        }
      } 
      // if there is match
      if(match) {
        //printf("vFour found: i = %i, j = %i\n", i, j);
        // update states and score
        for (int k = 0; k < 4; k++) {
          int* state;
          array2d_get(g->states, (payload_t*) &state, i + k, j);
          if (*state != 0) {
            *state -= 1; 
            countFires++;
          }
        }
        // add to block array
        Block* block = new Block(i, j, 4, numRows);
        GameState_addFire(g, block);
      }
    }
  }
  // check hFour
  for(int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols - 3; j++) {
      flag = false;
      // check all four
      for(int x = 0; x < 4; x++) {
        if(GameState_alreadyFired(g, i, j + x)) {
          flag = true;
          break;
        }
      }
      if(flag) {
        continue;
      }
      payload_t candy;
      array2d_get(g->candies, &candy, i, j);
      int this_color = ((Candy*)candy)->color;
      bool match = true;
      for (int k = 1; k < 4; k++) {
        array2d_get(g->candies, &candy, i, j + k);
        int next_color = ((Candy*)candy)->color;
        if (next_color != this_color) {
          match = false;
          break;
        }
      }
      // if there is match
      if(match) {
        //printf("hFour found: i = %i, j = %i\n", i, j);
        // update states and score
        for (int k = 0; k < 4; k++) {
          int* state;
          array2d_get(g->states, (payload_t*) &state, i, j + k);
          if (*state != 0) {
            *state -= 1;
            countFires++;
          }
          // add to block array
          Block* block = new Block(i, j + k, 1, numRows);
          GameState_addFire(g, block);
        }
      }
    }
  }
  // check vThree
  for(int i = 0; i < numRows - 2; i++) {
    for (int j = 0; j < numCols; j++) {
      flag = false;
      for(int x = 0; x < 3; x++) {
        if(GameState_alreadyFired(g, i + x, j)) {
          flag = true;
          break;
        }
      }
      if(flag) {
        continue;
      }
      payload_t candy;
      array2d_get(g->candies, &candy, i, j);
      int this_color = ((Candy*)candy)->color;
      bool match = true;
      for (int k = 1; k < 3; k++) {
        array2d_get(g->candies, &candy, i + k, j);
        int next_color = ((Candy*)candy)->color;
        if (next_color != this_color) {
          match = false;
          break;
        }
      }
      // if there is match
      if(match) {
        //printf("vThree found: i = %i, j = %i\n", i, j);
        // update states and score
        for (int k = 0; k < 3; k++) {
          int* state;
          array2d_get(g->states, (payload_t*) &state, i + k, j);
          if (*state != 0) {
            *state -= 1;
            countFires++;
          }
        }
        // add to block array
        Block* block = new Block(i, j, 3, numRows);
        GameState_addFire(g, block);
      }
    }
  }
  // check hThree
  for(int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols - 2; j++) {
      flag = false;
      for(int x = 0; x < 3; x++) {
        if(GameState_alreadyFired(g, i, j + x)) {
          flag = true;
          break;
        }
      }
      if(flag) {
        continue;
      }
      payload_t candy;
      array2d_get(g->candies, &candy, i, j);
      int this_color = ((Candy*)candy)->color;
      bool match = true;
      for (int k = 1; k < 3; k++) {
        array2d_get(g->candies, &candy, i, j + k);
        int next_color = ((Candy*)candy)->color;
        if (next_color != this_color) {
          match = false;
          break;
        }
      }

      // if there is match
      if(match) {
        //printf("hThree found: i = %i, j = %i\n", i, j);
        // update states and score
        for (int k = 0; k < 3; k++) {
          int* state;
          array2d_get(g->states, (payload_t*) &state, i, j + k);
          if (*state != 0) {
            *state -= 1;
            countFires++;
          }
          // add to block array
          Block* block = new Block(i, j + k, 1, numRows);
          GameState_addFire(g, block);
        }
      }
    }
  }
  return countFires;
}

json_t* GameState_serialize(GameState g) {
  json_t* candy_array = json_array();
  json_t* states_array = json_array();
  json_t* extension_offset_array = json_array();

  for(int i = 0; i < g->candies->numRows; i++) {
    for(int j = 0; j < g->candies->numCols; j++) {
      //candy
      payload_t payload;
      array2d_get(g->candies, &payload, i, j);
      Candy* candy = (Candy*)payload;
      json_t* json_candy = json_pack("{sisi}", "color", candy->color, "type", candy->type);
      // decref
      json_array_append(candy_array, json_candy);
      json_decref(json_candy);
      // state
      payload_t state;
      array2d_get(g->states, &state, i, j);
      json_t* json_state = json_integer(*(int*)state);
      json_array_append(states_array, json_state);
      json_decref(json_state);
      if(i == 0) {
        json_t* json_ext = json_integer(g->extensionOffset[j]);
        json_array_append(extension_offset_array, json_ext);
        json_decref(json_ext);
      }
    }
  }
  json_t* c_array = array2d_serialize(g->candies, candy_array);
  json_t* s_array = array2d_serialize(g->states, states_array);

  // pack everything
  json_t* gamestate = json_pack("{sososisiso}", "boardcandies", c_array, "boardstate", s_array, 
    "movesmade", g->movesMade, "currentscore", g->score, "extensionoffset", extension_offset_array);

  return gamestate;
}

int GameState_destroy(GameState g) {
  if (g == NULL) {
    return -1;
  }
  //int numRows = g->candies->numRows;
  //int numCols = g->candies->numCols;
  // free array
  array2d_destroy(g->candies, &CandyFreeFn);
  array2d_destroy(g->states, &StateFreeFn);
  array2d_destroy(g->index_pair_arr, &IndexPairFreeFn);
  // free IndexPair
  if(g->selected != NULL) {
    free(g->selected);
    g->selected = NULL;
  }
  // free extensionOffset
  if(g->extensionOffset != NULL) {
    free(g->extensionOffset);
    g->extensionOffset = NULL;
  }
  // free fire
  GameState_freeBlocks(g);
  if(g->fires != NULL) {
    free(g->fires);
    g->fires = NULL; 
  }
  // free the overall struct
  free(g); 
  return 0;
}
