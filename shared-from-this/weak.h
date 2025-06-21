#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        if (ctrl_ != nullptr) {
            ctrl_->IncreaseWeakCounter();
        }
    }

    template <class Y>
    WeakPtr(const WeakPtr<Y>& other) : ptr_(other.Get()), ctrl_(other.GetControl()) {
        if (ctrl_ != nullptr) {
            ctrl_->IncreaseWeakCounter();
        }
    }

    WeakPtr(WeakPtr&& other) : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        other.ptr_ = nullptr;
        other.ctrl_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : ptr_(other.ptr_), ctrl_(other.ctrl_) {
        if (ctrl_ != nullptr) {
            ctrl_->IncreaseWeakCounter();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const SharedPtr<T>& other) {
        ControlBlock* old_ctrl = ctrl_;
        ptr_ = other.ptr_;
        ctrl_ = other.ctrl_;
        if (ctrl_ != nullptr) {
            ctrl_->IncreaseWeakCounter();
        }
        if (old_ctrl != nullptr) {
            old_ctrl->DecreaseWeakCounter();
        }
        return *this;
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (this != &other) {
            ControlBlock* old_ctrl = ctrl_;
            ptr_ = other.ptr_;
            ctrl_ = other.ctrl_;
            if (ctrl_ != nullptr) {
                ctrl_->IncreaseWeakCounter();
            }
            if (old_ctrl != nullptr) {
                old_ctrl->DecreaseWeakCounter();
            }
        }
        return *this;
    }

    template <class Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        ControlBlock* old_ctrl = ctrl_;
        ptr_ = dynamic_cast<T*>(other.Get());
        ctrl_ = other.GetControl();
        if (ctrl_ != nullptr) {
            ctrl_->IncreaseWeakCounter();
        }
        if (old_ctrl != nullptr) {
            old_ctrl->DecreaseWeakCounter();
        }
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (this != &other) {
            ControlBlock* old_ctrl = ctrl_;
            if (ctrl_ != nullptr) {
                ctrl_->DecreaseWeakCounter();
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

    ~WeakPtr() {
        if (ctrl_ != nullptr) {
            ctrl_->DecreaseWeakCounter();
            if (ctrl_->GetSharedCounter() == 0 && ctrl_->GetWeakCounter() == 0) {
                delete ctrl_;
                ctrl_ = nullptr;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (ctrl_ != nullptr) {
            ctrl_->DecreaseWeakCounter();
            if (ctrl_->GetSharedCounter() == 0 && ctrl_->GetWeakCounter() == 0) {
                delete ctrl_;
            }
        }
        ptr_ = nullptr;
        ctrl_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(ctrl_, other.ctrl_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (ctrl_ != nullptr) {
            return ctrl_->GetSharedCounter();
        } else {
            return 0;
        }
    }

    bool Expired() const {
        return ctrl_ == nullptr || ctrl_->GetSharedCounter() == 0;
    }

    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        } else {
            return SharedPtr<T>(*this);
        }
    }

    T* Get() const {
        return ptr_;
    }

    ControlBlock* GetControl() const {
        return ctrl_;
    }

private:
    T* ptr_ = nullptr;
    ControlBlock* ctrl_ = nullptr;

    friend SharedPtr<T>;
};
