#include "view.h"
#include "Adapter.h"
#include <iostream>

void Adapter::_activate(GtkApplication *app, gpointer data) {
  View *view = (View*) data;
  view->activate(app, view);
}

void Adapter::_open(GtkApplication *app, gpointer files, gint n_files, gchar *hint, gpointer data) {
  View *view = (View*) data;
  view->open(app, files, n_files, hint, data);
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
