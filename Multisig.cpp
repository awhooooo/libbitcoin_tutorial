#include <bitcoin/system.hpp>
#include <iostream>
#include <cassert>
#include <string>

#include "bech32/bech32.h"
#include "bech32/segwit_addr.h"

using namespace libbitcoin;
using namespace libbitcoin::wallet;
using namespace libbitcoin::machine;
using namespace libbitcoin::chain;

wallet::ec_private privateKey1("secret for obvious reason 1");
wallet::ec_private privateKey2("secret for obvious reason 2");
wallet::ec_private privateKey3("secret for obvious reason 3");

wallet::ec_public publicKey1 = privateKey1.to_public();
wallet::ec_public publicKey2 = privateKey2.to_public();
wallet::ec_public publicKey3 = privateKey3.to_public();

// m to n multisig p2sh address
std::string P2SH_multisig(int m = 2, int n = 3) 
{
    assert(m < n);
    std::vector<ec_secret> privkey;
    std::vector<byte_array<ec_compressed_size>> pubkey;

    for (int i = 0; i < n; i++) {
        data_chunk seed(16);
        pseudo_random_fill(seed);
        ec_secret secretKey = bitcoin_hash(seed);

        std::string hexKey = encode_base16(secretKey);
        std::cout << "secret key " << i << ": " << hexKey << std::endl;

        wallet::ec_private privateKey(secretKey);
        std::cout << "Private key " << i << ": " << privateKey.encoded() << std::endl;

        wallet::ec_public publicKey = privateKey.to_public();
        std::cout << "Compressed Public Key " << i << ": " << publicKey.encoded() << std::endl;

        privkey.push_back(secretKey);
        pubkey.push_back(publicKey);
    }

    chain::script multiSig_script = chain::script(chain::script().to_pay_multisig_pattern(m, pubkey));
    std::cout << multiSig_script.to_string(0) << std::endl;
    std::cout << wallet::payment_address(multiSig_script).encoded() << std::endl; // for testnet, add 0xc4

    return wallet::payment_address(multiSig_script).encoded();
}

script multisig_script_and_addresses()
{
    std::vector<byte_array<ec_compressed_size>> vpubkey;
    vpubkey.reserve(3);
    vpubkey.push_back(publicKey1);
    vpubkey.push_back(publicKey2);
    vpubkey.push_back(publicKey3);

    script multisig_script = script().to_pay_multisig_pattern(2, vpubkey);
    std::cout << multisig_script.to_string(0) << "\n";
    std::string P2SH_multisig_address = wallet::payment_address(multisig_script).encoded();
    std::cout << "P2SH_multisig_address: " << P2SH_multisig_address << "\n";

    /** libbitcoin version 3 doesn't support witness addresses */
    data_chunk witness_program = sha256_hash_chunk(multisig_script.to_data(0));
    std::string P2WSH_multisig_address = segwit_addr::encode("bc", 0, witness_program);
    std::cout << "P2WSH_multisig_address: " << P2WSH_multisig_address << "\n";

    return multisig_script;
}

std::string P2SH_multisig_spend(void)
{
    wallet::payment_address from_address("33rik7q3qvsm3xUq5T94d3atN9BqJHirx2");
    wallet::payment_address to_address("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");

    script multisig_script = multisig_script_and_addresses();
    assert(from_address.hash() == bitcoin_short_hash(multisig_script.to_data(0)));

    hash_digest prev_txid1;
    decode_hash(prev_txid1, "db0b27dac0dd2f27339b08e3c68317dec30ae80f8a9e78d4d7659e53b07919c4");

    uint32_t input_index1 = 1;
	  output_point vin1(prev_txid1, input_index1);

    uint64_t input_value1;
	  decode_base10(input_value1, "0.01", 8);

    //make Input
	  input input1 = input();
	  input1.set_previous_output(vin1);
	  input1.set_sequence(0xffffffff);
	  script input_script1 = script::to_pay_script_hash_pattern(from_address.hash());

    uint64_t output_value1;
	  decode_base10(output_value1, "0.00999600", 8);
    script output_script1 = script().to_pay_key_hash_pattern(to_address.hash());
	  output output1(output_value1, output_script1);

    chain::transaction tx = chain::transaction();
    tx.set_version(2);
    tx.set_locktime(0);
  	tx.inputs().push_back(input1);
  	tx.outputs().push_back(output1);

    /** signature types are SigVersion::BASE */
    endorsement sig1; 
  	if (script::create_endorsement(sig1, privateKey1.secret(), multisig_script, tx, 0u, sighash_algorithm::all, script_version::unversioned, input_value1))
  	{
  		std::cout << "Signature #1: " << std::endl;
  		std::cout << encode_base16(sig1) << "\n" << std::endl; 
  	}
  
    endorsement sig2; 
  	if (script::create_endorsement(sig2, privateKey2.secret(), multisig_script, tx, 0u, sighash_algorithm::all, script_version::unversioned, input_value1))
  	{
  		std::cout << "Signature #2: " << std::endl;
  		std::cout << encode_base16(sig2) << "\n" << std::endl; 
  	}
  
    endorsement sig3; 
  	if (script::create_endorsement(sig3, privateKey3.secret(), multisig_script, tx, 0u, sighash_algorithm::all, script_version::unversioned, input_value1))
  	{
  		std::cout << "Signature #3: " << std::endl;
  		std::cout << encode_base16(sig3) << "\n" << std::endl; 
  	}

    operation::list scriptSig1;
    scriptSig1.reserve(4);
    data_chunk empty_sig;
  	scriptSig1.push_back(operation(empty_sig)); // 1st private key won't be needed
  	scriptSig1.push_back(operation(sig2)); 
    scriptSig1.push_back(operation(sig3));
	  scriptSig1.push_back(operation(multisig_script.to_data(0)));
    script unlockingScript1(scriptSig1);
	
    //Make Signed TX
  	tx.inputs()[0].set_script(unlockingScript1);
  	std::cout << "Raw Transaction: " << std::endl;
  	std::cout << encode_base16(tx.to_data(true, true)) << std::endl;
  
  	return encode_base16(tx.to_data(true, true));
	
}

std::string P2WSH_multisig_spend(void)
{
    /** libbitcoin version 3 doesn't support witness addresses */
    std::string from_address = "bc1q573uh8868utzw09whkwqe9emxkx3drkuyrky7am8cejvvjeddvkqvkj0ae";
    wallet::payment_address to_address("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");

    script multisig_script = multisig_script_and_addresses();
    std::vector<uint8_t> witness_program;
    witness_program.reserve(32);
    witness_program = segwit_addr::decode("bc", from_address).second;
    assert(witness_program == sha256_hash_chunk(multisig_script.to_data(0)));

    hash_digest prev_txid1;
    decode_hash(prev_txid1, "db0b27dac0dd2f27339b08e3c68317dec30ae80f8a9e78d4d7659e53b07919c4");

    uint32_t input_index1 = 0;
	  output_point vin1(prev_txid1, input_index1);

    uint64_t input_value1;
	  decode_base10(input_value1, "0.01", 8);

    //make Input
  	input input1 = input();
  	input1.set_previous_output(vin1);
  	input1.set_sequence(0xfffffffe);

    uint64_t output_value1;
	  decode_base10(output_value1, "0.00999600", 8);
    script output_script1 = script().to_pay_key_hash_pattern(to_address.hash());
	  output output1(output_value1, output_script1);

    chain::transaction tx = chain::transaction();
    tx.set_version(2);
    tx.set_locktime(0);
  	tx.inputs().push_back(input1);
  	tx.outputs().push_back(output1); 

    /** signature types are SigVersion::WITNESS_VO */
    endorsement sig1; 
  	if (script::create_endorsement(sig1, privateKey1.secret(), multisig_script, tx, 0u, sighash_algorithm::all, script_version::zero, 1000000))
  	{
  		std::cout << "Signature #1: " << std::endl;
  		std::cout << encode_base16(sig1) << "\n" << std::endl; 
  	}

    endorsement sig2; 
  	if (script::create_endorsement(sig2, privateKey2.secret(), multisig_script, tx, 0u, sighash_algorithm::all, script_version::zero, 1000000))
  	{
  		std::cout << "Signature #2: " << std::endl;
  		std::cout << encode_base16(sig2) << "\n" << std::endl; 
  	}

    endorsement sig3; 
  	if (script::create_endorsement(sig3, privateKey3.secret(), multisig_script, tx, 0u, sighash_algorithm::all, script_version::zero, 1000000))
  	{
  		std::cout << "Signature #3: " << std::endl;
  		std::cout << encode_base16(sig3) << "\n" << std::endl; 
  	}

    //make witness data
	  data_stack witness1;
    witness1.reserve(4);
    data_chunk empty_sig;
    witness1.push_back(empty_sig); // 1st private key won't be needed
  	witness1.push_back(to_chunk(sig2));
  	witness1.push_back(to_chunk(sig3));
    witness1.push_back(to_chunk(multisig_script.to_data(0)));
	  witness txinwitness1(witness1);
	
    // Make Signed TX
	  tx.inputs()[0].set_witness(txinwitness1);
    std::cout << "Raw Transaction: " << std::endl;

	  std::cout << encode_base16(tx.to_data(true, true)) << std::endl;

    return encode_base16(tx.to_data(true, true));

}

int main()
{
    P2SH_multisig_spend();
    std::cout << "\n";
    P2WSH_multisig_spend();
    return EXIT_SUCCESS;
}
