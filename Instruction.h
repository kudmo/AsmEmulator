#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <iostream>
#include <tuple>
#include <utility>
#include <type_traits>
#include <array>
#include "Memory.h"
#include "Addressing.h"

/**
 * @brief Skip template for reserving space in instruction encoding
 * @tparam N Number of bits to skip in instruction word
 */
template <size_t N>
struct Skip {};

template <typename>
struct is_skip : std::false_type {};

template <size_t N>
struct is_skip<Skip<N>> : std::true_type {};

template <typename T>
concept ValidArg = is_skip<T>::value || AddressingModeConcept<T>;

/**
 * @brief Base implementation for instruction decoding
 * 
 * @tparam CodeVal Opcode numeric value
 * @tparam CodeSize Opcode size in bits
 * @tparam CurrentOffset Current bit position in instruction word
 */
template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset, typename... Args>
requires (ValidArg<Args> && ...)
class InstructionImpl;

/**
 * @brief Terminal specialization of InstructionImpl (empty fields list)
 */
template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset>
class InstructionImpl<CodeVal, CodeSize, CurrentOffset> {
public:
    static constexpr auto fields = std::make_tuple(); ///< Empty fields tuple
    static constexpr size_t total_size = CurrentOffset; ///< Total instruction size
};

/**
 * @brief Specialization for handling addressing mode fields
 * 
 * @tparam Addressing Addressing mode template
 * @tparam Size Field size in bits
 * @tparam Rest Remaining field types
 */
template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset,
          template <size_t> class Addressing, size_t Size, typename... Rest>
        requires AddressingModeConcept<Addressing<Size>>
class InstructionImpl<CodeVal, CodeSize, CurrentOffset, Addressing<Size>, Rest...> {
private:
    using Next = InstructionImpl<CodeVal, CodeSize, CurrentOffset + Size, Rest...>;
public:
    /// Tuple of field descriptors (offset + addressing mode)
    static constexpr auto fields = std::tuple_cat(
        std::make_tuple(std::make_pair(CurrentOffset, Addressing<Size>())),
        Next::fields
    );
    static constexpr size_t total_size = Next::total_size; ///< Accumulated instruction size
};

/**
 * @brief Specialization for handling skipped bits
 * 
 * @tparam N Number of bits to skip
 * @tparam Rest Remaining field types
 */
template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset, size_t N, typename... Rest>
class InstructionImpl<CodeVal, CodeSize, CurrentOffset, Skip<N>, Rest...> {
private:
    using Next = InstructionImpl<CodeVal, CodeSize, CurrentOffset + N, Rest...>;
public:
    static constexpr auto fields = Next::fields; ///< Propagate fields list
    static constexpr size_t total_size = Next::total_size; ///< Propagated total size
};

/**
 * @brief Main instruction template class
 * 
 * @tparam CodeVal Numeric opcode value
 * @tparam CodeSize Opcode size in bits
 * @tparam Args Field components (addressing modes or skips)
 * 
 * Represents a machine instruction with:
 * - Fixed opcode and size
 * - Defined field structure
 * - Memory access capabilities
 */
template <size_t CodeVal, size_t CodeSize, typename... Args>
class Instruction {
    using Impl = InstructionImpl<CodeVal, CodeSize, CodeSize, Args...>;

public:
    static constexpr size_t code        = CodeVal;      ///< Instruction opcode value
    static constexpr size_t code_size   = CodeSize;     ///< Opcode size in bits
    static constexpr size_t total_size  = Impl::total_size; ///< Total instruction size
    static constexpr auto   fields      = Impl::fields; ///< Field descriptors tuple
    static constexpr size_t fields_count= std::tuple_size_v<decltype(fields)>; ///< Number of fields

    /**
     * @brief Get field descriptor by index
     * @tparam I Field index (0-based)
     * @return constexpr auto Field descriptor (offset + addressing mode)
     */
    template <size_t I>
    static constexpr auto get_field() {
        static_assert(I < fields_count, "Index out of range");
        return std::get<I>(fields);
    }
protected:
    /**
     * @brief Extract field bits from instruction word
     * @tparam I Field index
     * @param bits Full instruction word
     * @return Word<size> Extracted field bits
     */
    template <size_t I>
    static auto extract_field(const Word<total_size>& bits) {
        constexpr auto field  = get_field<I>();
        constexpr auto offset = field.first;
        constexpr auto addr   = field.second;
        constexpr auto size   = decltype(addr)::size;

        return Word<size>(bits.begin() + offset, bits.begin() + offset + size);
    }
};

/**
 * @brief A concept that defines the requirements for the number of arguments
 *
 * @tparam Count Required count of addressing modes
 * @tparam Ts Field components (addressing modes or skips)
 */
template <size_t Count, typename... Ts>
concept AddressingCount = ((0 + ... + (AddressingModeConcept<Ts> ? 1 : 0)) == Count);

/**
 * @brief One-argument instruction template class
 *
 * @tparam CodeVal Numeric opcode value
 * @tparam CodeSize Opcode size in bits
 * @tparam Args Field components (addressing modes or skips)
 */
template <size_t CodeVal, size_t CodeSize,typename... Args>
requires AddressingCount<1, Args...>
class SingleAddressInstruction : public Instruction<CodeVal, CodeSize, Args...>
{
    using Base = Instruction<CodeVal, CodeSize, Args...>;
    static constexpr auto field = Base::template get_field<0>();
public:
    template <BasicMemory MemType>
    static void execute(const Word<Base::total_size>& word, MemType& memory) {
        auto addr_bit = Base::template extract_field<0>(word);
        bits_t value_bit = field.second.read_by_address(addr_bit, memory);
    }
};

/**
 * @brief Two-argument instruction template class
 *
 * @tparam CodeVal Numeric opcode value
 * @tparam CodeSize Opcode size in bits
 * @tparam Args Field components (addressing modes or skips)
 */
template <size_t CodeVal, size_t CodeSize,typename... Args>
requires AddressingCount<2, Args...>
class TwoAddressInstruction : public Instruction<CodeVal, CodeSize, Args...>
{
    using Base = Instruction<CodeVal, CodeSize, Args...>;
    static constexpr auto field1 = Base::template get_field<0>();
    static constexpr auto field2 = Base::template get_field<1>();

public:
    template <BasicMemory MemType>
    static void execute(const Word<Base::total_size>& word, MemType& memory) {
        auto addr1_bit = Base::template extract_field<0>(word);
        auto addr2_bit = Base::template extract_field<1>(word);

        bits_t value1_bit = field1.second.read_by_address(addr1_bit, memory);
        bits_t value2_bit = field2.second.read_by_address(addr2_bit, memory);
    }
};

#endif //INSTRUCTION_H