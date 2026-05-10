#include <gtk/gtk.h>
#include "app.h"
#include "main_window.h"
#include "fileio.h"

static AppState g_state;

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    app_state_init(&g_state, app);
    main_window_build(&g_state);
}

static void on_shutdown(GtkApplication *app, gpointer user_data) {
    (void)app;
    (void)user_data;
    app_state_free(&g_state);
}

int main(int argc, char *argv[]) {
    /* GTK4 4.x на Windows по умолчанию использует D3D12/Vulkan рендерер,
       который при инициализации может вызвать stack overflow.
       Принудительно используем Cairo (стабильный CPU-рендерер). */
    g_setenv("GSK_RENDERER",  "cairo",  FALSE);
    g_setenv("GDK_BACKEND",   "win32",  FALSE);

    GtkApplication *app = gtk_application_new(
        "com.inventory.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate",  G_CALLBACK(on_activate),  NULL);
    g_signal_connect(app, "shutdown",  G_CALLBACK(on_shutdown),   NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
