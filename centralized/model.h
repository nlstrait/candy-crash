#ifndef __MODEL_H
#define __MODEL_H

extern "C" {
  #include "GameState.h"
  #include "GameDef.h"
}

#include <string>
#include <cstring>

using namespace std;

class Model {
  public:
    // constructor
    Model(const char *filepath);
    // destructor
    ~Model();
    // game state
    GameState gameState;
    // game definition
    GameDef gameDef;
    // serialization
    void saveGame(const char* filepath);
  private:
    // the given JSON file from command line
    json_t* json;
};
#endif // _MODEL_H

