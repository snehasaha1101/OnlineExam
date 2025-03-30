#include "server.h"

int main() {
    Server server(8080);  // Start server on port 8080
    server.start();
    return 0;
}
