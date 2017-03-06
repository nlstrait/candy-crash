#include "model.h"

using namespace std;

Model::Model(const char *str) {
  // decode the json file to gamedef and gamestate
  json_error_t error;
  json = json_load_file(str, 0, &error);
  json_t* gamedef;
  json_t* gamestate;
  if(json_object_size(json) == 2) {
    // unpack file
    json_unpack(json, "{s:o, s:o}", "gamedef", &gamedef, "gamestate", &gamestate);
    // create gamedef, gamestate
    gameDef = GameDef_new(gamedef);
    gameState = GameState_new(gamestate);
  } else {
    // unpack file
    json_unpack(json, "{s:o}", "gamedef", &gamedef);
    gameDef = GameDef_new(gamedef);
    gameState = GameState_new_from_GameDef(gameDef);
  }
} 

// destructor
Model::~Model() {
  // decref json
  json_decref(json);
  // destroy
  GameDef_destroy(gameDef);
  GameState_destroy(gameState);
}

void Model::saveGame(const char* filepath) {
  json_t* json_gamedef = GameDef_serialize(gameDef);
  json_t* json_gamestate = GameState_serialize(gameState);

  json_t* game = json_pack("{soso}", "gamedef", json_gamedef, "gamestate", json_gamestate);
  json_dump_file(game, filepath, 0);
  json_decref(json_gamedef);
  json_decref(json_gamestate);
  json_decref(game);
}
