//
// Created by anton on 03.12.18.
//

#include <fc/io/json.hpp>
#include "block_processor.hpp"

namespace uos {

    block_processor::block_processor(uos::block_processor_params _params) {
        _all_params = _params;
        init(_params);
    }

    void block_processor::init(block_processor_params _params) {
        from_rabbit = std::make_shared<thread_safe::threadsafe_queue<string>>();
        to_rabbit = std::make_shared<thread_safe::threadsafe_queue<string>>();

        ptr_mongo = std::make_shared<mongo_worker>(mongo_params(_params));
        ptr_rabbit = std::make_shared<rabbitmq_worker>(from_rabbit, rabbit_params(_params));
//        ptr_calculator = std::make_shared<uos_calculator>(uos_calculator_params(_params));
        ptr_calculator = std::make_shared<uos_calculator>();
    }

    void block_processor::stop() {

        ptr_rabbit->stop();
        exit = true;
        for (auto item : th_list) {
            if (item != nullptr) {
                if (item->joinable()) {
                    item->join();
                    item.reset();
                }
            }
        }
        th_list.clear();
    }

    void block_processor::start() {

        if (ptr_mongo == nullptr) return;
        if (ptr_calculator == nullptr) return;
        if (ptr_rabbit == nullptr) return;
        if (from_rabbit == nullptr) return;
        if (to_rabbit == nullptr) return;

        if (!th_list.empty())
            stop();

//        std::shared_ptr<std::thread> th_rabbit(
//                [&]() {
//                    ptr_rabbit->run();
//                }
//
//        );

        th_list.push_back(make_shared<thread>([&]() {
            ptr_rabbit->run();
        }));


        try {
            std::string temp;
            while (!exit) {
                if (from_rabbit->try_pop(temp)) {
                    auto block = fc::json::from_string(temp);

                    if (block.get_object().contains("command")) {

                        if (block["command"].as_string() == "calculate") {
                            //todo: move to separate thread (process_command)
                            std::cout << "Start calculating" << std::endl;
                            auto blocknum = block["blocknum"].as_uint64();
                            auto begin = block["begin"].as_uint64();
                            auto end = block["end"].as_uint64();
                            if (block_cache.begin()->first > begin) {
                                auto blocks = ptr_mongo->get_blocks_range(begin, block_cache.begin()->first);
                                block_cache.insert(blocks.begin(), blocks.end());
                            }
                            if (block_cache.end()->first < end) {
                                auto blocks = ptr_mongo->get_blocks_range(block_cache.end()->first, end);
                                block_cache.insert(blocks.begin(), blocks.end());
                            }

                            uos::uos_calculator temp_calc((uos_calculator_params) _all_params);
                            temp_calc.start_block = begin;
                            temp_calc.end_block = end;
                            temp_calc.current_block = blocknum;

                            for (auto item: block_cache) {
                                if (begin <= item.first <= end) {
                                    temp_calc.parse_block_activity(item.second);
                                }
                            }
                            temp_calc.calculate();
                            ptr_mongo->put_results(fc::json::to_string(temp_calc.to_variant()));
                            //todo send result to rabbit

                            std::cout << "End calculating" << std::endl;
                            continue;
                        }

                        if (block["command"].as_string() == "save_balance"){
                            wlog("catch balances");
                            if(!block.get_object().contains("blocknum")){
                                wlog("Block num not found");
                                continue;
                            }
                            ptr_mongo->put_balances(temp);
                        }
                        wlog(temp);
                        continue;
                    }
                    if (!block.get_object().contains("blocknum"))
                        continue;
                    auto blocknum = block["blocknum"].as_uint64();
                    ptr_mongo->put_block(temp);
                    block_cache[blocknum] = block;
                } else {
                    this_thread::sleep_for(10ms);
                }
            }
        }
        catch (...) {
            elog("Mongo error");
            stop();
        }


    }

}
