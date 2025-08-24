#ifndef ECS_HASH_H
    #define  ECS_HASH_H

    #include <bitset>
    #include <cstddef>

struct MaskHash {
    template<std::size_t N>
    std::size_t operator()(const std::bitset<N> &bs) const noexcept {
        constexpr std::size_t numWords = (N + 63) / 64;
        const auto *words = reinterpret_cast<const uint64_t*>(&bs);

        std::size_t h = 0xcbf29ce484222325ull;
        for (std::size_t i = 0; i < numWords; i++) {
            h ^= words[i];
            h *= 0x100000001b3ull;
        }
        return h;
    }
};

#endif
