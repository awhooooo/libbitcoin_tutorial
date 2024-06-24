#include "BTCMobick_ElectrumX.hpp"
#include <iostream>

using namespace ELECTRUMX;

int main()
{
    boost::asio::io_context electrumio;
    ElectrumX BTCMOBICK_wallet_server(electrumio, "220.85.71.15", 40008);
    std::cout << BTCMOBICK_wallet_server.block_header(111194, 0).dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.block_headers(111194, 10, 0).dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.estimate_fee(6).dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.headers_subscribe().dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.get_balance("bc1qdapypxj43wm3c9z0ke8jewe8ygv9yueef0rk3lhwggw89pdznyyq745sl5").dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.get_history("38YyTeMmGpV8oyAKkPjQjWDYCNqCHtSNw3").dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.get_mempool("38YyTeMmGpV8oyAKkPjQjWDYCNqCHtSNw3").dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.list_unspent("17swKRaQRhFiftSsGj2vC16Gfi7FHqTb85").dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.list_unspent("bc1pzhupp97yxu5pdj23evwg2f7vhfwqfr8k3t0ujr4majpy9uxqdk3spqq4zt").dump(4) << "\n" << std::endl;
    // std::cout << BTCMOBICK_wallet_server.broadcast_transaction("01000000015a93d0c1479895cab945b62e8ea1bd29c73e866da2e81a43b8bbf61eb1231e1400000000fd1e010047304402204c795c5d57e00a3adbb4c2da688bc5deb8ea435070e4a7f44a8175d59cc1c5ce022019a41a1d98e4d9b702d434d7e39d292cdce9857552ae68f235c88532cf168aa4014730440220268a0c0703fa399dde4616f7002cbf7878fda4d2e8fb261d2e9d656a72493829022001c2f08ee73eff91d33354e93f4c61846bf35febc01b9e808be2618093222c27014c8b522102e37aef8bb15ad7f072e1b8e44bc96f4b34f3958c7ff21638c7813ec44fbbb9d62103fec043cb84101a73f352224b1ae8bd6455e22ed84b7fbf07fa20000411d3a4a32103afd3f7a33d8396a159f09d57fc81c74f64126a40e1abaee9e6668f7f990c83b0210264330ef9b3382204d0b7f150008159927d9487100b3ea9fb86b8e1df1b8805cb54aeffffffff01c03fb5b30000000017a9144b437f02e67043098a955204e2873098e4ddc84a8700000000") << std::endl;
    std::cout << BTCMOBICK_wallet_server.get_transaction("315a109d4a0e801ee831d360e6445db4f1ab87b4c355713fc39ddf107309b029", true).dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.get_merkle("e67a0550848b7932d7796aeea16ab0e48a5cfe81c4e8cca2c5b03e0416850114", 111194).dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.id_from_pos(111194, 0, true).dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.get_fee_histogram().dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.server_banner().dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.server_donation().dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.server_features().dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.server_peers_subscribe().dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.server_ping().dump(4) << "\n" << std::endl;
    std::cout << BTCMOBICK_wallet_server.server_version("", "1.4").dump(4) << "\n" << std::endl;
    BTCMOBICK_wallet_server.disconnect();
    return 0;
}
