//
// Created by dioguabo-rei-delas on 9/2/25.
//

#ifndef LAZYANY_DYNAMICBITSET_H
#define LAZYANY_DYNAMICBITSET_H
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace lazy {
    class dynamicBitset {
    private:
        std::vector<uint64_t> blocks_;
        size_t size_;
        static constexpr size_t bits_per_block = 64;

    public:
        explicit dynamicBitset(size_t initial_size = 0) : size_(initial_size) {
            blocks_.resize((initial_size + bits_per_block - 1) / bits_per_block, 0);
        }

        [[nodiscard]] bool test(size_t pos) const {
            if (pos >= size_) return false;
            size_t block_idx = pos / bits_per_block;
            size_t bit_idx = pos % bits_per_block;
            return (blocks_[block_idx] >> bit_idx) & 1;
        }

        void set(size_t pos, bool value = true) {
            if (pos >= size_) {
                size_t new_size = pos + 1;
                size_t new_num_blocks = (new_size + bits_per_block - 1) / bits_per_block;
                if (new_num_blocks > blocks_.size()) {
                    blocks_.resize(new_num_blocks, 0);
                }
                size_ = new_size;
            }
            size_t block_idx = pos / bits_per_block;
            size_t bit_idx = pos % bits_per_block;
            if (value) {
                blocks_[block_idx] |= (1ULL << bit_idx);
            } else {
                blocks_[block_idx] &= ~(1ULL << bit_idx);
            }
        }

        void reset(size_t pos) {
            set(pos, false);
        }

        void flip(size_t pos) {
            if (pos >= size_) {
                set(pos, true); // Flip from implicit false to true
            } else {
                size_t block_idx = pos / bits_per_block;
                size_t bit_idx = pos % bits_per_block;
                blocks_[block_idx] ^= (1ULL << bit_idx);
            }
        }

        [[nodiscard]] auto size() const {
            return size_;
        }

        void set() {
            for (auto& block : blocks_) {
                block = ~0ULL;
            }
            // Clear unused bits in the last block
            if (size_ > 0) {
                size_t last_block = (size_ - 1) / bits_per_block;
                size_t bits_in_last_block = size_ % bits_per_block;
                if (bits_in_last_block != 0) {
                    uint64_t mask = (1ULL << bits_in_last_block) - 1;
                    blocks_[last_block] &= mask;
                }
            }
        }

        void reset() {
            for (auto& block : blocks_) {
                block = 0;
            }
        }

        void flip() {
            for (auto& block : blocks_) {
                block = ~block;
            }
            // Clear unused bits in the last block
            if (size_ > 0) {
                size_t last_block = (size_ - 1) / bits_per_block;
                size_t bits_in_last_block = size_ % bits_per_block;
                if (bits_in_last_block != 0) {
                    uint64_t mask = (1ULL << bits_in_last_block) - 1;
                    blocks_[last_block] &= mask;
                }
            }
        }

        void reserve(size_t new_capacity_bits) {
            size_t new_num_blocks = (new_capacity_bits + bits_per_block - 1) / bits_per_block;
            if (new_num_blocks > blocks_.size()) {
                blocks_.resize(new_num_blocks, 0);
            }
        }
    };
}
#endif //LAZYANY_DYNAMICBITSET_H