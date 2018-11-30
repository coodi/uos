//
// Created by anton on 26.11.18.
//

#include <regex>

#include <eosio/chain/name.hpp>
#include <eosio/chain/asset.hpp>
#include <fc/io/json.hpp>
#include "rate_calculator.hpp"


std::regex NAME_REGEX("^[^6-90][a-z1-5]*\\.?$");

static const char *const node_type_names[] = {"ACCOUNT", "CONTENT", "ORGANIZATION"};

namespace uos {

    void uos_calculator::set_weights_grv_idx(const double_type &activity_weight, const double_type &social_weight,
                                             const money_t &current_supply) {
//        gi_calculator = singularity::gravity_index_calculator(activity_weight,social_weight,current_supply);
    }

    void uos_calculator::set_params_activity(singularity::parameters_t params) {
        social_calculator = singularity::rank_calculator_factory::create_calculator_for_social_network(params);
        transfer_calculator = singularity::rank_calculator_factory::create_calculator_for_transfer(params);
        parameters = params;
    }

    singularity::double_type
    uos_calculator::calculate_index(singularity::money_t &balance, singularity::double_type &activity_index,
                                    singularity::double_type &social_index) {
//        return gi_calculator.calculate_index(balance,activity_index,social_index);
    }

    singularity::money_t
    uos_calculator::calculate_votes(singularity::money_t &balance, singularity::double_type &activity_index,
                                    singularity::double_type &social_index) {
//        return gi_calculator.calculate_votes(balance,activity_index,social_index);
    }

    singularity::account_activity_index_map_t
    uos_calculator::scale_activity_index(const singularity::account_activity_index_map_t &index_map) {
//        return gi_calculator.scale_activity_index(index_map);
    }

    void uos_calculator::parse_block_activity(fc::variant block) {
//        std::cout<<"start parse block - ";
        try {

            std::vector<std::shared_ptr<singularity::relation_t>> social_interactions;
            std::vector<std::shared_ptr<singularity::relation_t>> transfer_interactions;
            auto blocknum = block["blocknum"].as<uint32_t>();
            auto block_height = current_block - blocknum;
            if (start_block < blocknum > end_block)
                return;
            auto trxs = block["transactions"].get_array();
            for (auto &trx : trxs) {
                auto actions = trx["actions"].get_array();
                for (auto &action : actions) {
                    auto contr = action["account"].as_string();
                    auto act = action["action"].as_string();
                    if (contr == "uos.activity") {
                        auto data = action["data"];

///                 switch(action){
///                 case make_content_user

                        if (act == "makecontent") {
                            auto from = data["acc"].as_string();
                            auto to = data["content_id"].as_string();
//                            if (!(std::regex_match(to.c_str(), NAME_REGEX) &&
//                                  (std::regex_match(to.c_str(), NAME_REGEX)))) {
//                                std::cout<<"regex";
//                                continue;
//                            }
                            auto content_type_id = data["content_type_id"].as_string();
                            if (content_type_id == "4")
                                continue;
                            ownership_t ownership(from, to, block_height);
                            social_interactions.push_back(std::make_shared<ownership_t>(ownership));
//                            std::cout<<"1";
                        }

///                 case user_to_content
                        else if (act == "usertocont") {
                            auto from = data["acc"].as_string();
                            auto to = data["content_id"].as_string();
//                            if (!(std::regex_match(to.c_str(), NAME_REGEX) &&
//                                  (std::regex_match(to.c_str(), NAME_REGEX)))) {
//                                std::cout<<"regex";
//
//                                continue;
//                            }
                            auto content_type_id = data["interaction_type_id"].as_string();
                            if (content_type_id == "2") {
                                upvote_t upvote(from, to, block_height);
                                social_interactions.push_back(std::make_shared<upvote_t>(upvote));
                            } else if (content_type_id == "4") {
                                downvote_t downvote(from, to, block_height);
                                social_interactions.push_back(std::make_shared<downvote_t>(downvote));
                            }
//                            std::cout<<"2";
                        }

///                 case make_content_org
                        else if (act == "makecontorg") {
                            auto from = data["organization_id"].as_string();
                            auto to = data["content_id"].as_string();
//                            if (!(std::regex_match(to.c_str(), NAME_REGEX) &&
//                                  (std::regex_match(to.c_str(), NAME_REGEX)))) {
//                                std::cout<<"regex";
//                                continue;
//                            }
                            ownership_t ownership(from, to, block_height);
                            social_interactions.push_back(std::make_shared<ownership_t>(ownership));
//                            std::cout<<"3";
                        } else {
//                            std::cout<<"5";
                        }
                    }
///                 if transfer activity
                    else if (contr == "eosio.token") {
                        if (act == "transfer") {
                            auto data = action["data"];
                            auto from = data["from"].as_string();
                            auto to = data["to"].as_string();
                            auto memo = data["memo"].as_string();
                            auto quantity = eosio::chain::asset::from_string(data["quantity"].as_string());
                            transaction_t transfer(quantity.get_amount(), from, to, time_t(0), block_height);
                            transfer_interactions.push_back(std::make_shared<transaction_t>(transfer));
//                            std::cout<<"6";
                        } else {
//                            std::cout<<"7";
                        }
                    } else {
//                        std::cout<<"8"<<action["account"].as_string();
                    }
                }
            }
            if (!transfer_interactions.empty()) {
                if (transfer_calculator == nullptr) {
                    transfer_calculator = singularity::rank_calculator_factory::create_calculator_for_transfer(
                            parameters);
                }
                transfer_calculator->add_block(transfer_interactions);
                std::cout << "Add transfer ";
            }
            if (!social_interactions.empty()) {
                if (social_calculator == nullptr) {
                    social_calculator = singularity::rank_calculator_factory::create_calculator_for_social_network(
                            parameters);
                }
                social_calculator->add_block(social_interactions);
                std::cout << "Add social ";
            }
//            std::cout<<" end parse block | ";
        }
        catch (...) {
            std::cout << "Block parsing error" << std::endl;
            std::cout << fc::json::to_string(block) << std::endl;
        }

    }

    void uos_calculator::calculate_social2() {
        auto res = social_calculator->calculate();
        for (auto item : res) {
            auto scaled_map = gi_calculator.scale_activity_index(*item.second);
            for (auto gr_item : *item.second) {
                result.res_map[gr_item.first].name = gr_item.first;
                result.res_map[gr_item.first].type = node_type_names[item.first];
                result.res_map[gr_item.first].soc_rate_double_t = gr_item.second;
                result.res_map[gr_item.first].soc_rate = to_string(gr_item.second);
            }
            for (auto sc_item : scaled_map) {
                result.res_map[sc_item.first].name = sc_item.first;
                result.res_map[sc_item.first].soc_rate_scaled_double_t = sc_item.second;
                result.res_map[sc_item.first].soc_rate_scaled = to_string(sc_item.second);
            }
        }
    }

    void uos_calculator::calculate_transfer2() {
        auto res = social_calculator->calculate();
        for (auto item : res) {
            auto scaled_map = gi_calculator.scale_activity_index(*item.second);
            for (auto gr_item : *item.second) {
                result.res_map[gr_item.first].name = gr_item.first;
                result.res_map[gr_item.first].type = node_type_names[item.first];
                result.res_map[gr_item.first].trans_rate_double_t = gr_item.second;
                result.res_map[gr_item.first].trans_rate = to_string(gr_item.second);
            }
            for (auto sc_item : scaled_map) {
                result.res_map[sc_item.first].name = sc_item.first;
                result.res_map[sc_item.first].trans_rate_scaled_double_t = sc_item.second;
                result.res_map[sc_item.first].trans_rate_scaled = to_string(sc_item.second);
            }
        }
    }

    void uos_calculator::calculate_importance2() {
        for (auto &item : result.res_map) {
            if (item.second.type != node_type_names[singularity::ACCOUNT])
                continue;
            item.second.importance_double_t =
                    item.second.trans_rate_double_t * transfer_importance_share +
                    item.second.soc_rate_double_t * social_importance_share;

            item.second.importance_scaled_double_t =
                    item.second.trans_rate_scaled_double_t * transfer_importance_share +
                    item.second.soc_rate_scaled_double_t * social_importance_share;

            item.second.importance = to_string(item.second.importance_double_t);
            item.second.importance_scaled = to_string(item.second.importance_scaled_double_t);
        }
    }

    void uos_calculator::calculate_emission2() {
        //todo: load previous emission here
        double total_emission = initial_token_supply
                                * annual_emission_percent / 100
                                / seconds_per_year
                                * period / blocks_per_second;
        for (auto &item : result.res_map) {
            if (item.second.type != node_type_names[singularity::ACCOUNT])
                continue;
            item.second.current_emission_uos = total_emission * item.second.importance_double_t.convert_to<double>();
            item.second.current_cumulative_emission_uos =
                    item.second.prev_cumulative_emission_uos + item.second.current_emission_uos;

            item.second.current_emission = to_string(item.second.current_emission_uos);
            item.second.current_cumulative_emission = to_string(item.second.current_cumulative_emission_uos);
        }
    }

    void uos_calculator::calculate_mtree2() {
        std::vector<std::pair<string, string> > mt_input;
        for (auto item : result.res_map) {
            if (item.second.current_cumulative_emission_uos == 0)
                continue;
            string temp = "emission " + item.second.name + " " + item.second.current_cumulative_emission;
            mt_input.emplace_back( std::pair<string,string>{ temp, temp } );

            /// should be:
            /// mt_input.emplace_back( { item.first , to_string(item.second.current_cumulative_emission_uos) } );
            ///

        }
        mtree.set_accounts(mt_input);
        mtree.count_tree();
        result.result_hash = mtree.nodes_list[mtree.nodes_list.size() - 1][0];

    }

    void uos_calculator::calculate() {
        clear_result();
        std::cout<<"Start social"<<endl;
        calculate_social2();
        std::cout<<"Start transfer"<<endl;
        calculate_transfer2();
        std::cout<<"Start importance"<<endl;
        calculate_importance2();
        std::cout<<"Start emission"<<endl;
        calculate_emission2();
        std::cout<<"Start mtree"<<endl;
        calculate_mtree2();
        std::cout<<"Finish mtree"<<endl;
    }

    fc::variant uos_calculator::to_variant(){
        fc::mutable_variant_object temp;
        temp["current_block"] = current_block;
        temp["start_block"]   = start_block;
        temp["end_block"]     = end_block;
        temp["result"] = result.to_variant();
        return temp;
    }
}
