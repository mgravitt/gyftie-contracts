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

ACTION genesisgen::generate (name token_contract,
                            string symbol_string,
                            uint8_t    symbol_precision,
                            name from,
                            name to) 
{
    tokengen_table t_t (get_self(), get_self().value);
    auto t_itr = t_t.begin();
    if (t_itr != t_t.end()) {
        t_t.erase (t_itr);
    }

    symbol sym = symbol{symbol_code(symbol_string.c_str()), symbol_precision};

    accounts a_t (token_contract, from.value);
    auto a_itr = a_t.find (sym.code().raw());
    eosio_assert (a_itr != a_t.end(), "Gyfter does not have a GYFTIE balance.");

    t_t.emplace (get_self(), [&](auto &t) {
        t.pk                = t_t.available_primary_key();
        t.from              = from;
        t.to                = to;

        asset one_gyftie_token = asset { static_cast<int64_t>(pow(10, symbol_precision)), sym};
        // print ("    One gyftie token:   ", one_gyftie_token, "\n");

        print ("    Gyfter balance:     ", a_itr->balance, "\n");

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        eosio_assert (a_itr->balance <= (one_gyftie_token * 10), "Gyft feature is disabled for balances greater than 10 GFT.");

        //double gyft_benefit= 1.5;

        // if (a_itr->balance  < (one_gyftie_token * 10)) {
        //     gyft_benefit = 1.5;
        // } else if (a_itr->balance  < (one_gyftie_token * 100)) {
        //     gyft_benefit = 1.4;
        // } else if (a_itr->balance  < (one_gyftie_token * 1000)) {
        //     gyft_benefit = 1.3;
        // } else if (a_itr->balance  < (one_gyftie_token * 10000)) {
        //     gyft_benefit = 1.2;
        // } else if (a_itr->balance  < (one_gyftie_token * 100000)) {
        //     gyft_benefit = 1.1;
        // } else if (a_itr->balance  < (one_gyftie_token * 1000000)) {
        //     gyft_benefit = 1.05;
        // } 

        print ("    Gyft benefit    :   ", std::to_string(c.gyfter_payback_rate).c_str(), "\n");
        print ("    Balance amount  :   ", std::to_string(a_itr->balance.amount).c_str(), "\n");

        int64_t    gyft_amount = static_cast<int64_t>(a_itr->balance.amount * c.gyfter_payback_rate);
        print ("    Gyft amount     :   ", std::to_string(gyft_amount).c_str(), "\n");

        t.generated_amount  = asset { gyft_amount, sym } ;
        print ("    Generated amount:   ", t.generated_amount, "\n");

    });
}

EOSIO_DISPATCH(genesisgen, (generate)(setconfig)(reset))