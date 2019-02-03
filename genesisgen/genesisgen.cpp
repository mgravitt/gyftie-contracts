#include "genesisgen.hpp"
#include <math.h>

ACTION genesisgen::reset () 
{
    require_auth (get_self());
    
    tokengen_table t_t (get_self(), get_self().value);
    auto t_itr = t_t.begin();
    if (t_itr != t_t.end()) {
        t_t.erase (t_itr);
    }

    config_table config (get_self(), get_self().value);
    config.remove();
}

ACTION genesisgen::setconfig (const asset gft_eos_rate,
                                const float gyfter_payback_rate)
{
    require_auth (get_self());
        
    config_table config (get_self(), get_self().value);
    Config c;
    c.gft_eos_rate = gft_eos_rate;
    c.gyfter_payback_rate = gyfter_payback_rate;
    config.set (c, get_self());
}

ACTION genesisgen::generate (   name from,
                                asset gyfter_gft_balance,
                                name to) 
{
    tokengen_table t_t (get_self(), get_self().value);
    auto t_itr = t_t.begin();
    if (t_itr != t_t.end()) {
        t_t.erase (t_itr);
    }

    t_t.emplace (get_self(), [&](auto &t) {
        t.pk                = t_t.available_primary_key();
        t.from              = from;
        t.to                = to;

        asset one_gyftie_token = asset { static_cast<int64_t>(pow(10, gyfter_gft_balance.symbol.precision())), gyfter_gft_balance.symbol};
    
        // eosio_assert (gyfter_gft_balance <= (one_gyftie_token * 40), "Gyft feature is disabled for balances greater than 40 GFT.");

        double gyft_benefit= 0.0;

        if (gyfter_gft_balance  <= (one_gyftie_token * 10)) {
            gyft_benefit = 0.3;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 100)) {
            gyft_benefit = 0.2;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 1000)) {
            gyft_benefit = 0.1;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 10000)) {
            gyft_benefit = 0.03;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 100000)) {
            gyft_benefit = 0.02;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 1000000)) {
            gyft_benefit = 0.01;
        } 

        //print ("    Gyft benefit    :   ", std::to_string(gyft_benefit).c_str(), "\n");
        //print ("    Balance amount  :   ", std::to_string(gyfter_gft_balance.amount).c_str(), "\n");

        int64_t    gyft_amount = static_cast<int64_t>(gyfter_gft_balance.amount * gyft_benefit);
        //print ("    Gyft amount     :   ", std::to_string(gyft_amount).c_str(), "\n");

        t.generated_amount  = asset (gyft_amount, gyfter_gft_balance.symbol);
        //print ("    Generated amount:   ", t.generated_amount, "\n");
    });
}

EOSIO_DISPATCH(genesisgen, (generate)(setconfig)(reset))