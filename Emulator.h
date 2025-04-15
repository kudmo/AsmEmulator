#ifndef EMULATOR_H
#define EMULATOR_H

#include "Instruction.h"
#include <map>
#include <memory>


/**
 * @brief Concept for types that can be registered as instructions.
 *
 * A type \c T satisfies Executable<T, M> if it provides:
 *   - Four static constexpr size_t members:
 *     - \c code        : unique opcode
 *     - \c code_size   : bit-width of the opcode field
 *     - \c total_size  : total bit-width of the instruction
 *     - \c fields_count: number of subfields
 *   - A static \c execute(const bits_t&, M&) function
 *   - Is a class type
 *
 * @tparam T  The instruction type to check.
 * @tparam M  The memory model type.
 */
template <typename T, typename M>
concept Executable =
    requires {
        { T::code }        -> std::convertible_to<std::size_t>;
        { T::code_size }   -> std::convertible_to<std::size_t>;
        { T::total_size }  -> std::convertible_to<std::size_t>;
        { T::fields_count} -> std::convertible_to<std::size_t>;
    } &&
    requires(const bits_t& word, M& memory) {
        { T::execute(word, memory) };
    } &&
    std::is_class_v<T>;

/**
 * @brief A generic instruction-driven emulator.
 *
 * Manages registration and dispatch of instruction types satisfying
 * Executable<Instr, memory_type>.
 *
 * @tparam memory_type  A type satisfying MemoryWithInstructionPointer.
 */
template <typename memory_type> requires MemoryWithRegisters<memory_type> && InstructionPointerSystem<memory_type>
class Emulator {
private:
    /**
     * @brief Abstract base for instruction wrappers.
     */
    struct InstructionBase {
        /**
         * @brief Execute the instruction.
         * @param emulator Pointer to the owning emulator.
         * @param cmd      Raw instruction bits.
         */
        virtual void execute(Emulator* emulator, const bits_t& cmd) const = 0;
        virtual ~InstructionBase() = default;
    };

    /**
     * @brief Wraps a concrete instruction type \c Instr.
     *
     * @tparam Instr  Must satisfy Executable<Instr, memory_type>.
     */
    template <typename Instr>
    requires Executable<Instr, memory_type>
    struct InstructionWrapper : InstructionBase {
        void execute(Emulator* emulator, const bits_t& cmd) const override {
            Instr::execute(cmd, emulator->memory);
        }
    };

    /**
     * @brief Registers a single instruction type.
     *
     * @tparam Instr  Instruction type satisfying Executable<Instr, memory_type>.
     */
    template <typename Instr>
    requires Executable<Instr, memory_type>
    void register_one_instruction() {
        instructions[Instr::code] =
            std::make_unique<InstructionWrapper<Instr>>();
    }

    using InstructionPtr = std::unique_ptr<InstructionBase>;

    std::map<std::size_t, InstructionPtr> instructions;  ///< Opcode → instruction
    memory_type memory = memory_type();                  ///< Emulator memory state

    /**
     * @brief Dispatches a raw command to the registered instruction.
     * @param code  Opcode extracted from the instruction word.
     * @param cmd   Full instruction word.
     * @throws std::invalid_argument if the opcode is not registered.
     */
    void run_command(std::size_t code, const bits_t& cmd) {
        auto it = instructions.find(code);
        if (it != instructions.end()) {
            it->second->execute(this, cmd);
        } else {
            throw std::invalid_argument("Unknown instruction code");
        }
    }

public:
    /**
     * @brief Default-constructs the emulator with a default-initialized memory.
     */
    Emulator() = default;

    /**
     * @brief Constructs the emulator with a pre-initialized memory model.
     * @param mem  Initial memory state to use.
     */
    explicit Emulator(const memory_type& mem) : memory(mem) {}

    /**
     * @brief Registers a set of instruction types.
     *
     * Clears any previously registered instructions and registers each
     * provided \c Instrs in turn.
     *
     * @tparam Instrs  A parameter pack of types satisfying
     *                 Executable<Instr, memory_type>.
     * @return Reference to this emulator, for chaining.
     */
    template <typename... Instrs>
    requires (Executable<Instrs, memory_type> && ...)
    Emulator& setInstructions() {
        instructions.clear();
        (register_one_instruction<Instrs>(), ...);
        return *this;
    }
};

#endif //EMULATOR_H
