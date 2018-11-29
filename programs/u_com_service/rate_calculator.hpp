//
// Created by anton on 26.11.18.
//

#pragma once

#include <fc/variant.hpp>
#include <eosio/u_com/merkle_tree.hpp>
#include "uos_rates_service.hpp"
#include "../../libraries/singularity/include/singularity.hpp"


namespace uos{

    using singularity::double_type;
    using singularity::money_t;
    using singularity::account_activity_index_map_t;
    using singularity::parameters_t;
    typedef  std::map<singularity::node_type, std::shared_ptr<account_activity_index_map_t>> activity_map_t;

    class uos_calculator{

    public:

        singularity::gravity_index_calculator gi_calculator;
        std::shared_ptr<singularity::activity_index_calculator> social_calculator;
        std::shared_ptr<singularity::activity_index_calculator> transfer_calculator;
        std::shared_ptr<singularity::activity_period> activity_calculator; //todo:
        uint32_t start_block;
        uint32_t end_block;
        uint32_t current_block;

        singularity::parameters_t parameters;
        uos::result_set result;
        double transfer_importance_share;
        double social_importance_share;

        const uint32_t seconds_per_year = 365*24*3600;
        const double annual_emission_percent = 1.0;
        const int64_t initial_token_supply = 1000000000;
        const uint8_t blocks_per_second = 2;
        int32_t period = 300*2;
        merkle_tree<string> mtree;

    public:

        std::function< activity_map_t (activity_map_t&)> postprocessing_social;
        std::function< activity_map_t (activity_map_t&)> postprocessing_transfer;

        uos_calculator():gi_calculator(0.1,0.9,100000000000){
            start_block = 0;
            end_block   = 0;
            current_block = 0;
            transfer_importance_share = 0.1;
            social_importance_share = 0.1;

            postprocessing_social   = [](activity_map_t& map){return map;};
            postprocessing_transfer = [](activity_map_t& map){return map;};
        }
//        uos_calculator(){
//            start_block = 0;
//            end_block   = 0;
//            current_block = 0;
//        }
        void set_bounds(uint32_t begin, uint32_t end, uint32_t current){
            start_block     = begin;
            end_block       = end;
            current_block   = current;
        }
        void                            set_weights_grv_idx(const double_type &activity_weight,const double_type &social_weight,const money_t &current_supply);
        void                            set_params_activity(parameters_t);
        double_type                     calculate_index(money_t &balance, double_type &activity_index, double_type &social_index);
        money_t                         calculate_votes(money_t &balance, double_type &activity_index, double_type &social_index);
        account_activity_index_map_t    scale_activity_index(const account_activity_index_map_t& index_map);



        void parse_block_activity(fc::variant block);
//        void parse_block_grv(fc::variant block);

        void calculate_social2();
        void calculate_transfer2();
        void calculate_importance2();
        void calculate_emission2();
        void calculate_mtree2();

        void calculate();
        void calculate2();
    };
}
