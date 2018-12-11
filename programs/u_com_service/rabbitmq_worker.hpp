//
// Created by anton on 26.11.18.
//

#pragma once

#include <memory>
#include "SimplePocoHandler.h"
#include "thread_safe.hpp"

namespace uos{

    using std::string;

    struct rabbit_params{
        string   rabbit_host;
        uint16_t rabbit_port;
        string   rabbit_login;
        string   rabbit_password;
        string   rabbit_path;
        string   rabbit_input_queue;
        string   rabbit_output_queue;
    };

    class rabbitmq_worker{

        std::shared_ptr<thread_safe::threadsafe_queue<string>> rabbit_queue;
        string rb_queue_name;
        bool receiver = false;
        bool stop_transmit = false;
        rabbit_params __params;
        SimplePocoHandler *poco_handler;

    public:
        rabbitmq_worker(std::shared_ptr<thread_safe::threadsafe_queue<string>> &rb_out_queue,
                        bool __receiver,
                        const string &rb_address,
                        const uint16_t    &rb_port,
                        const string &rb_login,
                        const string &rb_password,
                        const string &rb_vhost,
                        const string &rb_queue)
                            {
                                receiver=__receiver;
                                rb_queue_name = rb_queue;
                                __params.rabbit_host = rb_address;
                                __params.rabbit_port = rb_port;
                                __params.rabbit_login = rb_login;
                                __params.rabbit_password = rb_password;
                                __params.rabbit_path = rb_vhost;
                                rabbit_queue = rb_out_queue;
                            };

        rabbitmq_worker(std::shared_ptr<thread_safe::threadsafe_queue<string>> &rb_out_queue,
                        bool __receiver,
                        rabbit_params params)
                            {
                                receiver=__receiver;
                                __params = params;
                                if(__receiver)
                                    rb_queue_name = params.rabbit_input_queue;
                                else
                                    rb_queue_name = params.rabbit_output_queue;
                                rabbit_queue = rb_out_queue;

                            };
        rabbitmq_worker(const rabbitmq_worker&) = delete;
        rabbitmq_worker& operator=(const rabbitmq_worker&) = delete;
        ~rabbitmq_worker(){}

        void run();
        void stop();

    };

}
