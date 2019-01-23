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
    config.set (c, get_self());
}


ACTION gftorderbook::limitbuygft (name buyer, asset price_per_gft, asset gft_amount)
{
    require_auth (buyer);

    print ("GFT Amount Asset: ", gft_amount, "\n");
    print ("GFT Amount Asset Amount: ", std::to_string(gft_amount.amount).c_str(), "\n");

    uint64_t gft_quantity = SCALER * gft_amount.amount / pow(10,gft_amount.symbol.precision());
    print ("GFT Quantity: ", std::to_string (gft_quantity).c_str(), "\n");

   // uint64_t scaled_gft_qty = gft_quantity * SCALER;
   // print ("Scaled GFT Quantity: ", std::to_string (scaled_gft_qty).c_str(), "\n");

    asset funds_needed = price_per_gft * gft_quantity / SCALER;
    print ("Funds needed for order: ", funds_needed, "\n");

    confirm_balance (buyer, funds_needed);
    
    buyorder_table b_t (get_self(), get_self().value);
    b_t.emplace (get_self(), [&](auto &b) {
        b.order_id = b_t.available_primary_key();
        b.buyer = buyer;
        b.price_per_gft = price_per_gft;
        b.gft_amount = gft_amount;
        b.order_value = funds_needed;
    });
}

ACTION gftorderbook::limitsellgft (name seller, asset price_per_gft, asset gft_amount)
{
    require_auth (seller);

    uint64_t gft_quantity = SCALER * gft_amount.amount / pow(10,gft_amount.symbol.precision());
    print ("GFT Quantity: ", std::to_string (gft_quantity).c_str(), "\n");

    // uint64_t scaled_gft_qty = gft_quantity * SCALER;
    // print ("Scaled GFT Quantity: ", std::to_string (scaled_gft_qty).c_str(), "\n");

    asset order_value = price_per_gft * gft_quantity / SCALER;
    print ("Order value: ", order_value, "\n");

    confirm_balance (seller, gft_amount);
    
    sellorder_table s_t (get_self(), get_self().value);
    s_t.emplace (get_self(), [&](auto &s) {
        s.order_id = s_t.available_primary_key();
        s.seller = seller;
        s.price_per_gft = price_per_gft;
        s.gft_amount = gft_amount;
        s.order_value = order_value;
    });
}

ACTION gftorderbook::buygft (uint64_t sellorder_id, name buyer) 
{
    require_auth (buyer);

    sellorder_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.find (sellorder_id);
    eosio_assert (s_itr != s_t.end(), "Sell Order ID does not exist.");

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    sendfrombal (c.gyftiecontract, s_itr->seller, buyer, s_itr->gft_amount, "Trade");
    sendfrombal (c.valid_counter_token_contract, buyer, s_itr->seller, s_itr->order_value, "Trade");

    s_t.erase (s_itr);
}

ACTION gftorderbook::sellgft (uint64_t buyorder_id, name seller) 
{
    require_auth (seller);

    buyorder_table b_t (get_self(), get_self().value);
    auto b_itr = b_t.find (buyorder_id);
    eosio_assert (b_itr != b_t.end(), "Buy Order ID does not exist.");

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    sendfrombal (c.valid_counter_token_contract, b_itr->buyer, seller, b_itr->order_value, "Trade");
    sendfrombal (c.gyftiecontract, seller, b_itr->buyer, b_itr->gft_amount, "Trade");

    b_t.erase (b_itr);
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

    require_auth (s_itr->seller);

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    sendfrombal (c.gyftiecontract, s_itr->seller, s_itr->seller, s_itr->gft_amount, "Cancelled Sell Order");

    s_t.erase (s_itr);
}

ACTION gftorderbook::removeorders () 
{
    require_auth (get_self());

    buyorder_table b_t (get_self(), get_self().value);
    auto b_itr = b_t.begin();
    while (b_itr != b_t.end()) {
        b_itr = b_t.erase (b_itr);
    }

    sellorder_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.begin();
    while (s_itr != s_t.end()) {
        s_itr = s_t.erase (s_itr);
    }
}

ACTION gftorderbook::transrec(name from, name to, asset quantity, string memo) {
    
    eosio_assert (!is_paused(), "Contract is paused - no actions allowed.");
    if (from == get_self()) {
        return; // sending funds as sender
    }

    print ("Code        : ", get_code(), "\n");
    print ("From        : ", from, "\n");
    print ("To          : ", to, "\n");
    print ("Quantity    : ", quantity, "\n");
    print ("Memo        : ", memo.c_str(), "\n");

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
                EOSIO_DISPATCH_HELPER(gftorderbook, (setconfig)(limitbuygft)(limitsellgft)(buygft)
                                                    (sellgft)(removeorders)
                                                    (delbuyorder)(delsellorder))
            }    
        }
        eosio_exit(0);
    }
}