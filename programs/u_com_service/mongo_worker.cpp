//
// Created by anton on 30.11.18.
//

#include <bsoncxx/json.hpp>
#include <sstream>
#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/variant_object.hpp>
#include <fc/exception/exception.hpp>
#include "mongo_worker.hpp"

namespace uos{

    using mongocxx::make_document;
    using mongocxx::kvp;

    bool mongo_worker::put_by_uniq_blocknum(const std::string &_val, const std::string &_db) {
        if(!connected)
            connect();
        try {
            fc::variant block = fc::json::from_string(_val, fc::json::legacy_parser, 3);
            block.get_object();
            bool found = false;
            if (block.get_object().contains("blocknum")) {
                try {
                    auto cursor = mongo_conn[connection_name][_db].find(
                            make_document(kvp("blocknum", block.get_object()["blocknum"].as_int64())));
                    if(cursor.begin()!=cursor.end()){
                        mongo_conn[connection_name][_db].delete_one(
                                make_document(kvp("blocknum", block.get_object()["blocknum"].as_int64())));
                        found = true;
                    }
                }
                catch (mongocxx::query_exception exception) {
                    found = false;
                }
                mongo_conn[connection_name][_db].insert_one(bsoncxx::from_json(_val));
                return true;
            }
        }
        catch (fc::exception exception){
            std::cout<<exception.to_string()<<std::endl;
        }
        return false;
    }

    fc::mutable_variant_object mongo_worker::get_by_blocknum(const uint32_t &_blocknum, const std::string &_db) {
        if(!connected)
            connect();
        auto cursor = mongo_conn[connection_name][_db].find(make_document(kvp("blocknum",int64_t (_blocknum))));
        fc::mutable_variant_object temp;
        uint32_t i = 0;
        for (auto&& doc : cursor) {
            temp[std::to_string(i)] = fc::json::from_string(bsoncxx::to_json(doc));
        }
        return temp;
    }

    fc::mutable_variant_object mongo_worker::get_block(const uint32_t &_blocknum) {
        return get_by_blocknum(_blocknum,db_blocks);
    }
    fc::mutable_variant_object mongo_worker::get_balances(const uint32_t &_blocknum) {
        return get_by_blocknum(_blocknum,db_balances);
    }
    fc::mutable_variant_object mongo_worker::get_results(const uint32_t &_blocknum) {
        return get_by_blocknum(_blocknum,db_results);
    }


    bool mongo_worker::put_block(const std::string &__val) {
        return put_by_uniq_blocknum(__val,db_blocks);
    }
    bool mongo_worker::put_balances(const std::string &__val) {
        return put_by_uniq_blocknum(__val,db_balances);
    }
    bool mongo_worker::put_results(const std::string &__val) {
        return put_by_uniq_blocknum(__val,db_results);
    }

    void mongo_worker::recheck_indexes() {
        if (!connected)
            connect();
        if (mongo_conn[connection_name][db_blocks].indexes().list().begin() ==
            mongo_conn[connection_name][db_blocks].indexes().list().end())
            mongo_conn[connection_name][db_blocks].create_index(make_document(kvp("blocknum", 1)));

        if (mongo_conn[connection_name][db_results].indexes().list().begin() ==
            mongo_conn[connection_name][db_results].indexes().list().end())
            mongo_conn[connection_name][db_results].create_index(make_document(kvp("blocknum", 1)));

        if (mongo_conn[connection_name][db_balances].indexes().list().begin() ==
            mongo_conn[connection_name][db_balances].indexes().list().end())
            mongo_conn[connection_name][db_balances].create_index(make_document(kvp("blocknum", 1)));
    }
}
