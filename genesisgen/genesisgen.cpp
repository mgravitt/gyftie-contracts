#include "genesisgen.hpp"


ACTION genesisgen::generate (name token_contract,
                            string symbol_string,
                            uint8_t    symbol_precision,
                            name from,
                            name to) 
{
    tokengen_singleton t_s (get_self(), get_self().value);
    t_s.set ( tokengen { asset { 10000, symbol { symbol_code(symbol_string.c_str()), symbol_precision }} }, get_self());
}

EOSIO_DISPATCH(genesisgen, (generate))