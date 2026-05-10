#include <gtk/gtk.h>
#include "app.h"
#include "main_window.h"
#include "fileio.h"
#ifdef _WIN32
#include <windows.h>
#endif

static AppState g_state;

// GTK "activate" signal handler — builds and shows the main window
/*
 * Called once by GtkApplication when the application is ready.
 * Initialises the global AppState and delegates window construction
 * to main_window_build.
 */
static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    app_state_init(&g_state, app);
    main_window_build(&g_state);
}

// GTK "shutdown" signal handler — releases all application resources
/*
 * Called by GtkApplication after the last window has been closed.
 * Frees the list and unrefs the GListStore held in AppState.
 */
static void on_shutdown(GtkApplication *app, gpointer user_data) {
    (void)app;
    (void)user_data;
    app_state_free(&g_state);
}

// Application entry point
/*
 * Forces GTK4 to use the Cairo renderer and Win32 backend on Windows
 * before any GTK initialisation occurs, preventing a D3D12 stack
 * overflow present in GTK4 >= 4.14 on MinGW builds.
 * Creates the GtkApplication, connects lifecycle signals, and runs
 * the GLib main loop.
 */
int main(int argc, char *argv[]) {
#ifdef _WIN32
    FreeConsole();
    SetEnvironmentVariableA("GSK_RENDERER", "cairo");
    SetEnvironmentVariableA("GDK_BACKEND",  "win32");
#endif

    GtkApplication *app = gtk_application_new(
        "com.inventory.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(app, "shutdown", G_CALLBACK(on_shutdown),  NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
