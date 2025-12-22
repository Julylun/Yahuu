#include <stdio.h>
#include "services/userService/userService.h"
#include "services/socketService/socketService.h"

int main() {
    printf("Server starting...\n");

    // 1. Initialize data layer
    if (UserService_createDocument() != 0) {
        // fprintf(stderr, "FATAL: Could not initialize the user document. Exiting.\n");
        // return 1;
    }
    printf("User document structure is ready.\n");

    // 2. Initialize network layer
    int server_fd = Socket_init(8080, 10); // Listen on port 8080
    if (server_fd < 0) {
        fprintf(stderr, "FATAL: Could not initialize server socket. Exiting.\n");
        return 1;
    }

    // 3. Run the server (this function blocks forever)
    Socket_run(server_fd);

    // The lines below are now effectively unreachable
    printf("Server shutting down.\n");
    return 0;
}
