#include "genesisgen.hpp"
#include <math.h>

ACTION genesisgen::generate (   name from,
                                asset gyfter_gft_balance,
                                name to) 
{
    asset one_gyftie_token = asset { static_cast<int64_t>(pow(10, gyfter_gft_balance.symbol.precision())), gyfter_gft_balance.symbol};

    //eosio_assert (gyfter_gft_balance < (one_gyftie_token * 10000), "Gyft feature is disabled for balances greater than 1000000 GFT.");

    double gyft_benefit= 0.0;
    asset gyft_benefit_amount = one_gyftie_token;


    if (gyfter_gft_balance  <= (one_gyftie_token * 3)) {
        gyft_benefit_amount = one_gyftie_token * 3;
        // gyft_benefit = 0.30;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 10)) {
        gyft_benefit_amount = one_gyftie_token * 4;
        // gyft_benefit = 0.20;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 20)) {
        gyft_benefit_amount = one_gyftie_token * 5;
        // gyft_benefit = 0.15;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 50)) {
        gyft_benefit_amount = one_gyftie_token * 6;
        // gyft_benefit = 0.10;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 100)) {
        gyft_benefit_amount = one_gyftie_token * 7;
        // gyft_benefit = 0.05;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 200)) {
        gyft_benefit_amount = one_gyftie_token * 8;
        // gyft_benefit = 0.025;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 500)) {
        gyft_benefit_amount = one_gyftie_token * 9;
        // gyft_benefit = 0.02;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 1000)) {
        gyft_benefit_amount = one_gyftie_token * 10;
        // gyft_benefit = 0.01;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 2000)) {
        gyft_benefit_amount = one_gyftie_token * 11;
        // gyft_benefit = 0.005;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 5000)) {
        gyft_benefit_amount = one_gyftie_token * 12;
        // gyft_benefit = 0.0025;
    } else if (gyfter_gft_balance  <= (one_gyftie_token * 10000)) {
        gyft_benefit_amount = one_gyftie_token * 15;
        // gyft_benefit = 0.0025;
    } else {
         gyft_benefit_amount = one_gyftie_token * 20;
    }

    //int64_t    gyft_amount = static_cast<int64_t>(gyfter_gft_balance.amount * gyft_benefit);

    tokengen_table t_t (get_self(), get_self().value);
    tokengen t;
    t.from = from;
    t.to = to;
    t.generated_amount = gyft_benefit_amount; //asset (gyft_amount, gyfter_gft_balance.symbol);
    t_t.set (t, get_self());
}

EOSIO_DISPATCH(genesisgen, (generate))