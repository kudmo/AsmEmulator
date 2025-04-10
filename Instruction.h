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

/**
 * @brief Base implementation for instruction decoding
 * 
 * @tparam CodeVal Opcode numeric value
 * @tparam CodeSize Opcode size in bits
 * @tparam CurrentOffset Current bit position in instruction word
 */
template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset, typename... Args>
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

    /**
     * @brief Execute instruction using specified memory
     * @tparam MemType Memory type satisfying BasicMemory concept
     * @param word Instruction word to decode
     * @param memory Memory system to operate on
     */
    template <BasicMemory MemType>
    static void execute(const Word<total_size>& word, MemType& memory) {
        execute_impl<MemType>(word, memory, std::make_index_sequence<fields_count>{});
    }

private:
    /**
     * @brief Implementation of instruction execution
     * @tparam MemType Memory type
     * @tparam Is Index sequence for field processing
     */
    template <BasicMemory MemType, size_t... Is>
    static void execute_impl(const Word<total_size>& bits,
                             MemType& memory,
                             std::index_sequence<Is...>)
    {
        ( get_field<Is>().second.read_by_address(
              extract_field<Is>(bits),
              memory
          ), ... );
    }

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

        Word<size> result(bits.begin() + offset, bits.begin() + offset + size);
        return result;
    }
};

#endif //INSTRUCTION_H