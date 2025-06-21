#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

struct Slug {
    template <typename T>
    void operator()(T* ptr) {
        delete ptr;
    }

    template <typename T>
    void operator()(T* ptr, int) {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = Slug>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr, Deleter()) {
    }

    template <typename U, std::enable_if_t<std::is_base_of_v<T, U>>>
    explicit UniquePtr(U* ptr = nullptr) : ptr_(dynamic_cast<T*>(ptr), Deleter()) {
    }

    UniquePtr(T* ptr, const Deleter& deleter) : ptr_(ptr, deleter) {
    }

    UniquePtr(T* ptr, Deleter&& deleter) : ptr_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(std::move(other.Get()), std::move(other.GetDeleter())) {
        other.Release();
    }

    template <typename U, typename Deleter_,
              std::enable_if_t<std::is_base_of_v<T, U> && std::is_base_of_v<Deleter, Deleter_>,
                               bool> = true>
    UniquePtr(UniquePtr<U, Deleter_>&& other) noexcept
        : ptr_(std::move(dynamic_cast<T*>(other.Get())),
               std::move(static_cast<Deleter&>(other.GetDeleter()))) {
        other.Release();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&) = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Reset();
            ptr_.GetFirst() = std::move(other.Get());
            ptr_.GetSecond() = std::move(other.GetDeleter());
            other.Release();
        }
        return *this;
    }

    template <typename U, typename Deleter_,
              std::enable_if_t<std::is_base_of_v<T, U> && std::is_base_of_v<Deleter, Deleter_>,
                               bool> = true>
    UniquePtr& operator=(UniquePtr<U, Deleter_>&& other) noexcept {
        Reset();
        ptr_.GetFirst() = std::move(dynamic_cast<T*>(other.Get()));
        ptr_.GetSecond() = std::move(static_cast<Deleter&>(other.GetDeleter()));
        other.Release();
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        DeletePointer();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        return std::exchange(ptr_.GetFirst(), nullptr);
    }

    void Reset(T* ptr = nullptr) {
        T* old_ptr = Get();
        ptr_.GetFirst() = ptr;
        if (old_ptr) {
            ptr_.GetSecond()(old_ptr);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }

    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }

    explicit operator bool() const {
        return ptr_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_.GetFirst();
    }

    T* operator->() const {
        return ptr_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> ptr_;

    void DeletePointer() {
        ptr_.GetSecond()(ptr_.GetFirst());
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_(CompressedPair(ptr, Deleter())) {
    }

    UniquePtr(T* ptr, const Deleter& deleter) : ptr_(CompressedPair(ptr, deleter)) {
    }

    UniquePtr(T* ptr, Deleter&& deleter) : ptr_(CompressedPair(ptr, std::move(deleter))) {
    }

    UniquePtr(UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(
              CompressedPair(std::move(other.ptr_.GetFirst()), std::move(other.ptr_.GetSecond()))) {
        other.Release();
    }

    template <typename U, typename Deleter_,
              std::enable_if_t<std::is_base_of_v<T, U> && std::is_base_of_v<Deleter, Deleter_>,
                               bool> = true>
    UniquePtr(UniquePtr<U, Deleter_>&& other) noexcept
        : ptr_(std::move(dynamic_cast<T*>(other.Get())),
               std::move(static_cast<Deleter&>(other.GetDeleter()))) {
        other.Release();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&) = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Reset();
            ptr_.GetFirst() = std::move(other.ptr_.GetFirst());
            ptr_.GetSecond() = std::move(other.ptr_.GetSecond());
            other.Release();
        }
        return *this;
    }

    template <typename U, typename Deleter_,
              std::enable_if_t<std::is_base_of_v<T, U> && std::is_base_of_v<Deleter, Deleter_>,
                               bool> = true>
    UniquePtr& operator=(UniquePtr<U, Deleter_>&& other) noexcept {
        Reset();
        ptr_.GetFirst() = std::move(dynamic_cast<T*>(other.Get()));
        ptr_.GetSecond() = std::move(static_cast<Deleter&>(other.GetDeleter()));
        other.Release();
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        DeletePointer();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        return std::exchange(ptr_.GetFirst(), nullptr);
    }

    void Reset(T* ptr = nullptr) {
        T* old_ptr = ptr_.GetFirst();
        ptr_.GetFirst() = ptr;
        if (old_ptr) {
            if constexpr (std::is_same_v<Deleter, Slug>) {
                ptr_.GetSecond()(old_ptr, 0);
            } else {
                ptr_.GetSecond()(old_ptr);
            }
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }

    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }

    explicit operator bool() const {
        return ptr_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_.GetFirst();
    }

    T* operator->() const {
        return ptr_.GetFirst();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Array dereference operators

    T& operator[](std::size_t i) {
        return ptr_.GetFirst()[i];
    }

    const T& operator[](std::size_t i) const {
        return ptr_.GetFirst()[i];
    }

private:
    CompressedPair<T*, Deleter> ptr_;

    void DeletePointer() {
        if constexpr (std::is_same_v<Deleter, Slug>) {
            ptr_.GetSecond()(ptr_.GetFirst(), 0);
        } else {
            ptr_.GetSecond()(ptr_.GetFirst());
        }
    }
};
