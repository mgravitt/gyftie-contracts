#include "secondgen.hpp"


ACTION secondgen::generate (name token_contract,
                            string symbol_string,
                            uint8_t    symbol_precision,
                            name from,
                            name to) 
{
    print ("    Inside generate action\n");

    tokengen_table t_t (get_self(), get_self().value);
    auto t_itr = t_t.begin();
    if (t_itr != t_t.end()) {
        t_t.erase (t_itr);
    }

    symbol sym = symbol{symbol_code(symbol_string.c_str()), symbol_precision};

    accounts a_t (get_self(), from.value);
    auto a_itr = a_t.find (sym.code().raw());
    eosio_assert (a_itr != a_t.end(), "Gyfter does not have a GYFTIE balance.");

    print ("    Writing generated token record.\n");
    t_t.emplace (get_self(), [&](auto &t) {
        t.pk                = t_t.available_primary_key();
        t.from              = from;
        t.to                = to;
        t.generated_amount  = a_itr->balance * 3;
    });
}

EOSIO_DISPATCH(secondgen, (generate))