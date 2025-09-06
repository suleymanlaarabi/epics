#ifndef ECS_VEC_WRAPPER_HPP
#define ECS_VEC_WRAPPER_HPP

extern "C" {
#include "ecs_vec.h"
}

#include <stdexcept>

class EcsVec {
public:
    explicit EcsVec(size_t elementSize)
        : _vec(ecs_vec_create(elementSize)) {}

    EcsVec(const EcsVec& other) {
        ecs_vec_copy(const_cast<ecs_vec*>(&other._vec), &_vec);
    }

    EcsVec& operator=(const EcsVec& other) {
        if (this != &other) {
            ecs_vec_free(&_vec);
            ecs_vec_copy(const_cast<ecs_vec*>(&other._vec), &_vec);
        }
        return *this;
    }

    EcsVec(EcsVec&& other) noexcept
        : _vec(other._vec) {
        other._vec = {nullptr, 0, 0, 0};
    }

    EcsVec& operator=(EcsVec&& other) noexcept {
        if (this != &other) {
            ecs_vec_free(&_vec);
            _vec = other._vec;
            other._vec = {nullptr, 0, 0, 0};
        }
        return *this;
    }

    ~EcsVec() {
        ecs_vec_free(&_vec);
    }

    void* push(const void* elem) {
        return ecs_vec_push(&_vec, elem);
    }

    void* add() {
        return ecs_vec_add(&_vec);
    }

    void pushZero() {
        ecs_vec_push_zero(&_vec);
    }

    void set(size_t index, const void* elem) {
        ecs_vec_set(&_vec, index, elem);
    }

    void removeFast(size_t index) {
        ecs_vec_remove_fast(&_vec, index);
    }

    void removeOrdered(size_t index) {
        ecs_vec_remove_ordered(&_vec, index);
    }

    void removeLast() {
        ecs_vec_remove_last(&_vec);
    }

    void* get(size_t index) const {
        if (index >= _vec.count) throw std::out_of_range("EcsVec::get index out of range");
        return (char*)_vec.data + index * _vec.size;
    }

    void* last() const {
        if (_vec.count == 0) throw std::out_of_range("EcsVec::last empty vector");
        return (char*)_vec.data + (_vec.count - 1) * _vec.size;
    }

    void ensure(size_t count) {
        ecs_vec_ensure(&_vec, count);
    }

    size_t count() const { return _vec.count; }
    size_t capacity() const { return _vec.capacity; }
    size_t elementSize() const { return _vec.size; }

    ecs_vec* raw() { return &_vec; }
    const ecs_vec* raw() const { return &_vec; }

private:
    ecs_vec _vec;
};

#endif // ECS_VEC_WRAPPER_HPP
