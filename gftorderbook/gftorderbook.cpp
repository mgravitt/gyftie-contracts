#include "gftorderbook.hpp"

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

ACTION gftorderbook::setstate (asset last_price, asset gft_for_sale, asset eos_to_spend) 
{
    require_auth (get_self());
    state_table state (get_self(), get_self().value);
    State s;
    s.last_price = last_price;
    // s.gft_for_sale = gft_for_sale;
    // s.eos_to_spend = eos_to_spend;
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

ACTION gftorderbook::limitbuygft (name buyer, asset price_per_gft, asset gft_amount)
{
    require_auth (buyer);
    confirm_balance (buyer, get_eos_order_value(price_per_gft, gft_amount));

    increase_buygft_liquidity (get_eos_order_value(price_per_gft, gft_amount));

    buyorder_table b_t (get_self(), get_self().value);
    b_t.emplace (get_self(), [&](auto &b) {
        b.order_id = b_t.available_primary_key();
        b.buyer = buyer;
        b.price_per_gft = price_per_gft;
        b.gft_amount = gft_amount;
        b.order_value = get_eos_order_value(price_per_gft, gft_amount);
        b.created_date = now();
    });

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
        limitsellgft(seller, price, get_available_balance (seller, order_gft_amount.symbol));
        processbook_deferred();
        return;
    }

    limitsellgft (seller, price, order_gft_amount);

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
    uint64_t sender_id = now();
    out.send(sender_id, _self);
}

ACTION gftorderbook::limitsellgft (name seller, asset price_per_gft, asset gft_amount)
{
    eosio_assert ( has_auth (seller) || has_auth (get_self()), "Permission denied.");

    confirm_balance (seller, gft_amount);
    increase_sellgft_liquidity (gft_amount);

    sellorder_table s_t (get_self(), get_self().value);
    s_t.emplace (get_self(), [&](auto &s) {
        s.order_id = s_t.available_primary_key();
        s.seller = seller;
        s.price_per_gft = price_per_gft;
        s.gft_amount = gft_amount;
        s.order_value = get_eos_order_value (price_per_gft, gft_amount);
        s.created_date = now();
    });

    processbook ();
}

ACTION gftorderbook::stacksell (name seller, asset gft_amount)
{
    // - sell 1% of amount @ just above current best offer
    // - sell 2% at 1% higher than above offer
    // - sell 3% at 2% higher than above offer
    // - sell 4% at 3% higher than above offer
    // - sell 5% at 4% higher than above offer

    config_table config (get_self(), get_self().value);
    auto c = config.get();
    eosio_assert (  has_auth (seller) || 
                    has_auth (get_self()) ||
                    has_auth (c.gyftiecontract), "Permission denied.");

    confirm_balance (seller, gft_amount);
    
    asset price = get_highest_buy() + asset { 200, c.valid_counter_token_symbol};
    float share = 0.01000000;

    asset order_gft = adjust_asset(gft_amount, share);

    eosio::transaction out{};
    out.actions.emplace_back(permission_level{_self, "owner"_n}, 
                            _self, "stacksellrec"_n, 
                            std::make_tuple(seller,
                                            gft_amount,
                                            gft_amount * 0,
                                            order_gft,
                                            price,
                                            1,
                                            1));
    out.delay_sec = 2;
    uint64_t sender_id = now();
    out.send(sender_id, _self);
}

ACTION gftorderbook::stackbuy (name buyer, asset eos_amount)
{
    require_auth (buyer);
    asset market_price = get_last_price ();

    limitbuygft (buyer, adjust_asset (market_price, 0.95000000), get_gft_amount(adjust_asset (market_price, 0.95000000), adjust_asset(eos_amount, 0.25000000)));
    limitbuygft (buyer, adjust_asset (market_price, 0.90000000), get_gft_amount(adjust_asset (market_price, 0.90000000), adjust_asset(eos_amount, 0.25000000)));
    limitbuygft (buyer, adjust_asset (market_price, 0.80000000), get_gft_amount(adjust_asset (market_price, 0.80000000), adjust_asset(eos_amount, 0.50000000)));
}

ACTION gftorderbook::stack (name account, asset gft_amount, asset eos_amount)
{
    // stacksell (account, gft_amount);
    stackbuy (account, eos_amount);
}

ACTION gftorderbook::marketbuy (name buyer, asset eos_amount) 
{
    require_auth (buyer);

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
    require_auth (seller);

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
    sellorder_table s_t (get_self(), get_self().value);
    auto s_index = s_t.get_index<"byprice"_n>();
    auto s_itr = s_index.begin ();

    buyorder_table b_t (get_self(), get_self().value);
    auto b_index = b_t.get_index<"byprice"_n>();
    auto b_itr = b_index.rbegin();

    if (s_itr == s_index.end() || b_itr == b_index.rend()) {
        return;
    }

    if (s_itr->price_per_gft <= b_itr->price_per_gft) {
        match_order (s_itr->order_id, b_itr->order_id);
        processbook ();
    }
}

ACTION gftorderbook::tradeexec (name buyer, name seller, name market_maker, asset gft_amount, asset price, asset maker_reward)
{
    require_auth (get_self());
}

ACTION gftorderbook::delbuyorder (uint64_t buyorder_id) 
{
    buyorder_table b_t (get_self(), get_self().value);
    auto b_itr = b_t.find (buyorder_id);
    eosio_assert (b_itr != b_t.end(), "Buy Order ID does not exist.");

    require_auth (b_itr->buyer);

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    sendfrombal (c.valid_counter_token_contract, b_itr->buyer, b_itr->buyer, b_itr->order_value, "Cancelled Buy Order");

    b_t.erase (b_itr);
}

ACTION gftorderbook::delsellorder (uint64_t sellorder_id) 
{
    sellorder_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.find (sellorder_id);
    eosio_assert (s_itr != s_t.end(), "Sell Order ID does not exist.");

    eosio_assert (  has_auth (s_itr->seller) || 
                    has_auth (get_self()), "Permission denied.");

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    sendfrombal (c.gyftiecontract, s_itr->seller, s_itr->seller, s_itr->gft_amount, "Cancelled Sell Order");

    s_t.erase (s_itr);
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
    eosio_assert (b_itr != b_t.end(), "Buy Order ID does not exist.");

    require_auth (get_self());

    b_t.erase (b_itr);
}

ACTION gftorderbook::admindelso (uint64_t sellorder_id) 
{
    sellorder_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.find (sellorder_id);
    eosio_assert (s_itr != s_t.end(), "Sell Order ID does not exist.");

    require_auth (get_self());

    s_t.erase (s_itr);
}

ACTION gftorderbook::transrec(name from, name to, asset quantity, string memo) {
    
    eosio_assert (!is_paused(), "Contract is paused - no actions allowed.");
    if (to != get_self()) {
        return; // this contract is not recepient
    }

    print ("Code        : ", get_code(), "\n");
    print ("From        : ", from, "\n");
    print ("To          : ", to, "\n");
    print ("Quantity    : ", quantity, "\n");
    print ("Memo        : ", memo.c_str(), "\n");

    if (memo.compare("FOR STAKING") == 0) {
        return;
    }

    config_table config (get_self(), get_self().value);
    auto c = config.get();
    eosio_assert (get_code() == c.gyftiecontract || get_code() == c.valid_counter_token_contract, "Funds are only accepted from Gyftie contract or valid counter token contract.");

    symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
    eosio_assert (quantity.symbol == gft_symbol || quantity.symbol == c.valid_counter_token_symbol, "Funds are only accepted in GFT symbol or valid counter token symbol.");

    if (get_code() == c.gyftiecontract) {
         eosio_assert (quantity.symbol == gft_symbol, "Invalid symbol from gyftie contract.");
    }

    if (get_code() == c.valid_counter_token_contract) {
        eosio_assert (quantity.symbol == c.valid_counter_token_symbol, "Invalid symbol from counter token contract.");
    }
    
    balance_table balances(_self, from.value);
    asset new_balance;
    auto it = balances.find(quantity.symbol.code().raw());
    if(it != balances.end()) {
        eosio_assert (it->token_contract == get_code(), "Transfer does not match existing token contract.");
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
                EOSIO_DISPATCH_HELPER(gftorderbook, (setconfig)(limitbuygft)(limitsellgft)(marketbuy)(marketsell)(stack)(stackbuy)(stacksell)(delsorders)
                                                    (removeorders)(processbook)(withdraw)(delconfig)(pause)(unpause)(tradeexec)(stacksellrec)
                                                    (delbuyorder)(delsellorder)(admindelso)(admindelbo)(clearstate)(setstate))
            }    
        }
        eosio_exit(0);
    }
}