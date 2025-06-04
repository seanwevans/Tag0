#include <array>
#include <cstdint>

#include <iostream>
#include <type_traits>


// Base Tagging Traits (Primary Template)
template <typename T> 
struct TagTraits;


// Type Policies
struct IntPolicy {
    static constexpr int64_t tag = 0b000;
    static constexpr int64_t tag_value(int64_t value) { return (value << 3) | tag; }
    static constexpr int64_t untag_value(int64_t value) { return value >> 3; }
};

struct DoublePolicy {
    static constexpr int64_t tag = 0b001;
};

struct BoolPolicy {
    static constexpr int64_t tag = 0b011;
    static constexpr int64_t tag_value(bool value) { return (value ? 1 : 0) | tag; }
    static constexpr bool untag_value(int64_t value) { return (value & ~0b111) != 0; }
};

template <> struct TagTraits<int64_t> : IntPolicy {};
template <> struct TagTraits<double> : DoublePolicy {};
template <> struct TagTraits<bool> : BoolPolicy {};

struct TaggedValue {
    int64_t raw;

    template <typename T, typename Heap>
    constexpr bool is_type() const {
        return (raw & 0b111) == TagTraits<T>::tag;
    }

    template <typename T, typename Heap>
    constexpr T as(const Heap& heap) const {
        if constexpr (std::is_same_v<T, int64_t>) {
            return is_type<int64_t, Heap>() ? TagTraits<int64_t>::untag_value(raw) : -1;
        }
        else if constexpr (std::is_same_v<T, double>) {
            return is_type<double, Heap>() ? heap.dereference(raw) : -1.0;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return is_type<bool, Heap>() ? TagTraits<bool>::untag_value(raw) : false;
        }
        return -1; // Unsupported type
    }

    template <typename T, typename Heap>
    static constexpr TaggedValue from(T value, Heap& heap) {
        if constexpr (std::is_same_v<T, int64_t>) {
            return { TagTraits<int64_t>::tag_value(value) };
        }
        else if constexpr (std::is_same_v<T, double>) {
            auto ptr = heap.allocate(value);
            if (ptr == -1) {
                assert(false && "Heap allocation failed in TaggedValue::from");
                return { -1 };
            }
            return { ptr };
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return { TagTraits<bool>::tag_value(value) };
        }
        return { -1 }; // Unsupported type
    }
};


// Heap for Boxing (for Types Needing Indirection)
template <typename T, size_t N> struct ConstexprHeap {
    std::array<T, N> storage{};
    size_t next_index = 0;

    constexpr int64_t allocate(T value) {
        if (next_index >= N) {
            return -1; // Sentinel for overflow
        }
        storage[next_index] = value;
        return (static_cast<int64_t>(next_index++) << 3) | TagTraits<T>::tag;
    }

    constexpr T dereference(int64_t ptr) const {
        size_t index = (ptr >> 3);
        if (index >= N) {
            return -1; // Sentinel for invalid pointer
        }
        return storage[index];
    }
};



// Generalized Addition
template <typename IntHeap, typename DoubleHeap>
constexpr TaggedValue add(const TaggedValue& a, const TaggedValue& b, IntHeap& int_heap, DoubleHeap& double_heap) {
    if (a.is_type<int64_t, IntHeap>() && b.is_type<int64_t, IntHeap>()) {
        return TaggedValue::from<int64_t>(
            a.as<int64_t, IntHeap>(int_heap) + b.as<int64_t, IntHeap>(int_heap),
            int_heap
        );
    }

    double fa = a.is_type<int64_t, IntHeap>()
        ? static_cast<double>(a.as<int64_t, IntHeap>(int_heap))
        : a.as<double, DoubleHeap>(double_heap);
    double fb = b.is_type<int64_t, IntHeap>()
        ? static_cast<double>(b.as<int64_t, IntHeap>(int_heap))
        : b.as<double, DoubleHeap>(double_heap);


    return TaggedValue::from<double>(fa + fb, double_heap);
}



// Test Functionality
constexpr auto test_tagging() {
    ConstexprHeap<int64_t, 8> int_heap;
    ConstexprHeap<double, 8> double_heap;

    auto a = TaggedValue::from<int64_t>(10, int_heap);
    assert(a.raw != -1 && "Allocation failed for int64_t in test_tagging");

    auto b = TaggedValue::from<double>(20.5, double_heap);
    assert(b.raw != -1 && "Allocation failed for double in test_tagging");

    auto result = add(a, b, int_heap, double_heap);

    return result.as<double, ConstexprHeap<double, 8>>(double_heap); // Should return 30.5
}

constexpr bool test_heap_overflow() {
    ConstexprHeap<double, 1> heap;
    auto first = heap.allocate(1.0);
    auto ok_first = first != -1;
    auto second = heap.allocate(2.0);
    auto ok_second = second == -1;
    return ok_first && ok_second;
}
static_assert(test_heap_overflow(), "Heap overflow test failed");

constexpr double result = test_tagging();


// Entry-point
int main() {
    std::cout << "Result: " << result << "\n"; // Outputs: Result: 30.5
    return 0;
}
