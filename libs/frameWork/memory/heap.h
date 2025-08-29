//
// Created by dioguabo-rei-delas on 8/17/25.
//

#ifndef CINDRA_HEAP_H
#define CINDRA_HEAP_H
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdlib.h>

constexpr size_t mBlockSIZE = 1024; //bytes
constexpr size_t mBlockCount = 1024 * 16;

inline struct Block {
private:
    void* memory;
    bool isFree = false;
    size_t offset = 0;
    explicit Block(void* data): memory(data), isFree(data ? true:false) {}
};

inline struct MemoryManagement {
private:
    void** memory;
    const size_t space = mBlockSIZE * mBlockCount;
    Block allocated[1024];
public:
    MemoryManagement():
    memory(static_cast<void**>(malloc(space))) {
        for ( int i = 0; i < mBlockCount; i++ ) {}
            allocated[i] = this->memory[i];
    }
}mManagement;



inline void MemoRyAlloc(size_t size, size_t aligment = alignof(max_align_t)) {

}

#endif //CINDRA_HEAP_H