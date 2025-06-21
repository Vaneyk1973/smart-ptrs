#pragma once

#include <type_traits>
#include <utility>

template <typename V>
inline constexpr bool is_compressed = std::is_empty_v<V> && !std::is_final_v<V>;  // NOLINT

template <typename F, typename S, bool is_compressed_F = is_compressed<F>,
          bool is_compressed_S = is_compressed<S>,
          bool is_derived = std::is_base_of_v<F, S> || std::is_base_of_v<S, F>>
class CompressedPair;

template <typename F, typename S, bool is_empty_F, bool is_empty_S, bool is_derived>
class CompressedPair {
public:
    CompressedPair() : first_(F()), second_(S()) {
    }

    CompressedPair(const F& first, const S& second) : first_(first), second_(second) {
    }

    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)) {
    }

    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second) {
    }

    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    S& GetSecond() {
        return second_;
    };

    const F& GetFirst() const {
        return first_;
    }

    const S& GetSecond() const {
        return second_;
    };

    void Swap(CompressedPair& other) {
        std::swap(first_, other.first_);
        std::swap(second_, other.second_);
    }

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, false, false> : private F {
public:
    CompressedPair() : second_(S()) {
    }

    CompressedPair(const F&, const S& second) : second_(second) {
    }

    CompressedPair(const F&, S&& second) : second_(std::move(second)) {
    }

    CompressedPair(F&&, const S& second) : second_(second) {
    }

    CompressedPair(F&&, S&& second) : second_(std::move(second)) {
    }

    F& GetFirst() {
        return *this;
    }

    S&& GetSecond() {
        return second_;
    };

    const F& GetFirst() const {
        return *this;
    }

    const S& GetSecond() const {
        return second_;
    };

    void Swap(CompressedPair& other) {
        std::swap(second_, other.second_);
    }

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true, false> : private S {
public:
    CompressedPair() : first_(F()) {
    }

    CompressedPair(const F& first, const S&) : first_(first) {
    }

    CompressedPair(const F& first, S&&) : first_(first) {
    }

    CompressedPair(F&& first, const S&) : first_(std::move(first)) {
    }

    CompressedPair(F&& first, S&&) : first_(std::move(first)) {
    }

    F& GetFirst() {
        return first_;
    }

    S& GetSecond() {
        return *this;
    };

    const F& GetFirst() const {
        return first_;
    }

    const S& GetSecond() const {
        return *this;
    };

    void Swap(CompressedPair& other) {
        std::swap(first_, other.first_);
    }

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, true, false> : private F, private S {
public:
    CompressedPair() {
    }

    CompressedPair(const F&, const S&) {
    }

    CompressedPair(const F&, S&&) {
    }

    CompressedPair(F&&, const S&) {
    }

    CompressedPair(F&&, S&&) {
    }

    CompressedPair(CompressedPair&&) {
    }

    F& GetFirst() {
        return *this;
    }

    S& GetSecond() {
        return *this;
    };

    const F& GetFirst() const {
        return *this;
    }

    const S& GetSecond() const {
        return *this;
    };

    void Swap(CompressedPair&) {
    }
};
