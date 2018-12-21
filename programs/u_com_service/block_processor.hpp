//
// Created by anton on 03.12.18.
//

#pragma once

#include <memory>
#include <thread>
#include <list>
#include "mongo_worker.hpp"
#include "rabbitmq_worker.hpp"
#include "rate_calculator.hpp"
#include "thread_safe.hpp"

namespace uos{

    using std::string;

    struct block_processor_params:
            public mongo_params,
            public rabbit_params,
            public uos_calculator_params
    {
        block_processor_params():mongo_params(),rabbit_params(),uos_calculator_params(){}
        string block_processor;
    };

    class block_processor{

        std::shared_ptr<mongo_worker> ptr_mongo;
        std::shared_ptr<rabbitmq_worker> ptr_rabbit_receiver;
        std::shared_ptr<rabbitmq_worker> ptr_rabbit_transmitter;
        std::shared_ptr<uos_calculator> ptr_calculator;
        std::shared_ptr<thread_safe::threadsafe_queue<string>> from_rabbit;
        std::shared_ptr<thread_safe::threadsafe_queue<string>> to_rabbit;
        std::map<uint64_t , fc::variant> block_cache;

        rabbit_params _rabbit_inpit_params;
        rabbit_params _rabbit_output_params;

        block_processor_params _all_params;
        mongo_params  _mongo_params;

        list<shared_ptr<thread>> th_list;
        bool exit = false;
        std::streambuf *out_buf;


    public:


        block_processor(block_processor_params _params);

        block_processor() = delete;
        block_processor(const block_processor&) = delete;
        block_processor& operator=(const block_processor&) = delete;

        void init(block_processor_params);
        void start();
        void stop();

        void process_command(fc::variant);

    };

}
