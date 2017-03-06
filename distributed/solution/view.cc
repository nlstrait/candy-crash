extern "C" {
  #include "GameState.h"
}
#include <iostream>
#include <fstream>
#include <sstream>
#include "view.h"
#include "Adapter.h"
#include <cstdlib>

using namespace std;

// USAGE HELPER
void usage(const char *exeName) {
  cout << "Usage: " << exeName << " [json file path]" << endl;
  exit(1);
}
// HELPER FREE FN
void View::WrapperFreeFn(payload_t ptr) {
   if(ptr != NULL) {
     free(ptr);
     ptr = NULL;
   }
}
// HELPER PRINT FN
void printInt(payload_t payload) {
  int num = *(int*)payload;
  printf("%i", num);
}

// all image paths
static char *candy_imagepath[18] = {
  (char*)"images/regular/state0/blue.png", (char*)"images/regular/state0/green.png", (char*)"images/regular/state0/purple.png",
  (char*)"images/regular/state0/orange.png", (char*)"images/regular/state0/red.png", (char*)"images/regular/state0/yellow.png",
  (char*)"images/regular/state1/blue.png", (char*)"images/regular/state1/green.png", (char*)"images/regular/state1/purple.png",
  (char*)"images/regular/state1/orange.png", (char*)"images/regular/state1/red.png", (char*)"images/regular/state1/yellow.png",
  (char*)"images/regular/state2/blue.png", (char*)"images/regular/state2/green.png", (char*)"images/regular/state2/purple.png",
  (char*)"images/regular/state2/orange.png", (char*)"images/regular/state2/red.png", (char*)"images/regular/state2/yellow.png",
};
static char *button_imagepath[4] = {
  (char*)"images/direction/left.png", (char*)"images/direction/right.png", (char*)"images/direction/up.png", 
  (char*)"images/direction/down.png"
};

// constructor
View::View() {
  selected = NULL;
  sock = NULL;
  peerSocket = NULL;
  received_message = "";
  numRows = 0;
  numCols = 0;
}

// destructor
View::~View() {
  if(selected != NULL) {
    free(selected);
    selected = NULL;
  }
  if(wrappers != NULL) {
     array2d_destroy(wrappers, &View::WrapperFreeFn);
  }
}

//-------------------------------CANDY SWAP----------------------------//

// Returns true is player has selected a candy.  Returns false otherwise
bool View::candyIsSelected() {
  return selected != NULL;
}

bool View::validSwap(int toRow, int toCol) {
  cout << (toRow >= 0 && toRow < numRows && toCol >=0 && toCol < numCols) << endl;
  return toRow >= 0 && toRow < numRows && toCol >=0 && toCol < numCols; 
}

string View::buildMoveMsg(int selRow, int selCol, int selDir) {
  string msg("{\"action\": \"move\", \"row\": ");
  msg += std::to_string(selRow);
  msg += ", \"column\": ";
  msg += std::to_string(selCol);
  msg += ", \"direction\": ";
  msg += std::to_string(selDir);
  msg += "}";
  cout << "Send mesg" << msg;
  return msg;
}

void View::selectCandy(GtkButton *button, gpointer data) {
  IndexPair ip = (IndexPair)data;
  // set the original selected candy to hide the border
  if(selected != NULL) {
    GtkWidget *b = gtk_grid_get_child_at((GtkGrid*)grid, selected->col, numRows - 1 - (selected->row));
    gtk_button_set_relief((GtkButton*)b, GTK_RELIEF_NONE);
  }
  // show the border of new selected candy
  gtk_button_set_relief(button, GTK_RELIEF_NORMAL); 
  if(selected == NULL) {
    selected = (IndexPair)malloc(sizeof(IndexPair_st));
  }
  selected->row = ip->row;  
  selected->col = ip->col;
}

void View::controlButtonClicked(GtkButton *button, gpointer data, char direction) {
  //GameState g = model->gameState;
  if (candyIsSelected()) {
    int selRow = selected->row;
    int selCol = selected->col;

    int toRow = selRow;
    int toCol = selCol;
    bool valid = false;
    int selDir = 0;
    switch (direction) {
      case 'L':
        toCol -= 1;  
        valid = validSwap(selRow, selCol - 1);
        selDir = 0;
        break;
      case 'R':
        toCol += 1;
        valid = validSwap(selRow, selCol + 1);
        selDir = 1;
	      break;
      case 'U':
        toRow += 1;
        valid = validSwap(selRow + 1, selCol);
        selDir = 2;
	      break;
      case 'D':
        toRow -= 1;
        valid = validSwap(selRow - 1, selCol);
        selDir = 3;
	      break;
    }
    if (valid) {
      // WE ARE READY TO SEND THE MESSAGE
      string msg = buildMoveMsg(selRow, selCol, selDir);
      // WE SHOULD HAVE A NON-NULL PEERSOCKET WHEN WE SWAP
      if(peerSocket == NULL) {
        cerr << "This should never happen!!!" << endl;
        exit(1);
      }
      peerSocket->WrappedWrite(msg.c_str(), msg.length());
      // read after writing the move
      socketReadJson();
      // update the board
      update();
    } else {
      cout << "Cannot swap to that direction!" << endl;
    }
  } else {
    cout << "No candy selected! Cannot move!" << endl;
  }
}

void View::leftButtonClicked(GtkButton *button, gpointer data) {
  controlButtonClicked(button, data, 'L');
}

void View::rightButtonClicked(GtkButton *button, gpointer data) {
  controlButtonClicked(button, data, 'R');
}

void View::upButtonClicked(GtkButton *button, gpointer data) {
  controlButtonClicked(button, data, 'U');
}

void View::downButtonClicked(GtkButton *button, gpointer data) {
  controlButtonClicked(button, data, 'D');
}

//--------------------------------UPDATE WINDOW--------------------------------------//
void View::updateBoard(GameState g) {
  for(int row = 0; row < g->candies->numRows; row++) {
    for(int column = 0; column < g->candies->numCols; column++) {
      GtkButton *button = (GtkButton*)gtk_grid_get_child_at((GtkGrid*)grid, column, g->candies->numRows - 1- row);
      payload_t payload;
      array2d_get(g->candies, &payload, row, column);
      // get right state
      int *state;
      array2d_get(g->states, (payload_t*) &state, row, column);
      gtk_button_set_image(button, gtk_image_new_from_file(candy_imagepath[(*(Candy*)payload).color + (*state) * 6]));
    }
  }
  // update moves
  std::string str = std::to_string(g->movesMade);
  std::string msg = str + " moves used";
  gtk_label_set_text((GtkLabel*)label, msg.c_str());

  // update score
  std::string str2 = std::to_string(g->score);
  std::string msg2 = "Score: " + str2;
  gtk_label_set_text((GtkLabel*)score, msg2.c_str());

  // remove the border of the selected candy
  if(selected != NULL) {
    GtkWidget *b = gtk_grid_get_child_at((GtkGrid*)grid, selected->col, numRows - 1 - (selected->row));
    gtk_button_set_relief((GtkButton*)b, GTK_RELIEF_NONE);
    if(selected != NULL) {
      free(selected);
      selected = NULL;
    }
  }
  // we destroy this used GameState in update()
}

void View::update() {
  // get game state from string
  GameState g = parseMsg_state(received_message);
  // update board
  updateBoard(g);
  // destroy
  GameState_destroy(g);
}

//-------------------------------SEND/RECEIVE MESSAGE---------------------------------//
// DECODE THE RECIEVED MESSAGE
int View::decode(string json_message) {
  // create json from string
  json_error_t error;
  json_t* json = json_loads(json_message.c_str(), 0, &error);
  // get the action
  json_t* json_action;
  json_unpack(json, "{s:s}", "action", &json_action);
  std::string action((char*)json_action);
  // decref
  //json_decref(json);
  // what is the action
  if(action == "hello") {
    return 0;
  } else if (action == "update") {
    return 1;
  } else {
    cout << action << endl;
    cout << "This should never happen!!! View decode is neither 0 nor 1. Check View code!" << endl;
    exit(1);
  }
}

// PARSE THE RECIEVED MESSAGE AND CREATE A GAMESTATE
GameState View::parseMsg_state(string msg) {
  json_error_t error;
  json_t* json = json_loads(msg.c_str(), 0, &error);
  json_t* gameinstance, *action;
  json_t* gamedef, *gamestate;
  // get game instance;
  json_unpack(json, "{s:s, s:o}", "action", &action, "gameinstance", &gameinstance);
  // get game state
  json_unpack(gameinstance, "{s:o, s:o}", "gamedef", &gamedef, "gamestate", &gamestate);
  // create game state
  GameState g = GameState_new(gamestate);
  // decref
  //json_decref(json);
  return g;
}

// PARSE THE RECIEVED MESSAGE AND CREATE A GAMEDEF
GameDef View::parseMsg_def(string msg) {
  json_error_t error;
  json_t* json = json_loads(msg.c_str(), 0, &error);
  json_t* gameinstance, action;
  json_t* gamedef, gamestate;
  // get game instance;
  json_unpack(json, "{s:s, s:o}", "action", &action, "gameinstance", &gameinstance);
  // get game state
  json_unpack(gameinstance, "{s:o, s:o}", "gamedef", &gamedef, "gamestate", &gamestate);
  // create game state
  GameDef d = GameDef_new(gamedef);
  return d;
}

// HELLO ACKNOWLEDGEMENT MESSAGE
std::string View::helloAck(const char *filepath) {
  // header 
  string message("{\"action\": \"helloack\", \"gameinstance\": ");

  // load gamedef:
  json_error_t error;
  json_t* json = json_load_file(filepath, 0, &error);
  // unpack to get gamedef 
  json_t* gamedef;
  json_unpack(json, "{s:o}", "gamedef", &gamedef);
  cout << message << endl;
  GameDef d = GameDef_new(gamedef);
  GameState g = GameState_new_from_GameDef(d);
  json_t* gamestate = GameState_serialize(g);
  // wrap def and state
  json_t* all = json_pack("{soso}", "gamedef", gamedef, "gamestate", gamestate);
  string addon = json_dumps(all, 0);
  // concat
  message += addon;
  message += "}";
  // return 
  return message;
}

//-----------------------------START THE GAME WINDOW------------------------------//
void View::activate(GtkApplication *app, gpointer data) {

  // get the message 
  const char *message = (const char*)data;
  cout << message << endl;
  // create gamestate
  GameState g = parseMsg_state(string(message));
  GameDef d = parseMsg_def(string(message));

  numRows = g->candies->numRows;
  numCols = g->candies->numCols;

  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *boxLabelAndButton;  
  
  cout << "before window initialization" << endl;

  // window and title
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "HW4");

  // big container, spacing between children is 20 pixels
  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  // add to window
  gtk_container_add(GTK_CONTAINER(window), box);

  // add grid to box
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(box), grid);

  // wrapper array
  wrappers = array2d_new(g->candies->numRows, g->candies->numCols);

  // add candies
  for(int i = 0; i < g->candies->numRows; i++) {
    for(int j = 0; j < g->candies->numCols; j++) {
      button = gtk_button_new();
      // set image
      payload_t payload;
      array2d_get(g->candies, &payload, i, j);
      int color = (*(Candy*)payload).color;
      if(color >= d->colors) {
        return;
      }
      // get right state;
      int *state;
      array2d_get(g->states, (payload_t*) &state, i, j);
      gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(candy_imagepath[color + (*state) * 6]));
      gtk_button_set_relief((GtkButton*)button, GTK_RELIEF_NONE); // hide border
      // listener
      array2d_get(g->index_pair_arr, &payload, i, j);
      // create wrapper
      Wrapper w = (Wrapper)malloc(sizeof(Wrapper_st));
      w->view = this;
      w->idx_pair = (IndexPair)payload;
      array2d_update(wrappers, (payload_t)w, i, j);
      g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_selectCandy), w);
      // add candy to grid
      gtk_grid_attach(GTK_GRID(grid), button, j, g->candies->numRows - 1 - i, 1, 1);
    }
  }
  // all button with same size
  gtk_grid_set_row_homogeneous((GtkGrid*)grid, TRUE);
  gtk_grid_set_column_homogeneous((GtkGrid*)grid, TRUE);
  
  // the right side of the window, spacing between each widget is 5 pixels
  boxLabelAndButton = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  // add to window box
  gtk_container_add(GTK_CONTAINER(box), boxLabelAndButton);

  // add label
  std::string str = std::to_string(g->movesMade);
  std::string msg = str + " moves made";
  label = gtk_label_new(msg.c_str());
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), label);

  // add score
  str.clear(); 
  msg.clear();
  str = std::to_string(g->score);
  msg = "Score: " + str;
  score = gtk_label_new(msg.c_str());
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), score);

  // "left" button
  button = gtk_button_new();
  gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(button_imagepath[0]));
  g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_leftButtonClicked), this);
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), button);
  // "right" button
  button = gtk_button_new();
  gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(button_imagepath[1]));
  g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_rightButtonClicked), this);
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), button);

  // "up" button
  button = gtk_button_new();
  gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(button_imagepath[2]));
  g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_upButtonClicked), this);
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), button);
  // "down" button
  button = gtk_button_new();
  gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(button_imagepath[3]));
  g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_downButtonClicked), this);
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), button);
  
  // show all widgets
  gtk_widget_show_all(window);
  
  // we do need gamestate and gamedef. stop being stupid!!!!
  //GameState_destroy(g);
  //GameDef_destroy(d);
}

void View::open(GtkApplication *app, gpointer files, gint n_files, gchar *hint, gpointer data) {
  activate(app, data);
}

int View::window(int argc, char **argv) {
  // create view
  GtkApplication *app;
  int status;
  // application
  app = gtk_application_new("com.starit_jiang.candycrush", G_APPLICATION_HANDLES_OPEN);
  // activate and handle the "open" signal
  g_signal_connect(app, "activate", G_CALLBACK(Adapter::_activate), this);
  g_signal_connect(app, "open", G_CALLBACK(Adapter::_open), this);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  // after the window is closed
  string msg = "{\"action\": \"bye\"}";
  peerSocket->WrappedWrite(msg.c_str(), msg.length());
  g_object_unref(app);
  return status; 
}

//-------------------------------------SOCKET----------------------------------------//
void View::socketReadJson() {
  char buf;
  received_message.clear();
  int numBraces = 0;
  do {
    int readCount = 0;
    while (readCount == 0) {
      readCount = peerSocket->WrappedRead(&buf, 1);
    }
    // SANITY CHECK
    if (numBraces == 0 && buf == '}') {
      cout << "Encountered unexpected \'}\'" << endl;
    }
    received_message += buf;
    // keep track of braces
    if (buf == '{') {
      numBraces++;
    } else if (buf == '}') {
      numBraces--;
    }
  } while (numBraces != 0);
  cout << "Model got message: " + received_message << endl;
}

void View::start(const char *filepath, int argc, char **argv) {
  if(argc != 2) usage(argv[0]);
  int port = 0;

  try {
    sock = new hw5_net::ServerSocket(port);
    sock->BindAndListen(AF_INET, &socketFd);
    cout << "Created bound socket. port = " << sock->port() << endl;

    // for connection
    int acceptedFd;
    string clientAddr, clientDNSName, serverAddress, serverDNSName;
    uint16_t clientPort;

    while(true) {
      sock->Accept( &acceptedFd, &clientAddr, &clientPort, &clientDNSName, &serverAddress, &serverDNSName);
      // wrap connect
      if(peerSocket != NULL) {
        delete peerSocket;
      }
      peerSocket = new hw5_net::ClientSocket(acceptedFd);
      // get hello
      socketReadJson();
      if(decode(received_message) != 0) {
        cerr << "Something is wrong! The first msg should be hello!" << endl;
        exit(1);
      }
      string msg;
      msg = helloAck(filepath); 
      peerSocket->WrappedWrite(msg.c_str(), msg.length());
      // open window
      socketReadJson();
      if(decode(received_message) != 1) {
        cerr << "Something is wrong! The second msg should be update!" << endl;
      }
      window(argc, argv);
    }

  } catch (...) {
    cerr << "EXCEPTION HAPPENED" << endl;
    return;
  }
}

//-----------------------------------MAIN--------------------------------//
int main(int argc, char **argv) {
    View *view = new View();
    // start view
    view->start(argv[1], argc, argv);
    return EXIT_SUCCESS; 
}
