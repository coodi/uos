//
// Created by anton on 26.11.18.
//

#include "rabbitmq_worker.hpp"

namespace uos{


    void rabbitmq_worker::run(std::ostream &out = std::cout) {
        rb_channel.consume(rb_queue_name, AMQP::noack).onReceived(
                [&](const AMQP::Message &message,
                    uint64_t deliveryTag,
                    bool redelivered)
                {
                    auto received_json = std::string(message.body(),message.bodySize());
                    out<<" [x] Received "
                       << received_json
                       << std::endl;
                    if(received_json.size()>0){
                        rabbit_queue->push(std::string(message.body(),message.bodySize()));
                        std::cout<<"*"<<std::endl;
                    }
                });
        std::cout << " [*] Waiting for messages. To exit press CTRL-C"<<std::endl;
        poco_handler.loop();

    }
    void rabbitmq_worker::stop() {
        std::cout<<"Rabbit quit"<<std::endl;
        poco_handler.quit();
    }

}
