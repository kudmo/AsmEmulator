#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <cstdint>
#include <concepts>
#include <array>
#include <stdexcept>

template <size_t Size>
struct Word {
    std::vector<bool> bits = std::vector<bool>(Size);
    using iterator = std::vector<bool>::iterator;
    using const_iterator = std::vector<bool>::const_iterator;

    explicit Word () = default;
    explicit Word(const std::vector<bool>& bits): bits(bits) {}
    explicit Word(const_iterator begin, const_iterator end): bits(begin, end) {}

    bool operator[](const size_t index) const { return bits[index]; }

    constexpr size_t size() const { return Size; }

    iterator begin() { return bits.begin(); }
    iterator end() { return bits.begin() + Size; }
    const_iterator begin() const { return bits.begin(); }
    const_iterator end() const { return bits.begin() + Size; }
};

template <typename T>
concept BasicMemory =
    requires {
        typename T::word_type;
        typename T::address_type;
    } &&
    requires(const T& c_mem, typename T::address_type addr) {
        { c_mem.read(addr) } -> std::same_as<typename T::word_type>;
    } &&
    requires(T& mem, typename T::address_type addr, const typename T::word_type& word) {
        { mem.write(addr, word) };
    };

template <size_t WordSize, size_t Capacity>
struct Memory {
    using address_type = uint64_t;
    using word_type = Word<WordSize>;
    std::vector<word_type> data = std::vector<word_type>(Capacity);

    word_type read(address_type address) const {
        if (address >= Capacity) throw std::out_of_range("Memory read overflow");
        return data[address];
    }

    void write(address_type address, const word_type& word) {
        if (address >= Capacity) throw std::out_of_range("Memory write overflow");
        data[address] = word;
    }
};

static_assert(BasicMemory<Memory<4,32>>);


template <typename T>
concept MemoryWithRegisters =
    BasicMemory<T> &&
    requires(const T& c_mem, typename T::address_type addr) {
        { c_mem.get_register(addr) } -> std::same_as<typename T::word_type>;
    } &&
    requires(T& mem, typename T::address_type addr, const typename T::word_type& word) {
        { mem.write_register(addr, word) };
    };

template <size_t WordSize, size_t Capacity, size_t RegisterCapacity>
struct AdvancedMemory : Memory<WordSize, Capacity> {
    using word_type = typename Memory<WordSize, Capacity>::word_type;
    using address_type = typename Memory<WordSize, Capacity>::address_type;

    std::vector<word_type> registers = std::vector<word_type>(RegisterCapacity);

    word_type get_register(address_type reg) const {
        if (reg >= RegisterCapacity) throw std::out_of_range("Memory read overflow");
        return registers[reg];
    }
    void write_register(address_type reg, word_type word) {
        if (reg >= RegisterCapacity) throw std::out_of_range("Memory read overflow");
        registers[reg] = word;
    }
};

static_assert(MemoryWithRegisters<AdvancedMemory<4,32,8>>);

#endif //MEMORY_H
