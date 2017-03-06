#ifndef __VIEW_H
#define __VIEW_H

extern "C" {
  #include "GameState.h"
  #include <gtk/gtk.h>
}

#include "model.h"
#include <string>
#include "ServerSocket.h"
#include "ClientSocket.h"

class View {
  public:
    View();
    // destructor
    ~View();
    // size of the view
    int numRows;
    int numCols;
    // string holding received message
    string received_message;
    // struct to pass for adapter
    typedef struct Wrapper_st {
       View *view;
       IndexPair idx_pair;
    } * Wrapper;
    // the 2d array to keep all the wrappers
    array2d wrappers;
    // ----------public methods-----------

    // Hide the border of the selected button (if any) and show the border of
    // the newly selected candy
    //
    // Arguments:
    // -button: the newly selected candy
    // -data: an GamePlusIndex struct holding the current game state
    //
    void selectCandy(GtkButton *button, gpointer data);
    // Activate the GUI of this game. 
    //
    // Arguments:
    // -app: the GtkApplication pointer
    // -data: the current game state
    //
    void activate(GtkApplication *app, gpointer data);

    // Need this method to handle the "oepn" signal from command line to open
    // the json file
    void open(GtkApplication *app, gpointer files, gint n_files, gchar *hint, gpointer data);

    // see controlButtonClicked();
    void leftButtonClicked(GtkButton *button, gpointer data);

    // see controlButtonClicked()
    void rightButtonClicked(GtkButton *button, gpointer data);

    // see controlButtonClicked()
    void upButtonClicked(GtkButton *button, gpointer data);

    // see controlButtonClicked()
    void downButtonClicked(GtkButton *button, gpointer data);

    void start(const char *filepath, int argc, char **argv);
    
    void socketReadJson();

  private:
    // selected
    IndexPair selected;
    // socket components
    int socketFd;
    hw5_net::ServerSocket *sock;
    hw5_net::ClientSocket *peerSocket;
    // game components
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *score;
    // ----------private methods-----------

    // needed to free the wrapper
    static void WrapperFreeFn(payload_t ptr); 
    // Check if there is a candy selected right now
    //
    // Returns:
    // true if there is a candy selected now; false otherwise
    bool candyIsSelected();

    bool validSwap(int toRow, int toCol);

    
    // The method to invoke when one of the direction control button is clicked.
    // It checks if there is a candy to swap and if the candy can be swapped to 
    // the given direction. If not, a message is printed out to terminal
    //
    // Arguments:
    // -button: the direction button to register with
    // -data: the current game state
    // -direction: the direction to swap the selected candy to. 'L'--left, 'R'--right,
    //             'U'--up, 'D'--left
    //
    void controlButtonClicked(GtkButton *button, gpointer data, char direction);
    // update the board when model made changes
    //
    // -g: the current gamestate
    void updateBoard(GameState g);  

    void update();

    string buildMoveMsg(int selRow, int selCol, int selDir);

    int decode(string json_message);

    GameState parseMsg_state(string msg);

    GameDef parseMsg_def(string msg);

    string helloAck(const char *filepath);

    int window(int argc, char **argv);
   
};

#endif // _VIEW_H
