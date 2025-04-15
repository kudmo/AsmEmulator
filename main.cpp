#include "Emulator.h"

using Mul = TwoAddressInstruction<
    0x0,2,
    RegisterAddressing<3>,
    RegisterAddressing<3>
>;
using Add = TwoAddressInstruction<
    0x1,2,
    DirectAddressing<3>,
    RegisterAddressing<3>
>;


int main() {
    std::vector<bool> cmd = {0,0,0,1,0,1,0,0};
    auto emulator = Emulator<ComputerMemory<4,32,8>>()
                    .setInstructions<Mul,Add>();

    return 0;
}
