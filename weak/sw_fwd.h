#pragma once

#include <exception>
#include <array>

class BadWeakPtr : public std::exception {};

class EnableSharedFromThisBase {};

template <typename T>
class EnableSharedFromThis;

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class ControlBlock {
public:
    ControlBlock() {
    }

    ControlBlock(const ControlBlock&) = delete;

    ControlBlock& operator=(const ControlBlock&) = delete;

    ControlBlock& operator=(ControlBlock&&) = delete;

    void IncreaseSharedCounter() {
        ++shared_counter_;
    }

    virtual void DecreaseSharedCounter(int counter = 1) {
        shared_counter_ -= counter;
    }

    void IncreaseWeakCounter(int counter = 1) {
        weak_counter_ += counter;
    }

    void DecreaseWeakCounter() {
        --weak_counter_;
    }

    int GetSharedCounter() {
        return shared_counter_;
    }

    int GetWeakCounter() {
        return weak_counter_;
    }

    virtual ~ControlBlock() {
    }

protected:
    int shared_counter_ = 0;
    int weak_counter_ = 0;

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);
};

template <typename T>
class ControlBlockPointer : public ControlBlock {
public:
    ControlBlockPointer() {
    }

    ControlBlockPointer(T* ptr) : obj_(ptr) {
        shared_counter_ = 1;
    }

    ControlBlockPointer(ControlBlockPointer&& other) : obj_(other.obj_) {
        shared_counter_ = other.shared_counter_;
        other.obj_ = nullptr;
        other.shared_counter_ = 0;
    }

    void DecreaseSharedCounter(int counter = 1) {
        shared_counter_ -= counter;
        if (shared_counter_ == 0) {
            if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
                ++shared_counter_;
                delete obj_;
                --shared_counter_;
                obj_ = nullptr;
            }
        }
    }

    ~ControlBlockPointer() {
        if (obj_ != nullptr) {
            if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
                ++shared_counter_;
                delete obj_;
                obj_ = nullptr;
                --shared_counter_;
            } else {
                delete obj_;
                obj_ = nullptr;
            }
        }
    }

private:
    T* obj_ = nullptr;
};

template <typename T>
class ControlBlockObject : public ControlBlock {
public:
    template <typename... Args>
    ControlBlockObject(Args&&... args) {
        new (&obj_) T(std::forward<Args>(args)...);
        shared_counter_ = 1;
    }

    ControlBlockObject(ControlBlockObject&& other) {
        obj_ = std::move(other.obj_);
        shared_counter_ = other.shared_counter_;
        other.DecreaseSharedCounter(shared_counter_);
    }

    void DecreaseSharedCounter(int counter = 1) {
        shared_counter_ -= counter;
        if (shared_counter_ == 0) {
            if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
                ++shared_counter_;
                reinterpret_cast<T*>(&obj_)->~T();
                --shared_counter_;
            } else {
                reinterpret_cast<T*>(&obj_)->~T();
            }
        }
    }

    ~ControlBlockObject() {
    }

private:
    alignas(T) std::array<char, sizeof(T)> obj_;

    friend SharedPtr<T>;
};
