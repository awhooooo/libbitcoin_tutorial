#include <bitcoin/system.hpp>
#include "BTCMobick_ElectrumX/BTCMobick_ElectrumX.hpp"

using namespace libbitcoin;
using namespace libbitcoin::wallet;
using namespace libbitcoin::machine;
using namespace libbitcoin::chain;
using namespace ELECTRUMX;

std::string P2WSH(void)
{
	wallet::ec_private privateKey1("secret for obvious reason 1");
	wallet::ec_public publicKey1 = privateKey1.to_public();
	
	data_chunk pubKey1 = to_chunk(privateKey1.to_public().point());
	
	short_hash KeyHash = bitcoin_short_hash(publicKey1.point());
	data_chunk locktime;
	extend_data(locktime, to_little_endian<uint32_t>(1713352486));
	
	script witness_script = script({operation(locktime), operation(opcode(177)), operation(opcode(117)), operation(opcode(118)), operation(opcode(169)), operation(to_chunk(KeyHash)), operation(opcode(136)), operation(opcode(172))});
	std::string from_address1 = "bc1qdapypxj43wm3c9z0ke8jewe8ygv9yueef0rk3lhwggw89pdznyyq745sl5";
	wallet::payment_address to_address1("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");
	
	hash_digest prev_txid1;
	decode_hash(prev_txid1, "259e72177f337180074b002cb138f2f066ebd531e4c1f7ea8e6b8a474e2583ea");
	
	uint32_t input_index1 = 0;
	output_point vin1(prev_txid1, input_index1);
	
	//make Input
	input input1 = input();
	input1.set_previous_output(vin1);
	input1.set_sequence(0xfffffffe);
	
	uint64_t output_value1;
	decode_base10(output_value1, "0.00999800", 8);
	script output_script1 = script().to_pay_key_hash_pattern(to_address1.hash());
	output output1(output_value1, output_script1);
	
	// Get current time as a time_point
	auto now = std::chrono::system_clock::now();
	auto epoch = std::chrono::system_clock::to_time_t(now);
	auto unix_time = static_cast<uint32_t>(epoch);

	chain::transaction tx = chain::transaction();
	tx.set_version(2);
	tx.set_locktime(unix_time - 50000);
	tx.inputs().push_back(input1);
	tx.outputs().push_back(output1); 
	
	endorsement sig1; 
	if (witness_script.create_endorsement(sig1, privateKey1.secret(), witness_script, tx, 0u, sighash_algorithm::all, script_version::zero, 1000000))
	{
		std::cout << "Signature: " << std::endl;
		std::cout << encode_base16(sig1) << "\n" << std::endl; 
	}
	
	//make Sig Script
	data_stack witness1; 
	witness1.push_back(to_chunk(sig1));
	witness1.push_back(to_chunk(pubKey1));
	witness1.push_back(to_chunk(witness_script.to_data(0)));
	witness txinwitness1(witness1);
	std::cout << txinwitness1.to_string() << "\n" << std:: endl;
	
	// Make Signed TX
	tx.inputs()[0].set_witness(txinwitness1);
	std::cout << "Raw Transaction: " << std::endl;
	
	std::cout << encode_base16(tx.to_data(true, true)) << std::endl;
	
	return encode_base16(tx.to_data(true, true));
}

std::string P2WPKH(void)
{
	wallet::ec_private privateKey1("secret for obvious reason 2");
	wallet::ec_public publicKey1 = privateKey1.to_public();
	
	data_chunk pubKey1 = to_chunk(privateKey1.to_public().point());
	
	short_hash KeyHash = bitcoin_short_hash(publicKey1.point());
	script witness_script = script({operation(opcode(0)), operation(to_chunk(KeyHash))});
	std::string from_address1 = "bc1q62qmcqa5jqw7r34ms3rgfad7068funr55wpx94";
	wallet::payment_address to_address1("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");
	
	hash_digest prev_txid1;
	decode_hash(prev_txid1, "c34b7ecf6a53f30e699219660deb69abc385a68b725a7dabdb0503f17c79016d");
	
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
	
	script p2wpkh_script_code = script::to_pay_key_hash_pattern(KeyHash);
	std::cout << p2wpkh_script_code.to_string(0) << std::endl;
	
	endorsement sig1; 
	// script::create_endorsement(sig1, privateKey1.secret(), p2wpkh_script_code, tx, 0u, sighash_algorithm::all, script_version::zero, 1000000);
	if (witness_script.create_endorsement(sig1, privateKey1.secret(), p2wpkh_script_code, tx, 0u, sighash_algorithm::all, script_version::zero, 1000000))
	{
		std::cout << "Signature: " << std::endl;
		std::cout << encode_base16(sig1) << "\n" << std::endl; 
	}
	
	//make Sig Script
	data_stack witness1; 
	witness1.push_back(sig1);
	witness1.push_back(pubKey1);
	witness txinwitness1(witness1);
	std::cout << txinwitness1.to_string() << "\n" << std:: endl;
	
	// Make Signed TX
	tx.inputs()[0].set_witness(txinwitness1);
	std::cout << "Raw Transaction: " << std::endl;
	
	std::cout << encode_base16(tx.to_data(true, true)) << std::endl;
	
	return encode_base16(tx.to_data(true, true));
}

int main()
{
	boost::asio::io_context electrumio;
	ElectrumX BTCMOBICK_wallet_server(electrumio, "220.85.71.15", 40008);
	
	std::string transaction1 = P2WSH();
	std::cout << BTCMOBICK_wallet_server.broadcast_transaction(transaction1) << std::endl;
	std::cout << "================================================================" << std::endl;
	std::string transaction2 = P2WPKH();
	std::cout << BTCMOBICK_wallet_server.broadcast_transaction(transaction2) << std::endl;
	
	return 0;
}
