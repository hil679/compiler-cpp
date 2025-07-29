#include <iostream>
using namespace std;

void traceMalloc(void *addr, size_t size) {

   cout << "[Memory Allocation] Addr: " << addr << " Size: " << size;
}

void traceLoad(void *addr, size_t size) {
    cout << "[Load] Addr: " << addr << "Size: " << size;
}

void traceStore(void *addr, size_t size) {

    cout << "[Store] Addr: " << addr << "Size: " << size;
}
