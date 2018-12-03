//
// Created by anton on 30.11.18.
//

#pragma once
#include <string>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/query_exception.hpp>

namespace uos{

    using std::string;

    class mongo_worker{

        bool                        put_by_uniq_blocknum(const string &_val,const string &_db);
        fc::mutable_variant_object  get_by_blocknum(const uint32_t &_blocknum, const string &_db);

    public:
        string uri;
        string connection_name;
        string db_blocks;
        string db_results;
        string db_balances;
        string user;
        string password;
        bool connected = false;

        mongocxx::client mongo_conn;
        mongocxx::instance inst{};

        mongo_worker(){}
        mongo_worker(const char* _uri, const char* _connection_name, const char* _db_blocks, const char* _db_results, const char* _db_balances)
                :uri(_uri),
                 connection_name(_connection_name),
                 db_blocks(_db_blocks),
                 db_results(_db_results),
                 db_balances(_db_balances)
        {
            mongo_conn = mongocxx::client{mongocxx::uri(uri)};
            connected = true;
            recheck_indexes();
        }

        void connect(){
            mongo_conn = mongocxx::client{mongocxx::uri(uri)};
            connected = true;
            recheck_indexes();
        }

        void recheck_indexes();

        fc::mutable_variant_object  get_block(const uint32_t &_blocknum);
        bool    put_block(const string& __val);

        fc::mutable_variant_object  get_balances(const uint32_t& _blocknum);
        bool    put_balances(const string& __val);

        fc::mutable_variant_object  get_results(const uint32_t& _blocknum);
        bool    put_results(const string& __val);


    };

}
