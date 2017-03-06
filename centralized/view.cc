extern "C" {
  #include "GameState.h"
}
#include <iostream>
#include "view.h"
#include "Adapter.h"

using namespace std;

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
View::View(const char *str) {
  // model
  model = new Model(str);
}

// destructor
View::~View() {
  delete model;
  if(wrappers != NULL) {
     array2d_destroy(wrappers, &View::WrapperFreeFn);
  }
}

void View::selectCandy(GtkButton *button, gpointer data) { 
  IndexPair pb = (IndexPair)data;
  GameState g = model->gameState;
  if(g->selected != NULL) {
    GtkWidget *b = gtk_grid_get_child_at((GtkGrid*)grid, g->selected->col, g->candies->numRows - 1 - (g->selected->row));
    gtk_button_set_relief((GtkButton*)b, GTK_RELIEF_NONE);
  }
  gtk_button_set_relief(button, GTK_RELIEF_NORMAL); // show border
  // changed!
  GameState_selected(g, pb);
}

// Returns true is player has selected a candy.  Returns false otherwise
bool View::candyIsSelected() {
  return model->gameState->selected != NULL;
}

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
}

// Handles the swapping of two candies
void View::swap(IndexPair swapWith) {
  GameState g = model->gameState;
  GameDef d = model->gameDef;
  IndexPair selected = g->selected;

  int scores = GameState_swap(g, d, selected, swapWith);

  if(scores == -1) {
    std::cout << "No templates made!" << std::endl;
    return;
  } // if code is not -1, it's the score count;
  
  //swap button images
  GtkButton *b1 = (GtkButton*)gtk_grid_get_child_at((GtkGrid*)grid, selected->col,
    g->candies->numRows - 1 - selected->row);
  GtkButton *b2 = (GtkButton*)gtk_grid_get_child_at((GtkGrid*)grid, swapWith->col, 
    g->candies->numRows - 1 - swapWith->row);
  payload_t payload;
  array2d_get(g->candies, &payload, selected->row, selected->col);
  // get right state
  int *state;
  array2d_get(g->states, (payload_t*) &state, selected->row, selected->col); 
  gtk_button_set_image(b1, gtk_image_new_from_file(candy_imagepath[(*(Candy*)payload).color + (*state) * 6]));
  // get right state
  array2d_get(g->states, (payload_t*) &state, swapWith->row, swapWith->col); 
  array2d_get(g->candies, &payload, swapWith->row, swapWith->col);
  gtk_button_set_image(b2, gtk_image_new_from_file(candy_imagepath[(*(Candy*)payload).color + (*state) * 6]));

  while(true) {
    int temp = GameState_checkTemplates(g);
    if(!GameState_templateFound(g)) {
      break;
    }
    scores += temp;
    GameState_inflictGravity(g, d);
  }
    
  // update board one more time
  updateBoard(g);

  // set selected to NULL and hide the border
  GtkWidget *b = gtk_grid_get_child_at((GtkGrid*)grid, g->selected->col,
    g->candies->numRows - 1 - (g->selected->row));
  gtk_button_set_relief((GtkButton*)b, GTK_RELIEF_NONE);

  // update the number of moves made
  g->movesMade += 1;
  std::string str = std::to_string(g->movesMade);
  std::string msg = str + " moves used";
  gtk_label_set_text((GtkLabel*)label, msg.c_str());

  g->score += scores;
  std::string str2 = std::to_string(g->score);
  std::string msg2 = "Score: " + str2;
  gtk_label_set_text((GtkLabel*)score, msg2.c_str());
  
  // free the selected field
  free(g->selected);
  g->selected = NULL;
}

void View::controlButtonClicked(GtkButton *button, gpointer data, char direction) {
  GameState g = model->gameState;
  if (candyIsSelected()) {
    int selRow = g->selected->row;
    int selCol = g->selected->col;
    payload_t payload;
    int code = 0;
    switch (direction) {
      case 'L':
        code = array2d_get(g->index_pair_arr, &payload, selRow, selCol - 1);
        break;
      case 'R':
        code = array2d_get(g->index_pair_arr, &payload, selRow, selCol + 1);
	      break;
      case 'U':
        code = array2d_get(g->index_pair_arr, &payload, selRow + 1, selCol);    
	      break;
      case 'D':
        code = array2d_get(g->index_pair_arr, &payload, selRow - 1, selCol);    
	      break;
    }
    if (!code) {
      swap((IndexPair)payload);
    } else {
      printf("Cannot swap to that direction!\n");
    }
  } else {
    printf("No candy selected!\n");
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

void View::activate(GtkApplication *app, gpointer data) {
  
  View *view = (View *) data;
  
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *boxLabelAndButton;  

  GameState g = model->gameState;
 
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
  //std::cout << "made wrappers" << std::endl;
  // add candies
  for(int i = 0; i < g->candies->numRows; i++) {
    for(int j = 0; j < g->candies->numCols; j++) {
      button = gtk_button_new();
      // set image
      payload_t payload;
      array2d_get(g->candies, &payload, i, j);
      int color = (*(Candy*)payload).color;
      if(color >= model->gameDef->colors) {
        std::cout << "The number of candies doesn't match! Exit Game!" << std::endl;
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
      w->view = view;
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
  score = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), score);

  str = std::to_string(g->score);
  msg = "Score: " + str;
  gtk_label_set_text((GtkLabel*)score, msg.c_str());

  // "up" button
  button = gtk_button_new();
  gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(button_imagepath[2]));
  g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_upButtonClicked), view);
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), button);
  // "down" button
  button = gtk_button_new();
  gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(button_imagepath[3]));
  g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_downButtonClicked), view);
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), button);
  // "left" button
  button = gtk_button_new();
  gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(button_imagepath[0]));
  g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_leftButtonClicked), view);
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), button);
  // "right" button
  button = gtk_button_new();
  gtk_button_set_image((GtkButton*)button, gtk_image_new_from_file(button_imagepath[1]));
  g_signal_connect(button, "clicked", G_CALLBACK(Adapter::_rightButtonClicked), view);
  gtk_container_add(GTK_CONTAINER(boxLabelAndButton), button);

  // show all widgets
  gtk_widget_show_all(window);
}

void View::open(GtkApplication *app, gpointer files, gint n_files, gchar *hint, gpointer data) {
  activate(app, data);
}

void View::WrapperFreeFn(payload_t ptr) {
   if(ptr != NULL) {
     free(ptr);
     ptr = NULL;
   }
}

int View::window(int argc, char **argv) {
  // create view
  GtkApplication *app;
  int status;
  // fetch from JSON
  // create array2d for holding IndexPair references
  // pass in new gamestate to activate
  app = gtk_application_new("com.starit_jiang.candycrush", G_APPLICATION_HANDLES_OPEN);
  // activate and handle the "open" signal
  g_signal_connect(app, "activate", G_CALLBACK(Adapter::_activate), this);
  g_signal_connect(app, "open", G_CALLBACK(Adapter::_open), this);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  // save game
  model->saveGame("gameOut.json");
  g_object_unref(app);
  return status;
}

int main(int argc, char **argv) {
  View *view = new View(argv[1]);
  return view->window(argc, argv);
}
