#include <bitcoin/system.hpp>
#include "bech32.h"
#include "segwit_addr.h"

using namespace libbitcoin;
using namespace libbitcoin::wallet;
using namespace libbitcoin::machine;
using namespace libbitcoin::chain;

int main()
{
    wallet::ec_private privateKey1("secret for obvious reason 1");

    data_chunk pubKey1 = to_chunk(privateKey1.to_public().point());

    wallet::payment_address from_address1("1EKzgpZGX2zBxeWw5Dw5zMGYHVUiV21MZP");
    wallet::payment_address to_address1("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");

    script input_script1 = script().to_pay_key_hash_pattern(from_address1.hash());
    script input_script2 = script().to_pay_key_hash_pattern(from_address1.hash());
    hash_digest prev_txid1;
    hash_digest prev_txid2;

	  decode_hash(prev_txid1, "7d72e75ad7de3fbec42155a65d7de3b5f9dcbda735776729ab98d16e3c525a16");
    decode_hash(prev_txid2, "f587b144e98699da701e5de7586dfce01e158fd09e51e8ab16a69af9400e83e4");

    uint32_t input_index1 = 0;
    uint32_t input_index2 = 0;
	  output_point vin1(prev_txid1, input_index1);
    output_point vin2(prev_txid2, input_index2);

    //make Input
  	input input1 = input();
  	input1.set_previous_output(vin1);
  	input1.set_sequence(0xffffffff);

    input input2 = input();
  	input2.set_previous_output(vin2);
  	input2.set_sequence(0xffffffff);

    uint64_t output_value1;
	  decode_base10(output_value1, "0.29999600", 8);
    script output_script1 = script().to_pay_key_hash_pattern(to_address1.hash());
	  output output1(output_value1, output_script1);

    std::string messageString = "Libbitcoin testing 2024-04-27";
  	data_chunk data(messageString.size());
  	auto source = make_safe_deserializer(data.begin(), data.begin() + messageString.size());
  	auto sink = make_unsafe_serializer(data.begin());
  	sink.write_string(messageString);
  
  	const auto nullData = source.read_bytes(messageString.size());
  	std::cout << "Message: " << std::endl;
  	std::cout << encode_base16(nullData) << std::endl;
  
  	output output2 = output();
  	output2.set_script(script(script().to_null_data_pattern(nullData)));
  	output2.set_value(0);

	  chain::transaction tx = chain::transaction();
    tx.set_version(2);
    tx.set_locktime(0);
  	tx.inputs().push_back(input1);
  	tx.inputs().push_back(input2);
  	tx.outputs().push_back(output1);
    tx.outputs().push_back(output2); 

    endorsement sig1; 
  	if(input_script1.create_endorsement(sig1, privateKey1.secret(), input_script1, tx, 0u, all))
  	{
  		std::cout << "Signature: " << std::endl;
  		std::cout << encode_base16(sig1) << "\n" << std::endl; 
  	}
  
      endorsement sig2; 
  	if(input_script2.create_endorsement(sig2, privateKey1.secret(), input_script2, tx, 1u, all))
  	{
  		std::cout << "Signature: " << std::endl;
  		std::cout << encode_base16(sig2) << "\n" << std::endl; 
  	}

    //make Sig Script
  	operation::list scriptSig1; 
  	scriptSig1.push_back(operation(sig1));
  	scriptSig1.push_back(operation(pubKey1));
  	script unlockingScript1(scriptSig1);
  	std::cout << unlockingScript1.to_string(0) << "\n" << std:: endl;

    operation::list scriptSig2; 
  	scriptSig2.push_back(operation(sig2));
  	scriptSig2.push_back(operation(pubKey1));
  	script unlockingScript2(scriptSig2);
  	std::cout << unlockingScript2.to_string(0) << "\n" << std:: endl;

    //Make Signed TX
	  tx.inputs()[0].set_script(unlockingScript1);
    tx.inputs()[1].set_script(unlockingScript2);
  	std::cout << "Raw Transaction: " << std::endl;
  	std::cout << encode_base16(tx.to_data()) << std::endl;
	
    return 0;
}
