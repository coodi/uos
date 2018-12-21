//
// Created by anton on 26.11.18.
//
#include <thread>
#include <fc/exception/exception.hpp>
#include <thread>
#include <chrono>
#include "rabbitmq_worker.hpp"

namespace uos {


    void rabbitmq_worker::run() {

        if(receiver) {
            std::cout<<"Run receiver"<<std::endl;
            if (rabbit_queue == nullptr) {
                std::cout << "Rabbit queue not initialized" << std::endl;
                return;
            }
            SimplePocoHandler handler(__params.rabbit_host ,__params.rabbit_port);
            AMQP::Connection connection(&handler, AMQP::Login(__params.rabbit_login, __params.rabbit_password), __params.rabbit_path);
            AMQP::Channel channel(&connection);
            channel.consume(rb_queue_name, AMQP::noack).onReceived(
                    [&](const AMQP::Message &message,
                        uint64_t deliveryTag,
                        bool redelivered) {
                        auto received_json = std::string(message.body(), message.bodySize());
                        if (received_json.size() > 0) {
                            rabbit_queue->push(std::string(message.body(), message.bodySize()));
//                        if(rabbit_queue->size()>100000)
//                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                    });
            std::cout << " [*] Waiting for messages. To exit press CTRL-C" << std::endl;
            poco_handler = &handler;
            handler.loop();
        }


        else{
            std::cout<<"Run transmitter"<<std::endl;
            if (rabbit_queue == nullptr) {
                std::cout << "Rabbit queue not initialized" << std::endl;
                return;
            }
            string temp;

            while(!stop_transmit){

                if(rabbit_queue->try_pop(temp)){
                    SimplePocoHandler handler(__params.rabbit_host ,__params.rabbit_port);
                    AMQP::Connection connection(&handler, AMQP::Login(__params.rabbit_login, __params.rabbit_password), __params.rabbit_path);
                    AMQP::Channel channel(&connection);
                    channel.onReady([&](){
                        if(handler.connected()){
                            channel.publish("",rb_queue_name,temp);
                            handler.quit();
                        }
                        else{
                            elog("Handler not connected");
                        }
                    });
                    handler.loop();

                } else{
                    std::this_thread::sleep_for( std::chrono::milliseconds(100) );
                }
            }

        }

    }

    void rabbitmq_worker::stop() {
        if(receiver) {
            if(poco_handler)
            poco_handler->quit();
        }
        else{
            stop_transmit = true;
        }
    }

}
