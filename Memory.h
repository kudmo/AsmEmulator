#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <cstdint>
#include <concepts>
#include <stdexcept>
#include "Utils.h"

/**
 * @brief Concept defining basic memory interface requirements.
 *
 * A type \c T satisfies BasicMemory if it provides:
 * - \c word_type: Type representing a memory word
 * - \c address_type: Type used for memory addressing
 * - \c read(): Const method to read words from memory
 * - \c write(): Method to write words to memory
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
 * A type \cT satisfies MemoryWithRegisters if it:
 * - Satisfies \c BasicMemory requirements
 * - Provides register-specific access methods:
 *   - \c get_register(): Read from register space
 *   - \c write_register(): Write to register space
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

/**
 * @brief Concept for systems with instruction pointer support
 *
 * A type satisfies MemoryWithInstructionPointer if it:
 * - Provides instruction pointer management methods
 *   - \c get_instruction_pointer():
 *   - \c increment_instruction_pointer():
 *   - \c increment_instruction_pointer(offset):
 */
template <typename T>
concept InstructionPointerSystem =
    requires(const T& mem) {
    { mem.get_instruction_pointer() } -> std::same_as<size_t>;
    } &&
    requires(T& mem, size_t addr) {
    { mem.set_instruction_pointer(addr) };
    } &&
    requires(T& mem, size_t offset) {
    { mem.increment_instruction_pointer(offset) };
    };

/**
 * @brief Memory system with integrated instruction pointer management
 *
 * @tparam WordSize         Bit width of memory words (must be > 0)
 * @tparam Capacity         Total number of addressable words in main memory
 * @tparam RegisterCapacity Number of general-purpose registers
 */
template <size_t WordSize, size_t Capacity, size_t RegisterCapacity>
class ComputerMemory : public AdvancedMemory<WordSize, Capacity, RegisterCapacity> {
private:
    size_t instruction_pointer = 0;

public:
    /// @brief Word type for memory operations
    using word_type = typename AdvancedMemory<WordSize, Capacity, RegisterCapacity>::word_type;

    /// @brief Address type for memory access
    using address_type = typename AdvancedMemory<WordSize, Capacity, RegisterCapacity>::address_type;

    /**
     * @brief Get current instruction pointer value
     * @return Current position in instruction stream
     */
    size_t get_instruction_pointer() const noexcept {
        return instruction_pointer;
    }

    /**
     * @brief Set instruction pointer to absolute position
     * @param value New instruction pointer value
     *
     * @throws std::out_of_range if value >= Capacity
     */
    void set_instruction_pointer(size_t value) {
        if (value >= Capacity) {
            throw std::out_of_range("Instruction pointer exceeds memory capacity");
        }
        instruction_pointer = value;
    }

    /**
     * @brief Advance instruction pointer with wrap-around
     * @param value Number of positions to advance (default 1)
     */
    void increment_instruction_pointer(size_t value = 1) noexcept {
        instruction_pointer = (instruction_pointer + value) % Capacity;
    }
};

#endif //MEMORY_H
