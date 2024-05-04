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


std::string P2SH_custom()
{
	wallet::ec_private privateKey1("L2rBJ7KNsGtSfgyEGE3xoXQa4KNUoBB3FLRN4WbLawpRgqAHuVDA");
	wallet::ec_public publicKey1 = privateKey1.to_public();
    data_chunk pubKey1 = to_chunk(publicKey1.point());

	short_hash KeyHash1 = bitcoin_short_hash(pubKey1);
    data_chunk locktime;
    extend_data(locktime, to_little_endian<uint32_t>(1713352486));

    script redeem_script = script({operation(locktime), operation(opcode(177)), operation(opcode(117)), operation(opcode(118)), operation(opcode(169)), operation(to_chunk(KeyHash1)), operation(opcode(136)), operation(opcode(172))});

    wallet::payment_address from_address1("3HHxQgNK1kwnBGq3zqBn5s6y2PF1j7Axf9");
    wallet::payment_address to_address1("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");

    hash_digest prev_txid1;
    decode_hash(prev_txid1, "40a44e49011bde168cd4ea8eabd032bb1a99870ca3dae223fa92f9d5fa62dc33");

    uint32_t input_index1 = 0;
	output_point vin1(prev_txid1, input_index1);

	uint64_t input_value1;
	decode_base10(input_value1, "0.01", 8);

    //make Input
	input input1 = input();
	input1.set_previous_output(vin1);
	input1.set_sequence(0xfffffffe);
	script input_script1 = script::to_pay_script_hash_pattern(from_address1.hash());

    uint64_t output_value1;
	decode_base10(output_value1, "0.00999770", 8);
    script output_script1 = script().to_pay_key_hash_pattern(to_address1.hash());
	output output1(output_value1, output_script1);
	
	// Get current time as a time_point
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::system_clock::to_time_t(now);
    auto unix_time = static_cast<uint32_t>(epoch);

    chain::transaction tx = chain::transaction();
    tx.set_version(2);
    tx.set_locktime(unix_time - 100000);
	tx.inputs().push_back(input1);
	tx.outputs().push_back(output1); 

	endorsement sig1; 
	if (script::create_endorsement(sig1, privateKey1.secret(), redeem_script, tx, 0u, sighash_algorithm::all, script_version::unversioned, input_value1))
	{
		std::cout << "Signature: " << std::endl;
		std::cout << encode_base16(sig1) << "\n" << std::endl; 
	}

	assert(from_address1.hash() == bitcoin_short_hash(redeem_script.to_data(0)));

	operation::list scriptSig1; 
	scriptSig1.push_back(operation(sig1));
	scriptSig1.push_back(operation(pubKey1));
	scriptSig1.push_back(operation(redeem_script.to_data(0)));
	script unlockingScript1(scriptSig1);
	std::cout << unlockingScript1.to_string(0) << "\n" << std:: endl;

    //Make Signed TX
	tx.inputs()[0].set_script(unlockingScript1);
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

// compile command
// g++ -std=c++17 -o Nested-Segwit Nested-Segwit.cpp BTCMobick_ElectrumX/BTCMobick_ElectrumX.cpp bech32/bech32.cpp bech32/segwit_addr.cpp \
// -I/Users/legacy/C++_bitcoin/installation_prefix/include \
// -I/opt/homebrew/Cellar/openssl@3/3.2.1/include \
// -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include \
// $(pkg-config --cflags --libs libbitcoin-system libbitcoin-client libbitcoin-network libbitcoin-protocol libsecp256k1 gmp) \
// -L/Users/legacy/C++_bitcoin/installation_prefix/lib \
// -L/opt/homebrew/Cellar/openssl@3/3.2.1/lib \
// -lcrypto -Wno-deprecated
