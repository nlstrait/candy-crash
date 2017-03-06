#include "model.h"
#include <iostream>

using namespace std;

string helloMsg = "{\"action\": \"hello\"}";


Model::Model(json_t* json) {
  // decode the json file to gamedef and gamestate
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




// -----------------------------------------------------------


void Model::usage(const char *exeName) {
  cout << "Usage: " << exeName << " host port" << endl;
  cout << "  Connect to hoest::port then send data and read replies" << endl;
  exit(1);
}


string Model::getMsg(hw5_net::ClientSocket* clientSocket) {
  char buf;
  string msg = "";
  int numBraces = 0;
  do {
    int readCount = 0;
    while (readCount == 0) {
      readCount = clientSocket->WrappedRead(&buf, 1);
    }
    // SANITY CHECK
    if (numBraces == 0 && buf == '}') {
      cout << "Encountered unexpected \'}\'" << endl;
    }
    msg += buf;
    // keep track of braces
    if (buf == '{') {
      numBraces++;
    } else if (buf == '}') {
      numBraces--;
    }
  } while (numBraces != 0);
  cout << "Model got message: " + msg << endl;
  return msg;
}


void Model::sendUpdate(hw5_net::ClientSocket* clientSocket) {
  json_t* json_gamestate = GameState_serialize(gameState);
  json_t* json_gamedef = GameDef_serialize(gameDef);
  // JSON LIBRARY ASKS US TO FREE THE RETURN VALUE OF json_dumps();
  char* gamedef_str = json_dumps(json_gamedef, 0);
  char* gamestate_str = json_dumps(json_gamestate, 0); 
  string updateMsg = "{\"action\": \"update\", \"gameinstance\": {\"gamedef\": "
                      + string(gamedef_str) + ", \"gamestate\": "
                      + string(gamestate_str) + "}}";
  cout << "sending update: " + updateMsg << endl;
  clientSocket->WrappedWrite(updateMsg.c_str(), updateMsg.length());
  // decref
  json_decref(json_gamestate);
  // free the return values
  free(gamedef_str);
  free(gamestate_str);
}


string Model::getAction(json_t* json) {
  json_t* json_action;
  json_unpack(json, "{s:o}", "action", &json_action);
  string str = string(json_string_value(json_action));
  cout << "Get action: " + str << endl;
  return str;
}


void Model::makeMove(json_t* json) {
  int row, col, dir;
  json_unpack(json, "{s:i,s:i,s:i}", "row", &row, "column", &col, "direction", &dir);
  IndexPair selected = IndexPair_new(row, col);

  int otherRow = row;
  int otherCol = col;
  switch(dir) {
    case 0: 
      otherCol--;
      break;
    case 1: 
      otherCol++;
      break;
    case 2: 
      otherRow++;
      break;
    case 3: 
      otherRow--;
      break;
  }
  IndexPair other = IndexPair_new(otherRow, otherCol);
  gameState->score += GameState_swap(gameState, gameDef, selected, other);

  int temp = GameState_checkTemplates(gameState);
  while(GameState_templateFound(gameState)) {
    gameState->score += temp;
    GameState_inflictGravity(gameState, gameDef);
    temp = GameState_checkTemplates(gameState);
  }

  // free idx pairs
  free(selected);
  free(other);
}


int main(int argc, char **argv) {
  
  // ensure correct number of arguments
  if (argc != 3) Model::usage(argv[0]);

  // get serverPort
  int serverPort;
  try {
    serverPort = stoi(argv[2]);
  } catch (...) {
    Model::usage(argv[0]);
  }

  try {
    // create client socket and send hello
    string serverName(argv[1]);
    //hw5_net::ClientSocket clientSocket(serverName, serverPort);
    hw5_net::ClientSocket* clientSocket = new hw5_net::ClientSocket(serverName, serverPort);
    cout << helloMsg << " sent" << endl;
    clientSocket->WrappedWrite(helloMsg.c_str(), helloMsg.length());

    // get message from server and create json
    string msg = Model::getMsg(clientSocket);
    json_error_t error;
    json_t* json = json_loads(msg.c_str(), 0, &error);

    // get action and check that it's what we expect
    string action = Model::getAction(json);
    if (action != "helloack") {
      cout << "Recieved unexpected message - expected \"helloack\"" << endl;
      return EXIT_FAILURE;
    }

    // initialize model
    json_t* json_instance;
    json_unpack(json, "{s:o}", "gameinstance", &json_instance);
    Model model(json_instance);

    // make string from gamestate json and send update
    model.sendUpdate(clientSocket);

    // get first move message, make the first move
    msg = Model::getMsg(clientSocket);
    json_t* json_move = json_loads(msg.c_str(), 0, &error);
    action = Model::getAction(json_move);

    if(action == "bye") {
      json_decref(json);
      json_decref(json_move);
      delete clientSocket;
      cout << "Exiting..." << endl;
      return EXIT_SUCCESS;
    }

    model.makeMove(json_move);
    model.sendUpdate(clientSocket);
    json_decref(json_move);

    json_t* temp = NULL;
    while (true) {
      // get next message
      msg = Model::getMsg(clientSocket);
      temp = json_loads(msg.c_str(), 0, &error);
      action = Model::getAction(temp);
      if(action != "move") {
        break;
      }
      model.makeMove(temp);
      model.sendUpdate(clientSocket);
      json_decref(temp);
    } 
    if(temp->refcount == 1) {
      json_decref(temp);
    }

    json_decref(json);
     
    delete clientSocket;

  } catch (string errString) {
    cerr << errString << endl;
    return EXIT_FAILURE;
  }

  cout << "Exiting..." << endl;

  return EXIT_SUCCESS;
}
