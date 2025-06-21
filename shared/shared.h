#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <algorithm>

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }

    SharedPtr(std::nullptr_t) {
    }

    explicit SharedPtr(T* ptr) : ptr_(ptr), ctrl_(new ControlBlockPointer<T>(ptr)) {
        if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
            ptr_->SetWeakThis(WeakPtr<T>(*this));
        }
    }

    template <typename U, std::enable_if_t<std::is_convertible_v<U, T>, bool> = true>
    explicit SharedPtr(U* ptr)
        : ptr_(dynamic_cast<T*>(ptr)), ctrl_(new ControlBlockPointer<U>(ptr)) {
        if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
            ptr_->SetWeakThis(WeakPtr<T>(*this));
        }
    }

    SharedPtr(const SharedPtr<T>& other) : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        if (other) {
            ctrl_->IncreaseSharedCounter();
        }
    }

    template <typename U, std::enable_if_t<std::is_convertible_v<U, T>, bool> = true>
    SharedPtr(const SharedPtr<U>& other) : ptr_(other.Get()), ctrl_(other.GetControl()) {
        if (ctrl_ != nullptr) {
            ctrl_->IncreaseSharedCounter();
        }
    }

    SharedPtr(SharedPtr&& other) : ptr_(other.ptr_) {
        ctrl_ = other.ctrl_;
        if (ctrl_ != nullptr) {
            ctrl_->IncreaseSharedCounter();
        }
        other.Reset();
    }

    template <typename U, std::enable_if_t<std::is_convertible_v<U, T>, bool> = true>
    SharedPtr(SharedPtr<U>&& other) : ptr_(other.Get()), ctrl_(other.GetControl()) {
        if (ctrl_ != nullptr) {
            ctrl_->IncreaseSharedCounter();
        }
        other.Reset();
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : ptr_(ptr), ctrl_(other.GetControl()) {
        if (ctrl_ != nullptr) {
            if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
                ptr_->SetWeakThis(WeakPtr<T>(*this));
            }
            ctrl_->IncreaseSharedCounter();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) : ctrl_(other.ctrl_) {
        if (!other.Expired()) {
            ptr_ = other.ptr_;
            if (ctrl_ != nullptr) {
                ctrl_->IncreaseSharedCounter();
            }
        } else {
            throw BadWeakPtr();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other && other) {
            ControlBlock* old_ctrl = ctrl_;
            if (ctrl_ != nullptr) {
                ctrl_->DecreaseSharedCounter();
            }
            ptr_ = other.ptr_;
            ctrl_ = other.ctrl_;
            if (ctrl_ != nullptr) {
                ctrl_->IncreaseSharedCounter();
            }
            if (old_ctrl != nullptr && old_ctrl->GetSharedCounter() == 0 &&
                old_ctrl->GetWeakCounter() == 0) {
                delete old_ctrl;
            }
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (this != &other && other) {
            ControlBlock* old_ctrl = ctrl_;
            if (ctrl_ != nullptr) {
                ctrl_->DecreaseSharedCounter();
            }
            ptr_ = other.ptr_;
            ctrl_ = other.ctrl_;
            if (old_ctrl != nullptr && old_ctrl->GetSharedCounter() == 0 &&
                old_ctrl->GetWeakCounter() == 0) {
                delete old_ctrl;
            }
            other.ptr_ = nullptr;
            other.ctrl_ = nullptr;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DeleteControl();
        ptr_ = nullptr;
        ctrl_ = nullptr;
    }

    void Reset(T* ptr) {
        DeleteControl();
        ptr_ = ptr;
        ctrl_ = new ControlBlockPointer(ptr);
    }

    template <typename U, std::enable_if_t<std::is_convertible_v<U, T>, bool> = true>
    void Reset(U* ptr) {
        DeleteControl();
        ptr_ = ptr;
        ctrl_ = new ControlBlockPointer<U>(ptr);
    }

    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(ctrl_, other.ctrl_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    size_t UseCount() const {
        if (ctrl_ != nullptr) {
            return ctrl_->GetSharedCounter();
        } else {
            return 0;
        }
    }

    ControlBlock* GetControl() const {
        return ctrl_;
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_ = nullptr;
    ControlBlock* ctrl_ = nullptr;

    void Release() {
        ptr_ = nullptr;
        ctrl_ = nullptr;
    }

    template <typename... Args>
    SharedPtr(bool, Args&&... args)
        : ctrl_(new ControlBlockObject<T>(std::forward<Args>(args)...)) {
        ptr_ = reinterpret_cast<T*>(&dynamic_cast<ControlBlockObject<T>*>(ctrl_)->obj_);
        if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
            ptr_->SetWeakThis(WeakPtr(*this));
        }
    }

    void DeleteControl() {
        if (ctrl_ != nullptr) {
            ctrl_->DecreaseSharedCounter();
            if (ctrl_->GetSharedCounter() == 0 && ctrl_->GetWeakCounter() == 0) {
                delete ctrl_;
                ctrl_ = nullptr;
            }
        }
    }

    template <typename Y, typename U>
    friend inline bool operator==(const SharedPtr<Y>& left, const SharedPtr<U>& right);

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

    friend WeakPtr<T>;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.ctrl_ == right.ctrl_;
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(true, std::forward<Args>(args)...);
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_this_);
    }

    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(weak_this_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    }

    WeakPtr<const T> WeakFromThis() const noexcept {
        return weak_this_;
    }

    template <class Y>
    void SetWeakThis(WeakPtr<Y>&& weak_this) {
        weak_this_ = weak_this;
    }

private:
    WeakPtr<T> weak_this_;
};
