#ifndef SPAN_H
    #define SPAN_H
    #include <cstddef>

template<typename T>
struct Span {
    T* data;
    std::size_t size;

    inline T* begin() const { return data; }
    inline T* end()   const { return data + size; }
    inline T& operator[](std::size_t i) const { return data[i]; }
    inline bool empty() const { return size == 0; }
};


#include <tuple>

template<typename... Ts>
struct ZipSpan {
    std::tuple<Ts*...> data;
    std::size_t size;

    struct iterator {
        std::tuple<Ts*...> ptrs;

        iterator& operator++() {
            std::apply([](auto*&... ps){ ((++ps), ...); }, ptrs);
            return *this;
        }

        auto operator*() const {
            return std::apply([](auto*... ps){ return std::tie(*ps...); }, ptrs);
        }

        bool operator!=(const iterator& other) const {
            return std::get<0>(ptrs) != std::get<0>(other.ptrs);
        }
    };

    iterator begin() const { return { data }; }
    iterator end() const {
        auto endPtrs = data;
        std::apply([n=size](auto*&... ps){ ((ps += n), ...); }, endPtrs);
        return { endPtrs };
    }

    auto operator[](std::size_t i) const {
        return std::apply([i](auto*... ps){ return std::tie(ps[i]...); }, data);
    }

    bool empty() const { return size == 0; }
};


#endif
