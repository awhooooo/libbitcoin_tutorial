#include "BTCMobick_Explorer.hpp"

using namespace EXPLORER_API;

int main()
{
    Explorer_API explorer = Explorer_API();
    std::cout << explorer.get_utxo_set().dump(4) << std::endl;
    // std::cout << explorer.get_address("1FeexV6bAHb8ybZjqQMjJrcCrHGW9sb6uF").dump(4) << std::endl;
    // std::cout << explorer.get_next_block().dump(4) << std::endl;
    std::cout << explorer.get_unconfirmed_tx().dump(4) << std::endl;
    std::cout << explorer.get_transaction("163b2f4d7b0bb08fab9dcad8a0fb3c44daec328f3e9025b93a98a19ce0296fc4").dump(4) << std::endl;
    // std::cout << explorer.get_block(556759).dump(4) << std::endl;
    // std::cout << explorer.get_block_header(111194).dump(4) << std::endl;
    std::cout << explorer.get_height().dump(4) << std::endl;
    return 0;
}