#include <iostream>
#include <fstream>
#include <fc/variant_object.hpp>
#include <fc/io/json.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <csignal>
#include <thread>
#include <chrono>
#include "SimplePocoHandler.h"
#include "block_processor.hpp"

#include <amqpcpp.h>

#include <bsoncxx/json.hpp>
#include <fc/log/logger.hpp>

std::function<void(int)> shutdown_handler;
void signal_handler(int signal) { shutdown_handler(signal); }

static int PROGRAMM_STOP=0;




int main(int argc, char** argv) {

    std::streambuf *str_buf;
    std::ofstream str_of;
    std::shared_ptr<thread_safe::threadsafe_queue<std::string>> q_from_rabbit(
            new thread_safe::threadsafe_queue<std::string>);

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    namespace bpo = boost::program_options;

    bool verbose = false;

    bpo::options_description desc("General u_com_service options");

    desc.add_options()
            ("verbose,v", bpo::bool_switch(&verbose), " run program in foreground")

            ("rabbit-host",         bpo::value<std::string>()->default_value("localhost"),  "RabbitMQ host")
            ("rabbit-input-queue",  bpo::value<std::string>()->default_value("hello"),      "RabbitMQ input data queue")
            ("rabbit-output-queue",  bpo::value<std::string>()->default_value("goodbye"),    "RabbitMQ output data queue")
            ("rabbit-path",         bpo::value<std::string>()->default_value("/"),      "RabbitMQ input/output data queue path")
            ("rabbit-login",  bpo::value<std::string>()->default_value("guest"),    "RabbitMQ login")
            ("rabbit-password",  bpo::value<std::string>()->default_value("guest"),    "RabbitMQ password")
            ("rabbit-port",  bpo::value<uint16_t >()->default_value(5672),    "RabbitMQ port")

            ("mongo-uri",  bpo::value<std::string>()->default_value("mongodb://localhost"),    "MongoDB URI")
            ("mongo-connection",  bpo::value<std::string>()->default_value("testbase"),    "MongoDB Connection name")
            ("mongo-user",  bpo::value<std::string>()->default_value(""),    "MongoDB user name")
            ("mongo-password",  bpo::value<std::string>()->default_value(""),    "MongoDB user password")
            ("mongo-db-blocks",  bpo::value<std::string>()->default_value("blocks_from_rabbit"),    "MongoDB collection for saving blocks")
            ("mongo-db-balances",  bpo::value<std::string>()->default_value("balances"),    "MongoDB collection for saving balances")
            ("mongo-db-resuls",  bpo::value<std::string>()->default_value("results"),    "MongoDB collection for saving calculation results")
            ;

    bpo::parsed_options parsed = bpo::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    bpo::variables_map vmap;
    bpo::store(parsed, vmap);
    bpo::notify(vmap);


    std::cout << "verbose: " << verbose << std::endl;
    pid_t pid, sid;
    int rv;
    if (!verbose) {
        pid = fork();
        if (pid < 0) {
            std::cerr << "Cannot create for process" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }
        str_of.open("log.log");
        str_buf = str_of.rdbuf();
    } else {
        str_buf = std::cout.rdbuf();
    }

    for(auto item : vmap){
        std::cout<<item.first<<std::endl;//<<" "<<item.second.as<std::string>()<<std::endl;
    }

    uos::block_processor_params _params;

    _params.rabbit_host = vmap.at("rabbit-host").as<std::string>();
    _params.rabbit_input_queue = vmap.at("rabbit-input-queue").as<std::string>();
    _params.rabbit_output_queue = vmap.at("rabbit-output-queue").as<std::string>();
    _params.rabbit_path = vmap.at("rabbit-path").as<std::string>();
    _params.rabbit_login = vmap.at("rabbit-login").as<std::string>();
    _params.rabbit_password = vmap.at("rabbit-password").as<std::string>();
    _params.rabbit_port = vmap.at("rabbit-port").as<uint16_t >();

    _params.mongo_password = vmap.at("mongo-user").as<std::string>();
    _params.mongo_user = vmap.at("mongo-password").as<std::string>();
    _params.mongo_db_results = vmap.at("mongo-db-resuls").as<std::string>();
    _params.mongo_db_blocks = vmap.at("mongo-db-blocks").as<std::string>();
    _params.mongo_db_balances = vmap.at("mongo-db-balances").as<std::string>();
    _params.mongo_connection_name = vmap.at("mongo-connection").as<std::string>();
    _params.mongo_uri = vmap.at("mongo-uri").as<std::string>();


//
//    _params.rabbit_host = "localhost";
//    _params.rabbit_input_queue = "hello";
//    _params.rabbit_output_queue = "goodbye";
//    _params.rabbit_path = "/";
//    _params.rabbit_login = "guest";
//    _params.rabbit_password = "guest";
//    _params.rabbit_port = 5672;
//    _params.mongo_password = "";
//    _params.mongo_user = "";
//    _params.mongo_db_results = "results";
//    _params.mongo_db_blocks = "blocks_from_rabbit";
//    _params.mongo_db_balances = "balances";
//    _params.mongo_connection_name = "testbase";
//    _params.mongo_uri = "mongodb://localhost";

    uos::block_processor test(_params);


    shutdown_handler = [&](int signal_number) {
        test.stop();
        std::cout << "Gotcha!" << std::endl;
        PROGRAMM_STOP = 1;
    };

    test.start();



/// 1-st thread starts

//    std::thread t_rabbit([&](){
//        rabbitmq_input.run(out);
//    });
//    std::cout<<std::endl<<"Rabbit runs away!"<<std::endl;

//todo: mongo connection


//
//    using namespace bsoncxx::types;
//    using bsoncxx::builder::basic::make_document;
//    using bsoncxx::builder::basic::kvp;
//
//    std::string test = "{\"blocknum\":6409458,\"block_timestamp\":\"2018-11-30T13:25:06.500\",\"transactions\":[{\"transaction_id\":\"d334a02853fbd6d6289fdc3a39dd515adb31463f3976fc3556e1a95b7206617c\",\"actions\":[]},{\"transaction_id\":\"eec25f9857d55211025248e74d096bc8e86139a704ffe76c928af5728072d7ed\",\"actions\":[{\"account\":\"uos.activity\",\"action\":\"usertouser\",\"data\":{\"acc_from\":\"liveislifeop\",\"acc_to\":\"nevermindyou\",\"interaction_type_id\":5},\"receiver\":[{\"actor\":\"liveislifeop\",\"permission\":\"active\"}]}]}]}";
//
//    uos::mongo_worker mongoose("mongodb://localhost","testbase","blocks_from_rabbit","results","balances");
//    std::cout<<fc::json::to_string(mongoose.get_block(6409445))<<std::endl;
//
//    mongoose.put_block(test);
//
//    std::cout<<fc::json::to_string(mongoose.get_block(6409445))<<std::endl;

    /// The mongocxx::instance constructor and destructor initialize and shut down the driver,
    /// respectively. Therefore, a mongocxx::instance must be created before using the driver and
    /// must remain alive for as long as the driver is in use.
//
//    mongocxx::instance inst{};
//
//    mongocxx::client mongo_conn{mongocxx::uri{"mongodb://localhost"}};
//    {
//    try{
//        auto db = mongo_conn["testbase"];
//        std::cout<<"here"<<std::endl;
//
//        ///delete database
//
//        db["trx"].drop();
//
//        /// create index
//        db["trx"].create_index( make_document( kvp( "block_id" , 1 )));
//
//        /// 1-st example
//        auto doc = make_document( kvp( "block_num",10 ),kvp("block_id", 12), kvp("memo","hello"));
//        db["trx"].insert_one(doc.view());
//
//        /// 2-nd example
//        db["trx"].insert_one(bsoncxx::from_json( R"xxx({ "block_id" : 10, "block_num" : 15, "memo" : "vasya" })xxx").view());
//
//        /// 3-rd example
//        auto trans_traces_doc = bsoncxx::builder::basic::document{};
//        trans_traces_doc.append(kvp( "block_num",20 ));
//        trans_traces_doc.append(bsoncxx::builder::concatenate_doc{bsoncxx::from_json( R"xxx({ "vasya" : "loh", "sdfsdf" : "sdfsd", "memo" : "vasya2" })xxx")});
//        db["trx"].insert_one(trans_traces_doc.view());
//
//
//    }
//    catch (...){
//        std::cout<<"Mongo error"<<std::endl;
//    }
//    }

//    uos::uos_calculator calculator;
//    calculator.set_bounds(0, 10000000, 10000000);
//    singularity::parameters_t params;
//    calculator.set_params_activity(params);
//    calculator.postprocessing_social   = [] (uos::activity_map_t& map){
//        std::cout<<" social postprocessing"<<std::endl;
//
//
//        return map;
//    };
//    calculator.postprocessing_transfer = [] (uos::activity_map_t& map){
//        std::cout<<" transfer postprocessing"<<std::endl;
//        return map;
//    };

//    while (!PROGRAMM_STOP) {
//        std::string temp;
//        std::cout << ".";
//        if (q_from_rabbit->try_pop(temp)) {
//            std::cout << "->";
//            auto block = fc::json::from_string(temp);
//
///// catch command "calculate" {
//            if( (block.get_object().contains("command")) && (block["command"].as_string()=="calculate")){
//
//                int c = ' ';
//                if(verbose) {
//                    do {
//                        std::cout << "Calculate? " << std::endl;
//                        std::cin >> c;
//                        c = std::tolower(c);
//                        if (PROGRAMM_STOP) break;
//                    } while (!((c == 'n') || (c == 'y')));
//                }
//                if (!verbose||(c=='y')){
//                    calculator.calculate();
//                    std::cout<<fc::json::to_string(calculator.to_variant())<<std::endl;
//                    //todo: output result
//                }
//                continue;
//            }
///// } catch command "calculate"
//
//            calculator.parse_block_activity(block);
//
//            std::cout << calculator.social_calculator->get_total_handled_block_count() << std::endl;
//            std::cout << calculator.transfer_calculator->get_total_handled_block_count() << std::endl;
//        } else {
////            std::cout << "+" << q_from_rabbit->size();
//            std::this_thread::sleep_for(std::chrono::milliseconds(10));
//        }
//    }

/// 2-nd thread starts
//
//    std::thread t_mongoose([&](){
//        try{
//            auto db = mongo_conn["testbase"];
//            std::cout<<"here"<<std::endl;
//            std::string temp;
//            while (!PROGRAMM_STOP) {
//                std::cout<<".";
//                if(q_from_rabbit->try_pop(temp)){
//                    std::cout<<"+";
//                    if(db["blocks_from_rabbit"].indexes().list().begin() == db["blocks_from_rabbit"].indexes().list().end()){
//                        db["blocks_from_rabbit"].create_index(make_document(kvp("blocknum",1)));
//                    }
//                    db["blocks_from_rabbit"].insert_one(bsoncxx::from_json(temp));
//                } else{
//                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
//                }
//            }
//        }
//        catch(...){
//            std::cout<<"Mongo error"<<std::endl;
//            signal_handler(SIGTERM);
//        }
//    });

//    if(t_rabbit.joinable())
//        t_rabbit.join();
//    if(t_mongoose.joinable())
//        t_mongoose.join();
//}
    return 0;
}