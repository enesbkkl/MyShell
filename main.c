#include "model.h"
#include "controller.h"
#include "view.h"

void global_cleanup() {
    if (getpid() == main_pid) {
        ShmBuf *shm = buf_init();
        if (shm) buf_cleanup(shm);
    }
}

int main(int argc, char **argv) {
    main_pid = getpid();  // main_pid ataması
    atexit(global_cleanup); // Program kapanırken temizlik
    
    if (!gtk_init_check(&argc, &argv)) {
        g_error("GTK init failed!");
        return 1;
    }
    
    init_view(argc, argv);
    gtk_main();

    // Program sonunda shared memory'yi temizle
    ShmBuf *shm = buf_init();
    if (shm) {
        buf_cleanup(shm);
    }
    
    return 0;
}

