#include <gtk/gtk.h>

static void on_activate(GtkApplication *app, gpointer ud) {
    (void)ud;
    GtkWidget *w = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(w), "GTK4 Test");
    gtk_window_set_default_size(GTK_WINDOW(w), 400, 200);
    gtk_window_present(GTK_WINDOW(w));
}

int main(int argc, char *argv[]) {
    g_setenv("GSK_RENDERER", "cairo", FALSE);
    g_setenv("GDK_BACKEND",  "win32", FALSE);
    GtkApplication *app = gtk_application_new(
        "com.test.minimal", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int r = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return r;
}
