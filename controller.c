#include "controller.h"
#include "model.h"
#include "view.h" // GUI güncellemeleri için

void handle_command(const char *input) {
    if (strncmp(input, "@msg ", 5) == 0) {
        send_message(input + 5); // Model katmanına iletilir
    } else {
        execute_command(input); // Model katmanı
    }
}

// Shared memory'den mesajları okuyup View'i günceller
void update_message_display() {
    char *msg = read_messages();
    if (msg && msg[0] != '\0') {
        append_to_view(msg); // View katmanı fonksiyonu (sonra tanımlanacak)
    }
    free(msg);
}