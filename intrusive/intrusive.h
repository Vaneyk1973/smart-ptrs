#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    }

    size_t DecRef() {
        --count_;
        return count_;
    }

    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {

    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    RefCounted() : counter_(new Counter()) {
    }

    // Increase reference counter.
    void IncRef() {
        counter_->IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        counter_->DecRef();
        if (RefCount() == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        if (counter_ != nullptr) {
            return counter_->RefCount();
        } else {
            return 0;
        }
    }

    RefCounted& operator=(const RefCounted& other) {
        Counter* old_counter = counter_;
        counter_ = other.counter_;
        IncRef();
        if (old_counter != nullptr) {
            delete old_counter;
        }
        return *this;
    }

    RefCounted& operator=(RefCounted&& other) {
        if (this != &other) {
            Counter* old_counter = counter_;
            if (counter_ != nullptr) {
                counter_->DecRef();
            }
            counter_ = std::move(other.counter_);
            other.counter_ = nullptr;
            if (RefCount() == 0) {
                IncRef();
            }
            if (old_counter != nullptr && old_counter->RefCount() == 0) {
                delete old_counter;
            }
        }
        return *this;
    }

    ~RefCounted() {
        if (counter_ != nullptr) {
            delete counter_;
            counter_ = nullptr;
        }
    }

private:
    Counter* counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() : ptr_(nullptr) {
    }

    IntrusivePtr(std::nullptr_t) : IntrusivePtr() {
    }

    IntrusivePtr(T* ptr) : ptr_(ptr) {
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        if (other.ptr_ != nullptr) {
            ptr_ = dynamic_cast<T*>(other.ptr_);
            ptr_->IncRef();
        } else {
            ptr_ = nullptr;
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        ptr_ = std::move(dynamic_cast<T*>(other.ptr_));
        other.ptr_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) : ptr_(other.ptr_) {
        if (other.ptr_ != nullptr) {
            ptr_->IncRef();
        }
    }

    IntrusivePtr(IntrusivePtr&& other) : ptr_(std::move(other.ptr_)) {
        other.ptr_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        Reset(other.ptr_);
        return *this;
    }

    template <typename Y>
    IntrusivePtr& operator=(const IntrusivePtr<Y>& other) {
        T* old_ptr = ptr_;
        if (other.ptr_ != nullptr) {
            ptr_ = dynamic_cast<T*>(other.ptr_);
            ptr_->IncRef();
        } else {
            ptr_ = nullptr;
        }
        if (old_ptr != nullptr) {
            old_ptr->DecRef();
        }
        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (this != &other) {
            T* old_ptr = ptr_;
            ptr_ = std::move(dynamic_cast<T*>(other.ptr_));
            other.ptr_ = nullptr;
            if (old_ptr != nullptr) {
                old_ptr->DecRef();
            }
        }
        return *this;
    }

    template <typename Y>
    IntrusivePtr& operator=(IntrusivePtr<Y>&& other) {
        if (ptr_ == other.ptr_) {
            ptr_->DecRef();
        }
        T* old_ptr = ptr_;
        ptr_ = std::move(dynamic_cast<T*>(other.ptr_));
        other.ptr_ = nullptr;
        if (old_ptr != nullptr) {
            old_ptr->DecRef();
        }
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
    }

    // Modifiers
    void Reset() {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
            ptr_ = nullptr;
        }
    }

    void Reset(T* ptr) {
        if (ptr != ptr_) {
            if (ptr_ != nullptr) {
                ptr_->DecRef();
            }
            ptr_ = ptr;
            if (ptr_ != nullptr) {
                ptr_->IncRef();
            }
        }
    }

    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

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
        if (ptr_ != nullptr) {
            return ptr_->RefCount();
        } else {
            return 0;
        }
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}
