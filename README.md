# Tagged Value Arithmetic

## Overview
This project implements a system for type-tagged values that can represent integers, doubles, and booleans in a unified way. It uses compile-time traits and constexpr functions to handle type-specific operations, providing a mechanism for safe value tagging, storage, and arithmetic.

## Key Concepts
### TaggedValue
- **TaggedValue** is a structure representing a tagged integer, double, or boolean.
- Tagging is implemented through a low-bit encoding scheme, where the lower three bits (`0b111`) identify the type.

### Type Policies
- **IntPolicy**: Encodes and decodes integers.
- **DoublePolicy**: Encodes doubles, stored in a heap.
- **BoolPolicy**: Encodes and decodes booleans directly into the tag.

### Heap
- **ConstexprHeap** is used to manage storage for types like doubles that require indirection.
- The heap simulates memory allocation at compile time, with overflow protection.

## Features
- **Type-Safe Access**: Accessing a value as the wrong type returns a default value (e.g., `-1` or `false`).
- **Compile-Time Evaluation**: The tagging system and operations like addition can be evaluated at compile time.
- **Generalized Addition**: Supports adding integers and doubles seamlessly, promoting integers to doubles when necessary.

## How It Works
### Tagging Mechanism
- **Integers**: Lower three bits are `0b000`.
- **Doubles**: Lower three bits are `0b001`, with storage in a heap.
- **Booleans**: Lower three bits are `0b011`.

### Example Walkthrough
The program demonstrates addition of an integer `10` and a double `20.5`.
The result is computed at compile time and printed as `30.5`.

```cpp
constexpr auto result = test_tagging();
std::cout << "Result: " << result << "\n";
```

## Code Breakdown
### Key Components
- **TagTraits**: Primary template to define type-specific tagging behavior.
- **ConstexprHeap**: Simple heap with compile-time storage for double values.
- **add**: Performs generalized addition between two `TaggedValue` instances.
- **test_tagging**: Example function that tests integer and double addition at compile time.

### Important Functions
- `TaggedValue::from<T>`: Creates a `TaggedValue` from a raw value.
- `TaggedValue::as<T>`: Retrieves a `TaggedValue` as the specified type.
- `ConstexprHeap::allocate`: Allocates space for a double.
- `ConstexprHeap::dereference`: Retrieves a stored double.

## Example
```cpp
ConstexprHeap<int64_t, 8> int_heap;
ConstexprHeap<double, 8> double_heap;

auto result = add(
    TaggedValue::from<int64_t>(10, int_heap),
    TaggedValue::from<double>(20.5, double_heap),
    int_heap,
    double_heap
);

std::cout << result.as<double, ConstexprHeap<double, 8>>(double_heap) << "\n";
// Outputs: 30.5
```

## Compilation
### Requirements
- C++17 or later

### Compile Command
```bash
g++ -std=c++17 main.cpp -o tagged_value
```

## Running the Program
```bash
./tagged_value
```

## Notes
- The code is designed for constexpr evaluation, but can run dynamically as well.
- Overflow protection is in place for heaps, but error reporting is minimal.
- Extendable by defining additional type policies and extending `TagTraits`.

## Future Improvements
- Implement support for more types (e.g., strings or user-defined types).
- Improve error handling and logging.
- Optimize heap allocation and dereferencing for better runtime performance.

