#ifndef SPARSESET_H
    #define SPARSESET_H

    #include <span>
    #include <vector>
    #include <cstddef>
    #include <cstdint>
    #include <unordered_map>
    #include <memory>
    #include <limits>
    #include <algorithm>

template<typename Component, typename IdT = uint64_t, unsigned PageBits = 16>
class SparseSet {
    public:
        using u64 = IdT;
        static_assert(PageBits > 0 && PageBits < (sizeof(IdT)*8), "PageBits out of range");

    private:
        static constexpr std::size_t PAGE_SIZE = std::size_t(1) << PageBits;
        static constexpr IdT PAGE_MASK = static_cast<IdT>(PAGE_SIZE - 1);
        static constexpr uint32_t EMPTY = std::numeric_limits<uint32_t>::max();

        struct Page {
            std::unique_ptr<uint32_t[]> indices;
            Page() : indices(std::make_unique<uint32_t[]>(PAGE_SIZE)) {
                std::fill_n(indices.get(), PAGE_SIZE, EMPTY);
            }
        };

        std::unordered_map<IdT, std::unique_ptr<Page>> pages;
        std::vector<Component> dense;

        Page* ensure_page(IdT page_id, bool create) {
            auto it = pages.find(page_id);
            if (it != pages.end()) return it->second.get();
            if (!create) return nullptr;
            auto p = std::make_unique<Page>();
            Page* ptr = p.get();
            pages.emplace(page_id, std::move(p));
            return ptr;
        }

        uint32_t index_for(IdT id) const {
            auto it = pages.find(id >> PageBits);
            if (it == pages.end()) return EMPTY;
            return it->second->indices[static_cast<std::size_t>(id & PAGE_MASK)];
        }

    public:
        inline void set(u64 id, const Component& value) {
            uint32_t existing = index_for(id);
            if (existing != EMPTY) {
                dense[existing] = value;
                return;
            }

            IdT page_id = id >> PageBits;
            std::size_t offset = static_cast<std::size_t>(id & PAGE_MASK);

            Page* page = ensure_page(page_id, true);
            uint32_t idx = static_cast<uint32_t>(dense.size());
            page->indices[offset] = idx;
            dense.push_back(value);
        }

        inline Component& get(u64 id) {
            uint32_t idx = index_for(id);
            return dense[idx];
        }

        inline Component& getOrAdd(u64 id) {
            uint32_t existing = index_for(id);
            if (existing != EMPTY) return dense[existing];
            Component value{};
            set(id, value);
            return get(id);
        }

        inline bool has(u64 id) const {
            return index_for(id) != EMPTY;
        }

        inline std::span<Component> values() {
            return { dense.data(), dense.size() };
        }

        inline size_t len() const {
            return dense.size();
        }
};

#endif
