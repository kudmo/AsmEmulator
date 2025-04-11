#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <cstdint>
#include <concepts>
#include <array>
#include <stdexcept>

using bits_t = std::vector<bool>;

/**
 * @brief A fixed-size binary word template class.
 *
 * @tparam Size The compile-time specified size of the Word (number of bits).
 *
 * This class represents a binary word of fixed size, stored as a sequence of bits.
 * It provides iterator access, element-wise operations, and size queries.
 * The underlying storage is a bits_t, but access is constrained to the first `Size` bits.
 */
template <size_t Size>
struct Word {
    /// Internal storage for bits
    bits_t bits = bits_t(Size);
    /// Iterator type for mutable bit access
    using iterator = bits_t::iterator;
    /// Iterator type for read-only bit access
    using const_iterator = bits_t::const_iterator;

    /**
     * @brief Default constructor.
     * @post Initializes all bits to false (0).
     */
    explicit Word() = default;
    /**
    * @brief Copy constructor.
    * @post Copy word value from other.
    */
    Word(const Word&) = default;
    /**
    * @brief Move constructor.
    * @post Move word value from other.
    */
    Word(Word&&) = default;
    /**
    * @brief Copy operator.
    * @post Copy word value from other.
    */
    Word& operator=(const Word&) = default;
    /**
    * @brief Move operator.
    * @post Move word value from other.
    */
    Word& operator=(Word&&) = default;
    /**
     * @brief Constructs a Word from a vector of booleans.
     * @param bits Source bits vector.
     * @throws std::invalid_argument if bits.size() not equal to Size
     */
    explicit Word(const bits_t& bits) {
        if (bits.size() != Size)
            throw std::invalid_argument("Word size must be equal to the size of vector");
        std::copy(bits.begin(), bits.end(), bits.begin());
    }

    /**
     * @brief Constructs a Word from an iterator range.
     * @param begin Start iterator of the bit sequence.
     * @param end End iterator of the bit sequence.
     * @throws std::invalid_argument if distance between begin and end not equal to Size
     */
    explicit Word(const_iterator begin, const_iterator end) {
        if (std::distance(begin, end) != Size)
            throw std::invalid_argument("Word size must be equal to the size of vector");
        std::copy(begin, end, bits.begin());
    }

    /**
     * @brief Accesses the bit at the specified position.
     * @param index Zero-based bit position (0 <= index < Size).
     * @return bool Value of the bit at `index`.
     * @throws std::invalid_argument if index out of bounds
     */
    bool operator[](const size_t index) const {
        if (index >= Size)
            throw std::invalid_argument("Word index out of bounds");
        return bits[index];
    }

    /**
     * @brief Returns the compile-time size of the Word.
     * @return constexpr size_t Always equal to the template parameter `Size`.
     */
    constexpr size_t size() const { return Size; }

    /**
     * @brief Returns an iterator to the first bit.
     */
    iterator begin() { return bits.begin(); }

    /**
     * @brief Returns an iterator past the last valid bit (position `Size`).
     */
    iterator end() { return bits.begin() + Size; }

    /**
     * @brief Returns a const iterator to the first bit.
     */
    const_iterator begin() const { return bits.begin(); }

    /**
     * @brief Returns a const iterator past the last valid bit (position `Size`).
     */
    const_iterator end() const { return bits.begin() + Size; }
};

/**
 * @brief Concept defining basic memory interface requirements.
 *
 * A type satisfies BasicMemory if it provides:
 * - `word_type`: Type representing a memory word
 * - `address_type`: Type used for memory addressing
 * - `read()`: Const method to read words from memory
 * - `write()`: Method to write words to memory
 *
 * @tparam T Type to check against memory concept
 */
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

/**
 * @brief Fixed-size memory implementation with bounds checking
 *
 * @tparam WordSize Bit-width of each memory word
 * @tparam Capacity Number of addressable words in memory
 *
 * Implements BasicMemory concept using vector storage. Provides:
 * - Fixed capacity determined at compile-time
 * - Runtime bounds checking on access
 * - Default-initialized memory contents
 *
 * @note Addresses are 0-based. Maximum valid address is Capacity-1
 */
template <size_t WordSize, size_t Capacity>
struct Memory {
    /// Address type
    using address_type = uint64_t;
    /// Word type with specified bit-width
    using word_type = Word<WordSize>;

    /// Storage container initialized with default-constructed words
    std::vector<word_type> data = std::vector<word_type>(Capacity);

    /**
     * @brief Read word from memory
     * @param address Memory location to read
     * @return word_type Copy of stored word at address
     * @throws std::out_of_range If address >= Capacity
     */
    word_type read(address_type address) const {
        if (address >= Capacity) throw std::out_of_range("Memory read overflow");
        return data[address];
    }

    /**
     * @brief Write word to memory
     * @param address Memory location to write
     * @param word Value to store at address
     * @throws std::out_of_range If address >= Capacity
     */
    void write(address_type address, const word_type& word) {
        if (address >= Capacity) throw std::out_of_range("Memory write overflow");
        data[address] = word;
    }
};

/**
 * @brief Concept extending BasicMemory with register operations
 *
 * A type satisfies MemoryWithRegisters if it:
 * - Satisfies BasicMemory requirements
 * - Provides register-specific access methods:
 *   - get_register(): Read from register space
 *   - write_register(): Write to register space
 *
 * @tparam T Type to check against register memory concept
 */
template <typename T>
concept MemoryWithRegisters =
    BasicMemory<T> &&
    requires(const T& c_mem, typename T::address_type addr) {
        { c_mem.get_register(addr) } -> std::same_as<typename T::word_type>;
    } &&
    requires(T& mem, typename T::address_type addr, const typename T::word_type& word) {
        { mem.write_register(addr, word) };
    };

/**
 * @brief Memory implementation with separate register space
 *
 * @tparam WordSize    Bit-width of memory words
 * @tparam Capacity    Number of main memory words
 * @tparam RegisterCapacity Number of register slots
 *
 * Extends BasicMemory with:
 * - Additional register storage space
 * - Separate register access methods
 * - Independent bounds checking for registers
 *
 * Satisfies both BasicMemory and MemoryWithRegisters concepts
 */
template <size_t WordSize, size_t Capacity, size_t RegisterCapacity>
struct AdvancedMemory : Memory<WordSize, Capacity> {
    using word_type = typename Memory<WordSize, Capacity>::word_type;    ///< Inherited word type
    using address_type = typename Memory<WordSize, Capacity>::address_type; ///< Inherited address type

    /// Register storage container (separate from main memory)
    std::vector<word_type> registers = std::vector<word_type>(RegisterCapacity);

    /**
     * @brief Read from register space
     * @param reg Register address (0-based)
     * @return word_type Value stored in register
     * @throws std::out_of_range For reg >= RegisterCapacity
     */
    word_type get_register(address_type reg) const {
        if (reg >= RegisterCapacity) throw std::out_of_range("Register read overflow");
        return registers[reg];
    }

    /**
     * @brief Write to register space
     * @param reg Register address (0-based)
     * @param word Value to store
     * @throws std::out_of_range For reg >= RegisterCapacity
     */
    void write_register(address_type reg, const word_type& word) {
        if (reg >= RegisterCapacity) throw std::out_of_range("Register write overflow");
        registers[reg] = word;
    }
};
static_assert(MemoryWithRegisters<AdvancedMemory<4,32,8>>);

#endif //MEMORY_H
