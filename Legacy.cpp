// => Use command "brew list" and check whether gmp and pkg-config is installed
// => assuming libbitcoin is installed with command "./install.sh --prefix=/home/me/myprefix --build-boost --disable-shared"
//    compile this cpp file with command "g++ -std=c++17 -o Legacy Legacy.cpp \
//    -I/home/me/myprefix/include -I/opt/homebrew/Cellar/openssl@3/3.2.1/include \
//    $(pkg-config --cflags --libs libbitcoin-system libbitcoin-protocol libbitcoin-client libsecp256k1 gmp) \
//    -L/home/me/myprefix/lib -L/opt/homebrew/Cellar/openssl@3/3.2.1/lib -lcrypto -Wno-deprecated"

#include <bitcoin/system.hpp>

using namespace libbitcoin;
using namespace libbitcoin::wallet;
using namespace libbitcoin::machine;
using namespace libbitcoin::chain;

void Legacy1()
{
	wallet::ec_private privateKey1("secret for obvious reason 1");
	
	data_chunk pubKey1 = to_chunk(privateKey1.to_public().point());
	
	wallet::payment_address from_address1("Legacy spending address");
	wallet::payment_address to_address1("Legacy receiving address");
	
	script input_script1 = script().to_pay_key_hash_pattern(from_address1.hash());
	script input_script2 = script().to_pay_key_hash_pattern(from_address1.hash());
	hash_digest prev_txid1;
	hash_digest prev_txid2;
	
	decode_hash(prev_txid1, "txid-1");
	decode_hash(prev_txid2, "txid-2");
	
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
	if (input_script1.create_endorsement(sig1, privateKey1.secret(), input_script1, tx, 0u, all))
	{
		std::cout << "Signature: " << std::endl;
		std::cout << encode_base16(sig1) << "\n" << std::endl; 
	}
	
	endorsement sig2; 
	if (input_script2.create_endorsement(sig2, privateKey1.secret(), input_script2, tx, 1u, all))
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

void Legacy2()
{
    	wallet::ec_private privateKey1("secret for obvious reason 2");

	data_chunk pubKey1 = to_chunk(privateKey1.to_public().point());
	
	wallet::payment_address from_address1("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");
	wallet::payment_address to_address1("16vnUfDxdMWYKYpw4gCqkwCHRy7jtzgJgn");
    	wallet::payment_address to_address2("3BCs2cczEkumrucjSdAT6RF6ozEzjp4tTP"); // Nested Segwit
    	wallet::payment_address to_address3("3HHxQgNK1kwnBGq3zqBn5s6y2PF1j7Axf9"); // P2SH Legacy
    	wallet::payment_address to_address4("33rik7q3qvsm3xUq5T94d3atN9BqJHirx2"); // Nested Segwit Multisig
	// libbitcoin-system doesn't support witness addresses
    	std::string to_address5("bc1q62qmcqa5jqw7r34ms3rgfad7068funr55wpx94"); // P2WPKH
    	std::string to_address6("bc1qdapypxj43wm3c9z0ke8jewe8ygv9yueef0rk3lhwggw89pdznyyq745sl5"); // P2WSH
    	std::string to_address7("bc1q573uh8868utzw09whkwqe9emxkx3drkuyrky7am8cejvvjeddvkqvkj0ae"); // P2WSH Multisig
	
	script input_script1 = script().to_pay_key_hash_pattern(from_address1.hash());
	hash_digest prev_txid1;
	
	decode_hash(prev_txid1, "0ef41f0fbcd34a6b2a976041b70051bdbf0dfa1e28651ad71167ff5eb2304cdd");
	
	uint32_t input_index1 = 1;
	output_point vin1(prev_txid1, input_index1);
	
	//make Input
	input input1 = input();
	input1.set_previous_output(vin1);
	input1.set_sequence(0xffffffff);
	
	uint64_t output_value1;
	decode_base10(output_value1, "28.4", 8);
	script output_script1 = script().to_pay_key_hash_pattern(to_address1.hash());
	output output1(output_value1, output_script1);
	
	uint64_t output_value2;
	decode_base10(output_value2, "0.01", 8);
	script output_script2 = script().to_pay_script_hash_pattern(to_address2.hash());
	output output2(output_value2, output_script2);
	
	uint64_t output_value3;
	decode_base10(output_value3, "0.01", 8);
	script output_script3 = script().to_pay_script_hash_pattern(to_address3.hash());
	output output3(output_value3, output_script3);
	
	uint64_t output_value4;
	decode_base10(output_value4, "0.01", 8);
	script output_script4 = script().to_pay_script_hash_pattern(to_address4.hash());
	output output4(output_value4, output_script4);
	
	std::vector<uint8_t> witnessprogram1 = segwit_addr::decode("bc", to_address5).second;
	assert(witnessprogram1.size() == 20);
	script output_script5 = script({operation(opcode(0)), operation(to_chunk(vectorToArray<20>(witnessprogram1)))});
	uint64_t output_value5;
	decode_base10(output_value5, "0.01", 8);
	output output5(output_value5, output_script5);
	
	std::vector<uint8_t> witnessprogram2 = segwit_addr::decode("bc", to_address6).second;
	assert(witnessprogram2.size() == 32);
	script output_script6 = script({operation(opcode(0)), operation(to_chunk(vectorToArray<32>(witnessprogram2)))});
	uint64_t output_value6;
	decode_base10(output_value6, "0.01", 8);
	output output6(output_value6, output_script6);
	
	std::vector<uint8_t> witnessprogram3 = segwit_addr::decode("bc", to_address7).second;
	assert(witnessprogram3.size() == 32);
	script output_script7 = script({operation(opcode(0)), operation(to_chunk(vectorToArray<32>(witnessprogram3)))});
	uint64_t output_value7;
	decode_base10(output_value7, "0.01", 8);
	output output7(output_value7, output_script7);
	
	chain::transaction tx = chain::transaction();
	tx.set_version(2);
	tx.set_locktime(0);
	tx.inputs().push_back(input1);
	tx.outputs().push_back(output1);
	tx.outputs().push_back(output2);
	tx.outputs().push_back(output3);
	tx.outputs().push_back(output4);
	tx.outputs().push_back(output5);
	tx.outputs().push_back(output6);
	tx.outputs().push_back(output7);
	
	endorsement sig1; 
	if (input_script1.create_endorsement(sig1, privateKey1.secret(), input_script1, tx, 0u, sighash_algorithm::all))
	{
	std::cout << "Signature: " << "\n";
	std::cout << encode_base16(sig1) << "\n\n"; 
	}
	
	//make Sig Script
	operation::list scriptSig1; 
	scriptSig1.push_back(operation(sig1));
	scriptSig1.push_back(operation(pubKey1));
	script unlockingScript1(scriptSig1);
	std::cout << unlockingScript1.to_string(0) << "\n\n";
	
	//Make Signed TX
	tx.inputs()[0].set_script(unlockingScript1);
	std::cout << "Raw Transaction: " << "\n";
	std::cout << encode_base16(tx.to_data(1)) << "\n\n";
}

int main()
{
	Legacy1();
	Legacy2();
	return EXIT_SUCCESS;
}
