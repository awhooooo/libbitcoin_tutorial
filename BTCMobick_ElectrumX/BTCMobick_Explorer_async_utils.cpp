#include "BTCMobick_Explorer_async.hpp"
#include <iostream>
#include <fstream>
#include <boost/asio/io_context.hpp>

// Function to process a batch of blocks with retry logic
void process_batch_with_retries(net::io_context& ioc, int start_block, int end_block, std::ofstream& file, int max_retries = 3)
{
    std::vector<std::shared_ptr<EXPLORER_API_ASYNC::Explorer_API_Client>> clients;
    std::vector<std::future<nlohmann::json>> futures;
    std::vector<nlohmann::json> all_txids;

    for (int block_number = start_block; block_number <= end_block; ++block_number)
    {
        auto client = std::make_shared<EXPLORER_API_ASYNC::Explorer_API_Client>(ioc);
        clients.push_back(client);  // Store the client to maintain its lifetime
        futures.push_back(client->block(block_number));
    }

    const int num_threads = 4;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&ioc]() {
            ioc.run();
        });
    }

    for (int i = 0; i < futures.size(); ++i)
    {
        int retries = 0;
        while (retries < max_retries)
        {
            try
            {
                auto json_response = futures[i].get();  // This will block until the asynchronous operation completes
                for (const auto& txid : json_response["tx"])  // Assuming the transaction IDs are under the "tx" field
                {
                    all_txids.push_back(txid);
                }
                break;  // Break the loop if successful
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
                if (++retries < max_retries)
                {
                    std::cerr << "Retrying (" << retries << "/" << max_retries << ")..." << std::endl;
                    auto client = std::make_shared<EXPLORER_API_ASYNC::Explorer_API_Client>(ioc);
                    clients[i] = client;  // Replace the client
                    futures[i] = client->block(start_block + i);  // Retry the same block
                }
                else
                {
                    std::cerr << "Failed after " << max_retries << " retries" << std::endl;
                }
            }
        }
    }

    ioc.stop();
    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    nlohmann::json all_txids_json = all_txids;
    file << all_txids_json.dump(4);

    ioc.restart();  // Prepare the io_context for the next batch
}

// Function to process a batch of blocks with retry logic
void process_batch_with_retries(net::io_context& ioc, const std::vector<std::string>& txid_list, std::set<std::string>& unique_addresses, int max_retries = 3)
{
    std::vector<std::shared_ptr<EXPLORER_API_ASYNC::Explorer_API_Client>> clients;
    std::vector<std::future<nlohmann::json>> futures;

    for (const auto& txid : txid_list)
    {
        auto client = std::make_shared<EXPLORER_API_ASYNC::Explorer_API_Client>(ioc);
        clients.push_back(client);  // Store the client to maintain its lifetime
        futures.push_back(client->transaction(txid));
    }

    const int num_threads = 4;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&ioc]() {
            ioc.run();
        });
    }

    for (int j = 0; j < futures.size(); ++j)
    {
        int retries = 0;
        while (retries < max_retries)
        {
            try
            {
                auto json_response = futures[j].get();  // This will block until the asynchronous operation completes
                for (const auto& vout : json_response["vout"])  
                {
                    if (vout.contains("scriptPubKey") && vout["scriptPubKey"].contains("address"))
                    {
                        auto addr = vout["scriptPubKey"]["address"];
                        unique_addresses.insert(addr.get<std::string>());
                    }
                }
                break;  // Break the loop if successful
            }
            catch (const std::exception& e)
            {
                retries += 1;
                std::string error_msg = e.what();
                std::cerr << "Error: " << error_msg << std::endl;

                if (error_msg.find("partial message") != std::string::npos || error_msg.find("timeout") != std::string::npos)
                {
                    std::cerr << "Partial message error encountered. Skipping txid: " << txid_list[j] << std::endl;
                    break;  // Skip retrying for partial message errors
                }

                if (retries < max_retries)
                {
                    std::cerr << "Retrying (" << retries << "/" << max_retries << ")..." << std::endl;
                    auto client = std::make_shared<EXPLORER_API_ASYNC::Explorer_API_Client>(ioc);
                    clients[j] = client;  // Replace the client
                    futures[j] = client->transaction(txid_list[j]);  // Retry the same transaction
                    std::cout << txid_list[j] << "\n";
                    
                    // Handle specific error types to decide retry strategy
                    // if (error_msg.find("timeout") != std::string::npos)
                    // {
                    //     std::cerr << "Timeout error encountered. Retrying after a delay." << std::endl;
                    //     std::this_thread::sleep_for(std::chrono::seconds(2));  // Delay before retrying
                    // }
                }
                else
                {
                    std::cerr << "Failed after " << max_retries << " retries, skipping txid: " << txid_list[j] << std::endl;
                    break; 
                }
            }
        }
    }

    ioc.stop();
    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    ioc.restart();  // Prepare the io_context for the next batch
}

void save_checkpoint(const std::set<std::string>& unique_addresses, const std::unordered_set<std::string>& processed_txids)
{
    std::ofstream checkpoint_file("checkpoint.json");
    if (checkpoint_file.is_open()) {
        nlohmann::json checkpoint_data;
        checkpoint_data["unique_addresses"] = unique_addresses;
        checkpoint_data["processed_txids"] = processed_txids;
        checkpoint_file << checkpoint_data.dump(4);  // Pretty print with an indent of 4 spaces
        checkpoint_file.close();
        std::cout << "Checkpoint saved." << std::endl;
    } else {
        std::cerr << "Unable to open checkpoint file for writing." << std::endl;
    }
}

void load_checkpoint(std::set<std::string>& unique_addresses, std::unordered_set<std::string>& processed_txids)
{
    std::ifstream checkpoint_file("checkpoint.json");
    if (checkpoint_file.is_open()) {
        nlohmann::json checkpoint_data;
        checkpoint_file >> checkpoint_data;
        checkpoint_file.close();

        for (const auto& addr : checkpoint_data["unique_addresses"]) {
            unique_addresses.insert(addr.get<std::string>());
        }

        for (const auto& txid : checkpoint_data["processed_txids"]) {
            processed_txids.insert(txid.get<std::string>());
        }

        std::cout << "Checkpoint loaded." << std::endl;
    } else {
        std::cerr << "No checkpoint file found. Starting fresh." << std::endl;
    }
}
