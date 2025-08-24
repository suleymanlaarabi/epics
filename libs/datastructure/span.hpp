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

#endif
