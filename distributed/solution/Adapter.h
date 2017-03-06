#ifndef __ADAPTER_H
#define __ADAPTER_H

extern "C" {
  #include <gtk/gtk.h>
}

class Adapter {
  public:
    static void _activate(GtkApplication *app, gpointer data);
    static void _open(GtkApplication *app, gpointer files, gint n_files, gchar *hint, gpointer data);
    static void _selectCandy(GtkButton *button, gpointer data);
    static void _leftButtonClicked(GtkButton *button, gpointer data);
    static void _rightButtonClicked(GtkButton *button, gpointer data);
    static void _upButtonClicked(GtkButton *button, gpointer data);
    static void _downButtonClicked(GtkButton *button, gpointer data);
};

#endif // __ADAPTER_H
