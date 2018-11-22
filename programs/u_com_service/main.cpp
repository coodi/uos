#include <iostream>
#include <fstream>
#include <fc/variant_object.hpp>
#include <fc/io/json.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <csignal>
#include "SimplePocoHandler.h"

#include <amqpcpp.h>

std::function<void(int)> shutdown_handler;
void signal_handler(int signal) { shutdown_handler(signal); }

static int PROGRAMM_STOP=0;

int main(int argc, char** argv){

    std::streambuf * str_buf;
    std::ofstream str_of;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    namespace bpo = boost::program_options;

    bool verbose = false;

    bpo::options_description desc("General u_com_service options");

    desc.add_options()
            ("verbose,v",bpo::bool_switch(&verbose)," run program in foreground")
            ;
    bpo::parsed_options parsed = bpo::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    bpo::variables_map vmap;
    bpo::store(parsed,vmap);
    bpo::notify(vmap);


    std::cout<<"verbose: "<<verbose<<std::endl;
    pid_t pid,sid;
    int rv;
    if(!verbose){
        pid=fork();
        if(pid<0) {
            std::cerr << "Cannot create for process" << std::endl;
            exit(EXIT_FAILURE);
        }
        if(pid>0){
            exit(EXIT_SUCCESS);
        }
        str_of.open("log.log");
        str_buf = str_of.rdbuf();
    }
    else{
        str_buf = std::cout.rdbuf();
    }

    std::ostream out(str_buf);

    SimplePocoHandler handler("localhost", 5672);

    shutdown_handler = [&](int signal_number){
        handler.quit();
        std::cout<<"Gotcha!"<<std::endl;
        PROGRAMM_STOP = 1;
    };

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareQueue("hello");
    channel.consume("hello", AMQP::noack).onReceived(
            [&](const AMQP::Message &message,
               uint64_t deliveryTag,
               bool redelivered)
            {
                std::string(message.body());
                out<<" [x] Received "
                          << std::string(message.body(),message.bodySize())
                          << std::endl;
            });

    //todo: make the 1-st thread
    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";
    handler.loop();


    return 0;
}