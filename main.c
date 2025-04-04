#include "view.h"
#include "model.h"

int main(int argc, char *argv[]) {
    // Shared memory'yi ana proses yönetecek
    ShmBuf *shm = buf_init();
    if (!shm) return EXIT_FAILURE;

    init_view(argc, argv);

    // Program kapanırken temizlik
    buf_cleanup(shm);
    shm_unlink(SHARED_FILE_PATH);
    
    return EXIT_SUCCESS;
}