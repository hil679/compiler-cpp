#include <iostream>
using namespace std;

extern "C" {
    void traceMalloc(void *addr, size_t size) {

    cout << "[Memory Allocation(Heap)] Addr: " << addr << " Size: " << size << endl;
    }
    void traceAllocStack(void *addr, size_t size) {

    cout << "[Memory Allocation(Stack)] Addr: " << addr << " Size: " << size << endl;
    }

    void traceLoad(void *addr, size_t size) {
        cout << "[Load] Addr: " << addr << " Size: " << size << endl;
    }

    void traceStore(void *addr, size_t size) {

        cout << "[Store] Addr: " << addr << " Size: " << size << endl;
    }
}
