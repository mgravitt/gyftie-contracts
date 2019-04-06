#include "gftorderbook.hpp"
#include <algorithm>

ACTION gftorderbook::setconfig (name gyftiecontract, 
                        name valid_counter_token_contract,
                        string valid_counter_symbol_string,
                        uint8_t valid_counter_symbol_precision)
{
    require_auth (get_self());
        
    config_table config (get_self(), get_self().value);
    Config c;
    c.gyftiecontract = gyftiecontract;
    c.valid_counter_token_contract = valid_counter_token_contract;
    c.valid_counter_token_symbol = symbol{symbol_code(valid_counter_symbol_string.c_str()), valid_counter_symbol_precision};
    c.paused = PAUSED;
    config.set (c, get_self());

    senderid_table sid (get_self(), get_self().value);
    SenderID s;
    s.last_sender_id = 1;
    sid.set (s, get_self());
}

ACTION gftorderbook::delconfig () 
{
    require_auth (get_self());
    config_table config (get_self(), get_self().value);
    config.remove();
}

ACTION gftorderbook::clearstate ()
{
    require_auth (get_self());
    state_table state (get_self(), get_self().value);
    state.remove();
}

ACTION gftorderbook::setstate (asset last_price) 
{
    require_auth (get_self());
    state_table state (get_self(), get_self().value);
    State s;
    s.last_price = last_price;
    s.sell_orderbook_size_gft = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
    s.buy_orderbook_size_gft = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
    state.set (s, get_self());
}

ACTION gftorderbook::compilestate () 
{
    asset sell_liquidity = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
    sellorder_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.begin();
    while (s_itr != s_t.end()) {
        sell_liquidity += s_itr->gft_amount;
        s_itr++;
    }

    asset buy_liquidity = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
    buyorder_table b_t (get_self(), get_self().value);
    auto b_itr = b_t.begin();
    while (b_itr != b_t.end()) {
        buy_liquidity += b_itr->gft_amount;
        b_itr++;
    }

    state_table state (get_self(), get_self().value);
    State s = state.get();
    s.sell_orderbook_size_gft = sell_liquidity;
    s.buy_orderbook_size_gft = buy_liquidity;
    state.set (s, get_self());
}

ACTION gftorderbook::pause () 
{
    require_auth (get_self());
    config_table config (get_self(), get_self().value);
    Config c = config.get();
    c.paused = PAUSED;
    config.set (c, get_self());
}

ACTION gftorderbook::unpause () 
{
    require_auth (get_self());
    config_table config (get_self(), get_self().value);
    Config c = config.get();
    c.paused = UNPAUSED;
    config.set (c, get_self());
}

ACTION gftorderbook::withdraw (name account)
{
    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");
    require_auth (account);

    symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
    config_table config (get_self(), get_self().value);
    auto c = config.get();

    balance_table b_t (get_self(), account.value);
    auto b_itr = b_t.find (gft_symbol.code().raw());

    if (b_itr != b_t.end()) {
        asset open_gft_balance = getopenbalance (account, gft_symbol);
        asset withdraw_gft_balance = b_itr->funds - open_gft_balance;
        if (withdraw_gft_balance.amount > 0) {
            sendfrombal (b_itr->token_contract, account, account, withdraw_gft_balance, "Withdrawal from GFT Order Book");
        }
    }

    b_itr = b_t.find (c.valid_counter_token_symbol.code().raw());
    if (b_itr != b_t.end()) {
        asset open_counter_balance = getopenbalance (account, c.valid_counter_token_symbol);
        asset withdraw_counter_balance = b_itr->funds - open_counter_balance;
        if (withdraw_counter_balance.amount > 0) {
            sendfrombal (b_itr->token_contract, account, account, withdraw_counter_balance, "Withdrawal from GFT Order Book");
        }
    }
}

ACTION gftorderbook::setrewconfig (uint64_t proximity_weight_scaled, uint64_t bucket_size_weight_scaled)
{
    require_auth (get_self());
    rewardconfig_table rewardconfig (get_self(), get_self().value);
    Rewardconfig r;
    r.proximity_weight_scaled = proximity_weight_scaled;
    r.bucket_size_weight_scaled = bucket_size_weight_scaled;
    rewardconfig.set (r, get_self());
}

ACTION gftorderbook::payrewbucket (uint64_t bucket_id)
{
    require_auth (get_self());

    orderbucket_table ob_t (get_self(), get_self().value);
    auto ob_itr = ob_t.find (bucket_id);
    eosio::check (ob_itr != ob_t.end(), "Bucket ID is not found.");

    if (ob_itr->reward_due.amount > 0) { 

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        asset remaining_reward = ob_itr->reward_due;
        
        bucketuser_table bu_t (get_self(), bucket_id);
        auto bu_itr = bu_t.begin();
        while (bu_itr != bu_t.end()) {
            float reward_share = (float) bu_itr->bucketuser_size.amount / (float) ob_itr->bucket_size.amount;

            asset reward = asset {  std::min(adjust_asset ( ob_itr->reward_due, reward_share).amount, 
                                                            remaining_reward.amount), 
                                    ob_itr->reward_due.symbol};

            remaining_reward -= reward;
            add_userreward (bu_itr->user, reward);
            //sendfrombal (c.gyftiecontract, c.gyftiecontract, bu_itr->user, reward, "Type 2 financial incentive paid to market makers. See 'How Gyftie Works' document - ask us for link.");
            bu_itr++;
        }

        ob_t.modify (ob_itr, get_self(), [&](auto &ob) {
            ob.reward_due *= 0;
        });
    }

    // if it's the last bucket, payout all rewards to the users
    ob_itr++;
    if (ob_itr == ob_t.end()) {
        eosio::transaction out{};
        out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
                                get_self(), "payrewards"_n, 
                                std::make_tuple());
        out.delay_sec = 1;
        out.send(get_next_sender_id(), get_self());    
    }
}

ACTION gftorderbook::payrewards () 
{
    require_auth (get_self());
    pay_userrewards();
}

ACTION gftorderbook::payrewbucks () 
{
    require_auth (get_self());

    orderbucket_table ob_t (get_self(), get_self().value);
    auto ob_itr = ob_t.begin ();
    
    while (ob_itr != ob_t.end()) {
        eosio::transaction out{};
        out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
                                get_self(), "payrewbucket"_n, 
                                std::make_tuple(ob_itr->bucket_id));
        out.delay_sec = 1;
        out.send(get_next_sender_id(), get_self());    
        ob_itr++;
    }
}

ACTION gftorderbook::addliqreward (asset liqreward) 
{
    config_table config (get_self(), get_self().value);
    auto c = config.get();
    eosio::check (  has_auth (get_self()) ||
                    has_auth (c.gyftiecontract), "Permission denied.");

    liqreward_table l_t (get_self(), get_self().value);
    Liqreward l = l_t.get_or_create(get_self(), Liqreward());
    l.availreward += liqreward;
    l_t.set (l, get_self());

    buildbuckets_deferred();
}

ACTION gftorderbook::payliqinfrew ()
{
    config_table config (get_self(), get_self().value);
    auto c = config.get();
    eosio::check (  has_auth (get_self()) ||
                    has_auth (c.gyftiecontract), "Permission denied.");

    vector<float> bucket_prox_scores;
    float total_prox_score=0;
    orderbucket_table ob_t (get_self(), get_self().value);
    auto ob_itr = ob_t.begin();
    while (ob_itr != ob_t.end()) {
        float prox_score = 0;
        if (ob_itr->bucket_size.amount > 0) {
            prox_score = (float) 100 / ((float)ob_itr->prox_bucket_max_scaled / (float)SCALER);
        }

        bucket_prox_scores.push_back (prox_score);       
        total_prox_score += prox_score;
        ob_itr++;
    }

    state_table state (get_self(), get_self().value);
    State s = state.get();
    asset total_liquidity = s.sell_orderbook_size_gft + s.buy_orderbook_size_gft;

    rewardconfig_table rewardconfig (get_self(), get_self().value);
    Rewardconfig r = rewardconfig.get();

    liqreward_table l_t (get_self(), get_self().value);
    Liqreward l = l_t.get();
    asset liquidity_reward = l.availreward;

    int bucket_counter =0;
    ob_itr = ob_t.begin();
    while (ob_itr != ob_t.end()) {

        if (ob_itr->bucket_size.amount > 0) {
            float proximity_weight = bucket_prox_scores[bucket_counter] / total_prox_score;            
            float bucket_size_weight = (float) ob_itr->bucket_size.amount / (float) total_liquidity.amount;
            float overall_weight = (float) ( (float) proximity_weight * ( (float) r.proximity_weight_scaled / (float) SCALER)) +
                                    ( (float) bucket_size_weight * ( (float) r.bucket_size_weight_scaled / (float) SCALER));
                    
            asset bucket_reward = adjust_asset (liquidity_reward, overall_weight);

            add_bucket_reward (ob_itr->bucket_id, bucket_reward);
        }

        bucket_counter++;
        ob_itr++;
    }

    payrewbucks_deferred();
}

ACTION gftorderbook::defbuckets () 
{
    addbucket (0, 10000);
    addbucket (10001, 20000);
    addbucket (20001, 30000);
    addbucket (30001, 50000);
    addbucket (50001, 70000);
    addbucket (70001, 100000);
    addbucket (100001, 150000);
    addbucket (150001, 200000);
    addbucket (200001, 300000);
    addbucket (300001, 500000);
    addbucket (500001, 700000);
    addbucket (700001, 1000000);
    addbucket (1000001, 2000000);
    addbucket (2000001, 3000000);
    addbucket (3000001, 4000000);
    addbucket (4000001, 5000000);
}

ACTION gftorderbook::addbucket (uint64_t prox_bucket_min_scaled, uint64_t prox_bucket_max_scaled)
{
    require_auth (get_self());
    eosio::check (prox_bucket_min_scaled >= 0, "Proximity bucket minimum must be greater than or equal to zero.");
    eosio::check (prox_bucket_max_scaled > 0, "Proximity bucket maximum must be greater than zero.");
    orderbucket_table ob_t (get_self(), get_self().value);
    auto ob_itr = ob_t.begin();

    while (ob_itr != ob_t.end()) {
        eosio::check (  (prox_bucket_min_scaled < ob_itr->prox_bucket_min_scaled && 
                        prox_bucket_max_scaled < ob_itr->prox_bucket_min_scaled) ||
                        (prox_bucket_max_scaled > ob_itr->prox_bucket_max_scaled && 
                        prox_bucket_min_scaled > ob_itr->prox_bucket_max_scaled), 
            "New bucket overlaps with existing bucket.");

        ob_itr++;
    }

   ob_t.emplace (get_self(), [&](auto &ob) {
       ob.bucket_id = ob_t.available_primary_key();
       ob.prox_bucket_min_scaled = prox_bucket_min_scaled;
       ob.prox_bucket_max_scaled = prox_bucket_max_scaled;
       ob.reward_due = asset { 0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
   });
}

ACTION gftorderbook::buildbuckets () 
{
    orderbucket_table ob_t (get_self(), get_self().value);
    auto ob_itr = ob_t.begin();
    while (ob_itr != ob_t.end()) {

        eosio::transaction out{};
        out.actions.emplace_back(permission_level{_self, "owner"_n}, 
                                _self, "buildbucket"_n, 
                                std::make_tuple(ob_itr->bucket_id));
        out.delay_sec = 1;
        out.send(get_next_sender_id(), _self);

        ob_itr++;
    }
}

ACTION gftorderbook::buildbucket (uint64_t bucket_id)
{
    require_auth (get_self());

    orderbucket_table ob_t (get_self(), get_self().value);
    auto ob_itr = ob_t.find (bucket_id);
    eosio::check (ob_itr != ob_t.end(), "Bucket ID is not found.");

    asset last_price = get_last_price();
    asset bucket_minimum_sell = last_price + adjust_asset (last_price, (float) ob_itr->prox_bucket_min_scaled / SCALER);
    asset bucket_maximum_sell = last_price + adjust_asset (last_price, (float) ob_itr->prox_bucket_max_scaled / SCALER);

    asset bucket_size = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
    clr_bucketuser (bucket_id);

    sellorder_table s_t (get_self(), get_self().value);
    auto s_index = s_t.get_index<"byprice"_n>();
    auto s_itr = s_index.begin ();

    // fast forward cursor to beg of bucket in sell order book
    while (s_itr != s_index.end() && s_itr->price_per_gft < bucket_minimum_sell) {
        s_itr++;
    }

    while (s_itr != s_index.end() && s_itr->price_per_gft < bucket_maximum_sell) {
        bucket_size += s_itr->gft_amount;
        add_bucketuser (bucket_id, s_itr->seller, s_itr->gft_amount);
        s_itr++;
    }

    asset bucket_maximum_buy = asset { std::max(  (last_price - adjust_asset (last_price, (float) ob_itr->prox_bucket_min_scaled / SCALER)).amount, (int64_t) 0), last_price.symbol};
    asset bucket_minimum_buy = asset { std::max(  (last_price - adjust_asset (last_price, (float) ob_itr->prox_bucket_max_scaled / SCALER)).amount, (int64_t) 0), last_price.symbol};

    buyorder_table b_t (get_self(), get_self().value);
    auto b_index = b_t.get_index<"byprice"_n>();
    auto b_itr = b_index.rbegin ();

    if ( bucket_maximum_buy.amount > 0 ) {
        // fast forward cursor to beg of bucket in buy order book
        while (b_itr != b_index.rend() && b_itr->price_per_gft > bucket_maximum_buy) {
            b_itr++;
        }

        while (b_itr != b_index.rend() && b_itr->price_per_gft > bucket_minimum_buy) {
            bucket_size += b_itr->gft_amount;
            add_bucketuser (bucket_id, b_itr->buyer, b_itr->gft_amount);
            b_itr++;
        }
    }

    ob_t.modify (ob_itr, get_self(), [&](auto &ob) {
        ob.bucket_size = bucket_size;
        ob.bucket_minimum_sell = bucket_minimum_sell;
        ob.bucket_maximum_sell = bucket_maximum_sell;
        ob.bucket_minimum_buy = bucket_minimum_buy;
        ob.bucket_maximum_buy = bucket_maximum_buy;
    });

    // kick off reward payment if this is the last bucket
    ob_itr++;
    if (ob_itr == ob_t.end()) {
        payliqinfrew ();
    }
}

ACTION gftorderbook::limitbuygft (name buyer, asset price_per_gft, asset gft_amount)
{
    require_auth (buyer);

    eosio::check ( is_gyftie_account (buyer), "Buyer is not a gyftie account." );
    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");

    add_limitbuy_order (buyer, price_per_gft, gft_amount);

    processbook ();
}


ACTION gftorderbook::stackbuyrec (name buyer, 
                                asset orig_eos_amount, 
                                asset cumulative_stacked_eos,
                                asset order_eos_amount, 
                                asset price, 
                                uint32_t next_price_adj, 
                                uint32_t next_share_adj)
{    
    require_auth (get_self());

    if (cumulative_stacked_eos >= orig_eos_amount) {
        processbook_deferred();
        return;
    }

    if (order_eos_amount > get_available_balance (buyer, order_eos_amount.symbol)) {
        add_limitbuy_order (buyer, price, get_gft_amount(price, get_available_balance (buyer, order_eos_amount.symbol)));
        processbook_deferred();
        return;
    }

    add_limitbuy_order (buyer, price, get_gft_amount(price, order_eos_amount));

    asset next_price = asset {  std::max (adjust_asset (price, 1 - ((float) next_price_adj / (float) 100)).amount, 
                                          (int64_t) 1), price.symbol };
    asset next_order_eos = adjust_asset (orig_eos_amount, (float) next_share_adj / (float) 100);

    eosio::transaction out{};
    out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
                            _self, "stackbuyrec"_n, 
                            std::make_tuple(buyer,
                                            orig_eos_amount,
                                            cumulative_stacked_eos + order_eos_amount,
                                            next_order_eos,
                                            next_price,
                                            next_price_adj + 1,
                                            next_share_adj + 10));
    out.delay_sec = 2;
    out.send(get_next_sender_id(), get_self());
}

ACTION gftorderbook::stackbuy (name buyer, asset eos_amount)
{
    require_auth (buyer);
    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");
    eosio::check ( is_gyftie_account (buyer), "Seller is not a gyftie account." );

    confirm_balance (buyer, eos_amount);    
    asset price = asset {  std::max (get_lowest_sell().amount - 1, (int64_t) 1), eos_amount.symbol };
    float share = 0.01000000;
    asset order_eos =  adjust_asset(eos_amount, share);

    eosio::transaction out{};
    out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
                            get_self(), "stackbuyrec"_n, 
                            std::make_tuple(buyer,
                                            eos_amount,
                                            eos_amount * 0,
                                            order_eos,
                                            price,
                                            1,
                                            1));

    out.delay_sec = 2;
    out.send(get_next_sender_id(), get_self());
}

ACTION gftorderbook::limitsellgft (name seller, asset price_per_gft, asset gft_amount)
{
    config_table config (get_self(), get_self().value);
    auto c = config.get();
    
    eosio::check (  has_auth (seller) || 
                    has_auth (get_self()) ||
                    has_auth (c.gyftiecontract), "Permission denied.");

    eosio::check ( is_gyftie_account (seller), "Seller is not a gyftie account." );
    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");

    add_limitsell_order (seller, price_per_gft, gft_amount);
  
    processbook ();
}

ACTION gftorderbook::stacksellrec (name seller, 
                                asset orig_gft_amount, 
                                asset cumulative_stacked,
                                asset order_gft_amount, asset price, 
                                uint32_t next_price_adj, uint32_t next_share_adj)
{
    require_auth (get_self());

    if (cumulative_stacked >= orig_gft_amount) {
        processbook_deferred();
        return;
    }

    if (order_gft_amount > get_available_balance (seller, order_gft_amount.symbol)) {
        add_limitsell_order (seller, price, get_available_balance (seller, order_gft_amount.symbol));
        processbook_deferred();
        return;
    }

    add_limitsell_order (seller, price, order_gft_amount);

    asset next_price = adjust_asset (price, 1 + ( (float) next_price_adj / (float) 100));
    asset next_order_gft = adjust_asset (orig_gft_amount, (float) next_share_adj / (float) 100);

    eosio::transaction out{};
    out.actions.emplace_back(permission_level{_self, "owner"_n}, 
                            _self, "stacksellrec"_n, 
                            std::make_tuple(seller,
                                            orig_gft_amount,
                                            cumulative_stacked + order_gft_amount,
                                            next_order_gft,
                                            next_price,
                                            next_price_adj + 1,
                                            next_share_adj + 1));
    out.delay_sec = 2;
    out.send(get_next_sender_id(), _self);
}

ACTION gftorderbook::stacksell (name seller, asset gft_amount)
{
    // - sell 1% of amount @ just above current best offer
    // - sell 2% at 1% higher than above offer
    // - sell 3% at 2% higher than above offer
    // - sell 4% at 3% higher than above offer
    // - sell 5% at 4% higher than above offer

    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");

    config_table config (get_self(), get_self().value);
    auto c = config.get();
    eosio::check (  has_auth (seller) || 
                    has_auth (get_self()) ||
                    has_auth (c.gyftiecontract), "Permission denied.");

    eosio::check ( is_gyftie_account (seller), "Seller is not a gyftie account." );

    confirm_balance (seller, gft_amount);
    
    asset price = get_highest_buy() + asset { 200, c.valid_counter_token_symbol};
    float share = 0.01000000;

    asset order_gft = adjust_asset(gft_amount, share);

    eosio::transaction out{};
    out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
                            get_self(), "stacksellrec"_n, 
                            std::make_tuple(seller,
                                            gft_amount,
                                            gft_amount * 0,
                                            order_gft,
                                            price,
                                            1,
                                            1));

    out.delay_sec = 2;
    out.send(get_next_sender_id(), get_self());
}

ACTION gftorderbook::stack (name account, asset gft_amount, asset eos_amount)
{
    stacksell (account, gft_amount);
    stackbuy (account, eos_amount);
}

ACTION gftorderbook::marketbuy (name buyer, asset eos_amount) 
{
    require_auth (buyer);
    eosio::check ( is_gyftie_account (buyer), "Buyer is not a gyftie account." );
    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");

    sellorder_table s_t (get_self(), get_self().value);
    auto s_index = s_t.get_index<"byprice"_n>();
    auto s_itr = s_index.begin ();
    if (s_itr == s_index.end()) {
        return;
    }

    asset remainder_to_spend = eos_amount - s_itr->order_value;
    
    buygft (s_itr->order_id, buyer, eos_amount);

    if (remainder_to_spend.amount > 0) {
        marketbuy (buyer, remainder_to_spend);
    }    
}

ACTION gftorderbook::marketsell (name seller, asset gft_amount) 
{
    config_table config (get_self(), get_self().value);
    auto c = config.get();

    eosio::check (  has_auth (seller) || 
                    has_auth (get_self()) ||
                    has_auth (c.gyftiecontract), "Permission denied.");

    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");

    buyorder_table b_t (get_self(), get_self().value);
    auto b_index = b_t.get_index<"byprice"_n>();
    auto b_itr = b_index.rbegin();

    if (b_itr == b_index.rend()) {
        return;
    }

    asset remainder_to_sell = gft_amount - b_itr->gft_amount;
    
    sellgft (b_itr->order_id, seller, gft_amount);

    if (remainder_to_sell.amount > 0) {
        marketsell (seller, remainder_to_sell);
    }    
}

ACTION gftorderbook::processbook () 
{
    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");
    sellorder_table s_t (get_self(), get_self().value);
    auto s_index = s_t.get_index<"byprice"_n>();
    auto s_itr = s_index.begin ();

    buyorder_table b_t (get_self(), get_self().value);
    auto b_index = b_t.get_index<"byprice"_n>();
    auto b_itr = b_index.rbegin();

    if (s_itr == s_index.end() || b_itr == b_index.rend()) {
        //buildbuckets_deferred();
        return;
    }

    if (s_itr->price_per_gft <= b_itr->price_per_gft) {
        match_order (s_itr->order_id, b_itr->order_id);
        processbook ();
    }

  //  buildbuckets_deferred();
}

ACTION gftorderbook::tradeexec (name buyer, name seller, name market_maker, asset gft_amount, asset price, asset maker_reward)
{
    require_auth (get_self());
}

ACTION gftorderbook::reassign (name current, name newacct, asset quantity) 
{
    require_auth (current);

    balance_table b_t(get_self(), current.value);
    auto b_itr = b_t.find(quantity.symbol.code().raw());
    eosio::check (b_itr != b_t.end(), "Current user does not have a balance.");
    eosio::check (b_itr->funds >= quantity, "Overdrawn balance for reassignment.");

    b_t.modify(b_itr, get_self(), [&](auto& b_current){
        b_current.funds -= quantity;
    });

    balance_table b_t_new (get_self(), newacct.value);
    auto b_itr_new = b_t_new.find(quantity.symbol.code().raw());
   
    if (b_itr_new == b_t_new.end()) {
        b_t_new.emplace (get_self(), [&](auto& b_new){
            b_new.funds = quantity;
            b_new.token_contract  = b_itr->token_contract;
        });
    } else {
        eosio::check (b_itr_new->token_contract == b_itr->token_contract, "Reassigned asset does not match token contract.");
        b_t_new.modify (b_itr_new, get_self(), [&](auto& b_new){
            b_new.funds += quantity;
        });
    }
}

ACTION gftorderbook::delbuyorder (uint64_t buyorder_id) 
{
    buyorder_table b_t (get_self(), get_self().value);
    auto b_itr = b_t.find (buyorder_id);
    eosio::check (b_itr != b_t.end(), "Buy Order ID does not exist.");

    eosio::check (  has_auth (b_itr->buyer) || 
                    has_auth (get_self()), "Permission denied.");

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    sendfrombal (c.valid_counter_token_contract, b_itr->buyer, b_itr->buyer, b_itr->order_value, "Cancelled Buy Order");

    b_t.erase (b_itr);
    //buildbuckets_deferred();
}

ACTION gftorderbook::delsellorder (uint64_t sellorder_id) 
{
    sellorder_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.find (sellorder_id);
    eosio::check (s_itr != s_t.end(), "Sell Order ID does not exist.");

    eosio::check (  has_auth (s_itr->seller) || 
                    has_auth (get_self()), "Permission denied.");

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    sendfrombal (c.gyftiecontract, s_itr->seller, s_itr->seller, s_itr->gft_amount, "Cancelled Sell Order");

    s_t.erase (s_itr);
    //buildbuckets_deferred();
}

ACTION gftorderbook::delsorders (uint64_t low_sellorder_id, uint64_t high_sellorder_id)
{
    for (int64_t order_id = low_sellorder_id; order_id <= high_sellorder_id; order_id++) {
        delsellorder (order_id);
    }
}

ACTION gftorderbook::removeorders () 
{
    require_auth (get_self());

    buyorder_table b_t (get_self(), get_self().value);
    auto b_itr = b_t.begin();
    while (b_itr != b_t.end()) {
        delbuyorder (b_itr->order_id);
        b_itr++;
    }

    sellorder_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.begin();
    while (s_itr != s_t.end()) {
        delsellorder (s_itr->order_id);
        s_itr++;
    }
}

ACTION gftorderbook::admindelbo (uint64_t buyorder_id) 
{
    buyorder_table b_t (get_self(), get_self().value);
    auto b_itr = b_t.find (buyorder_id);
    eosio::check (b_itr != b_t.end(), "Buy Order ID does not exist.");

    require_auth (get_self());

    b_t.erase (b_itr);
}

ACTION gftorderbook::admindelso (uint64_t sellorder_id) 
{
    sellorder_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.find (sellorder_id);
    eosio::check (s_itr != s_t.end(), "Sell Order ID does not exist.");

    require_auth (get_self());

    s_t.erase (s_itr);
}

ACTION gftorderbook::transrec(name from, name to, asset quantity, string memo) {
    
    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");
    if (to != get_self()) {
        return; // this contract is not recepient
    }

    // print ("Code        : ", get_code(), "\n");
    // print ("From        : ", from, "\n");
    // print ("To          : ", to, "\n");
    // print ("Quantity    : ", quantity, "\n");
    // print ("Memo        : ", memo.c_str(), "\n");

    if (memo.compare("FOR STAKING") == 0) {
        return;
    }

    config_table config (get_self(), get_self().value);
    auto c = config.get();
    eosio::check (get_code() == c.gyftiecontract || get_code() == c.valid_counter_token_contract, "Funds are only accepted from Gyftie contract or valid counter token contract.");

    symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
    eosio::check (quantity.symbol == gft_symbol || quantity.symbol == c.valid_counter_token_symbol, "Funds are only accepted in GFT symbol or valid counter token symbol.");

    if (get_code() == c.gyftiecontract) {
         eosio::check (quantity.symbol == gft_symbol, "Invalid symbol from gyftie contract.");
    }

    if (get_code() == c.valid_counter_token_contract) {
        eosio::check (quantity.symbol == c.valid_counter_token_symbol, "Invalid symbol from counter token contract.");
    }
    
    balance_table balances(_self, from.value);
    asset new_balance;
    auto it = balances.find(quantity.symbol.code().raw());
    if(it != balances.end()) {
        eosio::check (it->token_contract == get_code(), "Transfer does not match existing token contract.");
        balances.modify(it, get_self(), [&](auto& bal){
            // Assumption: total currency issued by eosio.token will not overflow asset
            bal.funds += quantity;
            new_balance = bal.funds;
        });
    }
    else
        balances.emplace(get_self(), [&](auto& bal){
            bal.funds = quantity;
            bal.token_contract  = get_code();
            new_balance = quantity;
        });

    print ("\n");
    print(name{from}, " deposited:       ", quantity, "\n");
    print(name{from}, " funds available: ", new_balance);
    print ("\n");
}

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        if (action == "transfer"_n.value && code != receiver) {
            eosio::execute_action(eosio::name(receiver), eosio::name(code), &gftorderbook::transrec);
        }
        if (code == receiver) {
            switch (action) { 
                EOSIO_DISPATCH_HELPER(gftorderbook, (setconfig)(limitbuygft)(limitsellgft)(marketbuy)(marketsell)(stack)(stackbuy)(stacksell)(delsorders)(defbuckets)
                                                    (removeorders)(processbook)(withdraw)(delconfig)(pause)(unpause)(tradeexec)(stacksellrec)(stackbuyrec)(compilestate)
                                                    (delbuyorder)(delsellorder)(admindelso)(admindelbo)(clearstate)(setstate)(reassign)(addliqreward)(payrewards)
                                                    (setrewconfig)(addbucket)(buildbucket)(buildbuckets)(payliqinfrew)(payrewbucket)(payrewbucks))
            }    
        }
        eosio_exit(0);
    }
}