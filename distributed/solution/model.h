#ifndef __MODEL_H
#define __MODEL_H

extern "C" {
  #include "GameState.h"
  #include "GameDef.h"
}

#include <string>
#include <cstring>
#include "ClientSocket.h"

using namespace std;

class Model {
  public:
    // constructor
    Model(json_t* json);
    // destructor
    ~Model();
    // game state
    GameState gameState;
    // game definition
    GameDef gameDef;
    // serialization
    void saveGame(const char* filepath);
    // makes a move using json object
    void makeMove(json_t* json);
    // sends update message over nextwork based on current gamestate
    void sendUpdate(hw5_net::ClientSocket* clientSocket);
    // outputs message stating terms of usage
    static void usage(const char *exeName);
    // fetches message over network
    static string getMsg(hw5_net::ClientSocket* clientSocket);
    // fetches the action of the provided message
    static string getAction(json_t* json);
};
#endif // _MODEL_H

