//
// Created by anton on 30.11.18.
//

#include <fc/io/json.hpp>
#include <fc/variant_object.hpp>
#include "uos_rates_service.hpp"

namespace uos{

    inline std::string rate_to_string(double __val){
        const size_t __n = __gnu_cxx::__numeric_traits<double>::__max_exponent10 + 10;
        return __gnu_cxx::__to_xstring<string>(&std::vsnprintf, __n,"%f", __val);
    }

    std::string to_string(const double_type &_val){
        return rate_to_string(_val.convert_to<double>());
    }

    inline std::string asset_to_string(double __val){
        const size_t __n = __gnu_cxx::__numeric_traits<double>::__max_exponent10 + 4;
        return __gnu_cxx::__to_xstring<string>(&std::vsnprintf, __n,"%f", __val);
    }

    std::string to_string(const uos::asset_type &_val){
        return asset_to_string(_val.convert_to<double>());
    }

    fc::variant result_item::to_variant(){
        fc::mutable_variant_object temp;
        temp["name"]                = name;
        temp["type"]                = type;
        temp["soc_rate"]            = soc_rate;
        temp["soc_rate_d"]          = to_string(soc_rate_double_t);
        temp["soc_rate_scaled"]     = soc_rate_scaled;
        temp["soc_rate_scaled_d"]   = to_string(soc_rate_scaled_double_t);
        temp["trans_rate"]          = trans_rate;
        temp["trans_rate_d"]        = to_string(trans_rate_double_t);
        temp["trans_rate_scaled"]          = trans_rate_scaled;
        temp["trans_rate_scaled_d"]        = to_string(trans_rate_scaled_double_t);
        temp["importance"]          = importance;
        temp["importance_d"]        = to_string(importance_double_t);
        temp["importance_scaled"]   = importance_scaled;
        temp["importance_scaled_d"] = to_string(importance_scaled_double_t);
        temp["prev_cum_emission"]   = prev_cumulative_emission;
        temp["prev_cum_emiussion_d"] = to_string(prev_cumulative_emission_uos);
        temp["current_emission"]     = current_emission;
        temp["current_emission_d"]   = to_string(current_emission_uos);
        temp["current_cum_emission"] = current_cumulative_emission;
        temp["current_cum_emission_d"] = to_string(current_cumulative_emission_uos);
        return temp;
    }

    std::string result_item::to_json(){
        return fc::json::to_string(to_variant());
    }

    fc::variant result_set::to_variant(){
        fc::mutable_variant_object temp;
        temp["block_num"] = block_num;
        temp["result_hash"] = result_hash;
        fc::mutable_variant_object items;
        for (auto item : res_map) {
            items[item.first] = item.second.to_variant();
        }
        temp["items"] = items;
        return temp;
    }

    std::string result_set::to_string(){
        return fc::json::to_string(to_variant());
    }
}
