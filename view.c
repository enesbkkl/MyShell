#include "view.h"
#include "controller.h"
#include <stdio.h>

// Global widgets
static GtkWidget *text_view = NULL;
static GtkWidget *entry = NULL;

// Forward declaration (bildirim)
static gboolean append_to_view_idle(gpointer data);

// GTK callback fonksiyonu
static void on_command_entered(GtkEntry *entry, gpointer user_data) {
    const char *command = gtk_entry_get_text(entry);
    if (command && *command) {
        handle_command(command); // Controller'a yönlendir
        gtk_entry_set_text(entry, ""); // Girişi temizle
    }
}

// View'e yazı ekleme (thread-safe)
void append_to_view(const char *text) {
    g_idle_add((GSourceFunc)append_to_view_idle, g_strdup(text));
}

// GTK main thread'de çalışacak yardımcı fonksiyon
static gboolean append_to_view_idle(gpointer data) {
    char *text = (char *)data;
    if (!text_view || !text) return G_SOURCE_REMOVE;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    
    gtk_text_buffer_insert(buffer, &end, text, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
    
    // Scroll to bottom
    GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, "end", &end, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text_view), mark, 0.0, FALSE, 0.0, 0.0);
    gtk_text_buffer_delete_mark(buffer, mark);
    
    g_free(text);
    return G_SOURCE_REMOVE;
}

// Mesajları kontrol eden timer fonksiyonu
static gboolean check_messages(gpointer user_data) {
    update_message_display(); // Controller'ı çağır
    return G_SOURCE_CONTINUE; // Sürekli çalışsın
}

// GTK arayüzünü başlatma
void init_view(int argc, char **argv) {
    gtk_init(&argc, &argv);

    // Ana pencere
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Multi-Shell");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Dikey kutu
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Terminal çıktı alanı
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    // Komut girişi
    entry = gtk_entry_new();
    g_signal_connect(entry, "activate", G_CALLBACK(on_command_entered), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    // Kontroller
    gtk_widget_grab_focus(entry);
    gtk_widget_show_all(window);

    // Mesaj kontrol timer'ı (100ms'de bir)
    g_timeout_add(100, check_messages, NULL);
    
    gtk_main();
}