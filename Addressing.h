#ifndef ADDRESSINGMODE_H
#define ADDRESSINGMODE_H

#include "Memory.h"

/**
 * @brief Base class for addressing mode implementations
 *
 * @tparam Derived CRTP derived class type
 * @tparam Size Bit-width of addresses used by this addressing mode
 */
template <typename Derived, size_t Size>
struct AddressingMode {
    using address_type = Word<Size>; ///< Address representation type
    static constexpr size_t size = Size; ///< Address size in bits

    /**
     * @brief Read from memory using specific addressing mode
     *
     * @tparam MemType Memory type satisfying BasicMemory concept
     * @param address Target address/operand
     * @param memory Memory instance to access
     * @return typename MemType::word_type Resulting data word
     */
    template <BasicMemory MemType>
    typename MemType::word_type read_by_address(const address_type& address, MemType& memory) const {
        return static_cast<const Derived*>(this)->read_by_address_impl(address, memory);
    }
};

/**
 * @brief Immediate addressing mode implementation
 *
 * @tparam Size Bit-width of addresses and immediate values
 *
 * Returns the address itself as the data value (immediate value addressing)
 */
template <size_t Size>
struct ImmediateAddressing : public AddressingMode<ImmediateAddressing<Size>, Size> {
    template <BasicMemory MemType>
    typename MemType::word_type read_by_address_impl(
        const typename ImmediateAddressing::address_type& address,
        MemType& memory
    ) const {
        return typename MemType::word_type();
    };
};

/**
 * @brief Register addressing mode implementation
 *
 * @tparam Size Bit-width of register addresses
 *
 * Accesses registers in memory systems with register banks
 */
template <size_t Size>
struct RegisterAddressing : public AddressingMode<RegisterAddressing<Size>, Size> {
    template <MemoryWithRegisters MemType>
    typename MemType::word_type read_by_address_impl(
        const typename RegisterAddressing::address_type& address,
        MemType& memory
    ) const {
        return typename MemType::word_type();
    };
};

#endif //ADDRESSINGMODE_H
