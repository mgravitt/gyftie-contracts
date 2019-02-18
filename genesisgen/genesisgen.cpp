#include "genesisgen.hpp"
#include <math.h>

ACTION genesisgen::test () 
{

}

ACTION genesisgen::generate (   name from,
                                asset gyfter_gft_balance,
                                name to) 
{
    asset one_gyftie_token = asset { static_cast<int64_t>(pow(10, gyfter_gft_balance.symbol.precision())), gyfter_gft_balance.symbol};

    // eosio_assert (gyfter_gft_balance <= (one_gyftie_token * 40), "Gyft feature is disabled for balances greater than 40 GFT.");

    double gyft_benefit= 0.0;

    if (gyfter_gft_balance  <= (one_gyftie_token * 10)) {
        gyft_benefit = 0.15;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 100)) {
        gyft_benefit = 0.10;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 1000)) {
        gyft_benefit = 0.05;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 10000)) {
        gyft_benefit = 0.03;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 100000)) {
        gyft_benefit = 0.02;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 1000000)) {
        gyft_benefit = 0.01;
    } 

    int64_t    gyft_amount = static_cast<int64_t>(gyfter_gft_balance.amount * gyft_benefit);

    tokengen_table t_t (get_self(), get_self().value);
    tokengen t;
    t.from = from;
    t.to = to;
    t.generated_amount = asset (gyft_amount, gyfter_gft_balance.symbol);
    t_t.set (t, get_self());
}

EOSIO_DISPATCH(genesisgen, (generate)(test))