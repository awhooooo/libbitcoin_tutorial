#include <bitcoin/system.hpp>
#include <ostream>
#include "bech32/bech32.h"
#include "bech32/segwit_addr.h"
#include "BTCMobick_ElectrumX/BTCMobick_ElectrumX.hpp"

using namespace libbitcoin;
using namespace libbitcoin::wallet;
using namespace libbitcoin::machine;
using namespace libbitcoin::chain;
using namespace ELECTRUMX;

std::string P2SH_P2WPKH(void)
{
    wallet::ec_private privateKey1("secret for obvious reason");

    data_chunk pubKey1 = to_chunk(privateKey1.to_public().point());
    short_hash keyHash1 = bitcoin_short_hash(pubKey1);

    wallet::payment_address from_address1("3BCs2cczEkumrucjSdAT6RF6ozEzjp4tTP");
    wallet::payment_address to_address1("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");

   script script_code1 = script().to_pay_key_hash_pattern(keyHash1);
   script witness_script1 = script({operation(opcode(0)), operation(to_chunk(keyHash1))});
	
    hash_digest prev_txid1;
    decode_hash(prev_txid1, "0188044a1e8fc492316281f44cb91a43ee279df4b830213fba77374fa92b539b");

    uint32_t input_index1 = 0;
    output_point vin1(prev_txid1, input_index1);

    //make Input
    input input1 = input();
    input1.set_previous_output(vin1);
    input1.set_sequence(0xffffffff);

    uint64_t output_value1;
    decode_base10(output_value1, "0.00999800", 8);
    script output_script1 = script().to_pay_key_hash_pattern(to_address1.hash());
    output output1(output_value1, output_script1);

    chain::transaction tx = chain::transaction();
    tx.set_version(2);
    tx.set_locktime(0);
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output1); 

    endorsement sig1; 
    if (script::create_endorsement(sig1, privateKey1.secret(), script_code1, tx, 0u, sighash_algorithm::all, script_version::zero, 1000000))
    {
	std::cout << "Signature: " << std::endl;
	std::cout << encode_base16(sig1) << "\n" << std::endl; 
    }

    // make witness
    data_stack witness1; 
    witness1.push_back(to_chunk(sig1));
    witness1.push_back(to_chunk(pubKey1));
    witness txinwitness1(witness1);
    std::cout << txinwitness1.to_string() << "\n" << std:: endl;

    // Make Signed TX
    tx.inputs()[0].set_script(script(to_chunk(witness_script1.to_data(1)), 0));
    tx.inputs()[0].set_witness(txinwitness1);
    std::cout << "Raw Transaction: " << std::endl;

    std::cout << encode_base16(tx.to_data(true, true)) << std::endl;

    return encode_base16(tx.to_data(true, true));
}


int main()
{
    boost::asio::io_context electrumio;
    ElectrumX BTCMOBICK_wallet_server(electrumio, "220.85.71.15", 40008);
    
    std::string transaction1 = P2SH_P2WPKH();
    std::cout << BTCMOBICK_wallet_server.broadcast_transaction(transaction1) << std::endl;
	
    return 0;
}
