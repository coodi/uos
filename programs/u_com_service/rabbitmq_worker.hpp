//
// Created by anton on 26.11.18.
//

#pragma once

#include <memory>
#include "SimplePocoHandler.h"
#include "thread_safe.hpp"

namespace uos{

    class rabbitmq_worker{

        std::shared_ptr<thread_safe::threadsafe_queue<std::string>> rabbit_queue;

        SimplePocoHandler poco_handler;
        AMQP::Connection rb_connection;
        AMQP::Channel rb_channel;
        std::string rb_queue_name;


    public:
        rabbitmq_worker(std::shared_ptr<thread_safe::threadsafe_queue<std::string>> &rb_out_queue,
                        const std::string &rb_address,
                        const uint16_t    &rb_port,
                        const std::string &rb_login,
                        const std::string &rb_password,
                        const std::string &rb_vhost,
                        const std::string &rb_queue):
                poco_handler(rb_address,rb_port),
                rb_connection(&poco_handler, AMQP::Login(rb_login, rb_password), rb_vhost),
                rb_channel(&rb_connection)
                            {
                                rb_queue_name = rb_queue;
                                rb_channel.declareQueue(rb_queue);
                                rabbit_queue = rb_out_queue;
                            };

        rabbitmq_worker(const rabbitmq_worker&) = delete;
        rabbitmq_worker& operator=(const rabbitmq_worker&) = delete;
        ~rabbitmq_worker(){stop();}

        void run(std:: ostream &out);
        void stop();

    };

}
