#include <bitcoin/system.hpp>
#include <ostream>

using namespace libbitcoin;
using namespace libbitcoin::wallet;
using namespace libbitcoin::machine;
using namespace libbitcoin::chain;

short_hash makeHashLock(const std::string& secret)
{
    data_chunk secretChunk = to_chunk(secret);
    short_hash hashLock = bitcoin_short_hash(secretChunk);
    return hashLock;
}

int main()
{
    // A wants to swap his tBTC to BTCMobick
    wallet::ec_private privateKeyA("testnet private key wif");
    wallet::ec_public pubKeyA = privateKeyA.to_public();
    short_hash KeyHashA = bitcoin_short_hash(pubKeyA.point());
    wallet::payment_address addressA = pubKeyA.to_payment_address(wallet::payment_address::testnet_p2kh);
    wallet::payment_address addressA_1("mainnet legacy address");

    // B wants to swap his BTCMobick to tBTC
    wallet::ec_private privateKeyB("mainnet private key wif");
    wallet::ec_public pubKeyB = privateKeyB.to_public();
    short_hash KeyHashB = bitcoin_short_hash(pubKeyB.point());
    wallet::payment_address addressB = pubKeyB.to_payment_address(wallet::payment_address::mainnet_p2kh);
    wallet::payment_address addressB_1("testnet legacy address");

    short_hash password = makeHashLock("paranoid");
    data_chunk locktimeA;
    extend_data(locktimeA, to_little_endian<uint32_t>(1714839143));
    // OP_IF OP_HASH160 [HASH SECRET] OP_EQUALVERIFY OP_DUP OP_HASH160 [B PUBKEYHASH] OP_ELSE [nLocktime] OP_CHECKLOCKTIMEVERIFY OP_DROP OP_DUP OP_HASH160 [A PUBKEYHASH] OP_ENDIF OP_EQUALVERIFY OP_CHECKSIG
    script redeem_scriptA = script({operation(opcode::if_), operation(opcode::hash160), operation(to_chunk(password)), operation(opcode::equalverify), 
                                    operation(opcode::dup), operation(opcode::hash160), operation(to_chunk(KeyHashB)), operation(opcode::else_), 
                                    operation(locktimeA), operation(opcode::checklocktimeverify), operation(opcode::drop), operation(opcode::dup), operation(opcode::hash160), 
                                    operation(to_chunk(KeyHashA)), operation(opcode::endif), operation(opcode::equalverify), operation(opcode::checksig)});

    data_chunk locktimeB;
    extend_data(locktimeB, to_little_endian<uint32_t>(1714832423));
    // OP_IF OP_HASH160 [HASH SECRET] OP_EQUALVERIFY OP_DUP OP_HASH160 [A PUBKEYHASH] OP_ELSE [nLocktime] OP_CHECKLOCKTIMEVERIFY OP_DROP OP_DUP OP_HASH160 [B PUBKEYHASH] OP_ENDIF OP_EQUALVERIFY OP_CHECKSIG
    script redeem_scriptB = script({operation(opcode::if_), operation(opcode::hash160), operation(to_chunk(password)), operation(opcode::equalverify), 
                                    operation(opcode::dup), operation(opcode::hash160), operation(to_chunk(KeyHashA)), operation(opcode::else_), 
                                    operation(locktimeB), operation(opcode::checklocktimeverify), operation(opcode::drop), operation(opcode::dup), operation(opcode::hash160), 
                                    operation(to_chunk(KeyHashB)), operation(opcode::endif), operation(opcode::equalverify), operation(opcode::checksig)});

    // A will send his tBTC to contract address A => so encoding with testnet prefix
    std::cout << redeem_scriptA.to_string(0) << std::endl;
    wallet::payment_address contract_addressA = payment_address(redeem_scriptA, payment_address::testnet_p2sh);
    std::cout << "=======> " << contract_addressA.encoded() << std::endl;

    // B will send his BTCMobick to contract address B => so encoding with mainnet prefix
    std::cout << redeem_scriptB.to_string(0) << std::endl;
    wallet::payment_address contract_addressB = payment_address(redeem_scriptB, payment_address::mainnet_p2sh);
    std::cout << "=======> " << contract_addressB.encoded() << std::endl;


    
    // B redeems his tBTC from contract_address_A to addressB_1
    hash_digest prev_txid1;
    decode_hash(prev_txid1, "testnet bitcoin transaction id");

    uint32_t input_index1 = 0;
    output_point vin1(prev_txid1, input_index1);

    uint64_t input_value1;
    decode_base10(input_value1, "0.00099620", 8);

    //make Input
    input input1 = input();
    input1.set_previous_output(vin1);
    input1.set_sequence(0xfffffffe);

    uint64_t output_value1;
    decode_base10(output_value1, "0.00095000", 8);
    script output_script1 = script().to_pay_key_hash_pattern(addressB_1.hash());
    output output1(output_value1, output_script1);

    chain::transaction tx1 = chain::transaction();
    tx1.set_version(2);
    tx1.set_locktime(0);
    tx1.inputs().push_back(input1);
    tx1.outputs().push_back(output1); 

    endorsement sig1; 
    script::create_endorsement(sig1, privateKeyB.secret(), redeem_scriptA, tx1, 0u, sighash_algorithm::all, script_version::unversioned, input_value1);

    std::string text1("paranoid");
    data_chunk secret_password1(text1.begin(), text1.end());

    operation::list scriptSig1;
    scriptSig1.push_back(operation(sig1));
    scriptSig1.push_back(operation(to_chunk(pubKeyB.point())));
    scriptSig1.push_back(secret_password1);
    scriptSig1.push_back(operation(opcode::push_positive_1));
    scriptSig1.push_back(operation(redeem_scriptA.to_data(0)));
    script unlockingScript1(scriptSig1);
    std::cout << unlockingScript1.to_string(0) << "\n" << std:: endl;

    //Make Signed TX
    tx1.inputs()[0].set_script(unlockingScript1);
    std::cout << "Raw Transaction: " << std::endl;
    std::cout << encode_base16(tx1.to_data(true, true)) << "\n" << std::endl;
    

    
    // A redeems his BTCMobick from contract_addressB to addresssA_1
    hash_digest prev_txid2;
    decode_hash(prev_txid2, "mainnet bitcoin transaction id");

    uint32_t input_index2 = 0;
    output_point vin2(prev_txid2, input_index2);

    uint64_t input_value2;
    decode_base10(input_value2, "0.001", 8);

    //make Input
    input input2 = input();
    input2.set_previous_output(vin2);
    input2.set_sequence(0xfffffffe);

    uint64_t output_value2;
    decode_base10(output_value2, "0.00099600", 8);
    script output_script2 = script().to_pay_key_hash_pattern(addressA_1.hash());
    output output2(output_value2, output_script2);

    chain::transaction tx2 = chain::transaction();
    tx2.set_version(2);
    tx2.set_locktime(0);
    tx2.inputs().push_back(input2);
    tx2.outputs().push_back(output2); 

    endorsement sig2; 
    script::create_endorsement(sig2, privateKeyA.secret(), redeem_scriptB, tx2, 0u, sighash_algorithm::all, script_version::unversioned, input_value2);
	
    std::string text2("paranoid");
    data_chunk secret_password2(text2.begin(), text2.end());

    operation::list scriptSig2; 
    scriptSig2.push_back(operation(sig2));
    scriptSig2.push_back(operation(to_chunk(pubKeyA.point())));
    scriptSig2.push_back(secret_password2);
    scriptSig2.push_back(operation(opcode::push_positive_1));
    scriptSig2.push_back(operation(redeem_scriptB.to_data(0)));
    script unlockingScript2(scriptSig2);
    std::cout << unlockingScript2.to_string(0) << "\n" << std:: endl;

    //Make Signed TX
    tx2.inputs()[0].set_script(unlockingScript2);
    std::cout << "Raw Transaction: " << std::endl;
    std::cout << encode_base16(tx2.to_data(true, true)) << std::endl;
    
    std::cout << std::endl;
    
    // A redeems his tBTC from contract_addressA to addresssA_1
    hash_digest prev_txid3;
    decode_hash(prev_txid3, "testnet bitcoin transaction id");

    uint32_t input_index3 = 0;
    output_point vin3(prev_txid3, input_index3);

    uint64_t input_value3;
    decode_base10(input_value3, "0.00099620", 8);

    //make Input
    input input3 = input();
    input3.set_previous_output(vin3);
    input3.set_sequence(0xfffffffe);

    uint64_t output_value3;
    decode_base10(output_value3, "0.00093000", 8);
    script output_script3 = script().to_pay_key_hash_pattern(addressA_1.hash());
    output output3(output_value3, output_script3);

    // Get current time as a time_point
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::system_clock::to_time_t(now);
    auto unix_time = static_cast<uint32_t>(epoch);

    chain::transaction tx3 = chain::transaction();
    tx3.set_version(2);
    tx3.set_locktime(unix_time - 30000);
    tx3.inputs().push_back(input3);
    tx3.outputs().push_back(output3); 

    endorsement sig3; 
    script::create_endorsement(sig3, privateKeyA.secret(), redeem_scriptA, tx3, 0u, sighash_algorithm::all, script_version::unversioned, input_value3);

    operation::list scriptSig3;
    scriptSig3.push_back(operation(sig3));
    scriptSig3.push_back(operation(to_chunk(pubKeyA.point())));
    scriptSig3.push_back(operation(opcode::push_size_0));
    scriptSig3.push_back(operation(redeem_scriptA.to_data(0)));
    script unlockingScript3(scriptSig3);
    std::cout << unlockingScript3.to_string(0) << "\n" << std:: endl;

    //Make Signed TX
    tx3.inputs()[0].set_script(unlockingScript3);
    std::cout << "Raw Transaction: " << std::endl;
    std::cout << encode_base16(tx3.to_data(true, true)) << "\n" << std::endl;



    // B redeems his BTCMobick from contract_addressB to addresssB_1
    hash_digest prev_txid4;
    decode_hash(prev_txid4, "mainnet bitcoin transaction id");

    uint32_t input_index4 = 0;
    output_point vin4(prev_txid4, input_index4);

    uint64_t input_value4;
    decode_base10(input_value4, "0.001", 8);

    //make Input
    input input4 = input();
    input4.set_previous_output(vin4);
    input4.set_sequence(0xfffffffe);

    uint64_t output_value4;
    decode_base10(output_value4, "0.00099600", 8);
    script output_script4 = script().to_pay_key_hash_pattern(addressB_1.hash());
    output output4(output_value4, output_script4);

    chain::transaction tx4 = chain::transaction();
    tx4.set_version(2);
    tx4.set_locktime(unix_time - 30000);
    tx4.inputs().push_back(input4);
    tx4.outputs().push_back(output4); 

    endorsement sig4; 
    script::create_endorsement(sig4, privateKeyB.secret(), redeem_scriptB, tx4, 0u, sighash_algorithm::all, script_version::unversioned, input_value4);
	
    operation::list scriptSig4; 
    scriptSig4.push_back(operation(sig4));
    scriptSig4.push_back(operation(to_chunk(pubKeyB.point())));
    scriptSig4.push_back(operation(opcode::push_size_0));
    scriptSig4.push_back(operation(redeem_scriptB.to_data(0)));
    script unlockingScript4(scriptSig4);
    std::cout << unlockingScript4.to_string(0) << "\n" << std:: endl;

    //Make Signed TX
    tx4.inputs()[0].set_script(unlockingScript4);
    std::cout << "Raw Transaction: " << std::endl;
    std::cout << encode_base16(tx4.to_data(true, true)) << std::endl;
    

    return 0;
}
