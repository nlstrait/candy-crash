#include "view.h"
#include "Adapter.h"
#include <iostream>

void Adapter::_activate(GtkApplication *app, gpointer data) {
  View *view = (View*) data;
  char *str = new char[view->received_message.length() + 1];
  strcpy(str, view->received_message.c_str());
  view->activate(app, str);
  delete str;
}

void Adapter::_open(GtkApplication *app, gpointer files, gint n_files, gchar *hint, gpointer data) {
  View *view = (View*) data;
  char *str = new char[view->received_message.length() + 1];
  strcpy(str, view->received_message.c_str());
  view->open(app, files, n_files, hint, str);
  delete str;
}

void Adapter::_selectCandy(GtkButton *button, gpointer data) {
  View::Wrapper w = (View::Wrapper) data;
  View *view = w->view;
  view->selectCandy(button, w->idx_pair);
}

void Adapter::_leftButtonClicked(GtkButton *button, gpointer data) {
  View *view = (View*) data;
  view->leftButtonClicked(button, data);
}

void Adapter::_rightButtonClicked(GtkButton *button, gpointer data) {
  View *view = (View*) data;
  view->rightButtonClicked(button, data);
}

void Adapter::_upButtonClicked(GtkButton *button, gpointer data) {
  View *view = (View*) data;
  view->upButtonClicked(button, data);
}

void Adapter::_downButtonClicked(GtkButton *button, gpointer data) {
  View *view = (View*) data;
  view->downButtonClicked(button, data);
}
