
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
#include <algorithm>    // std::find
#include <eosiolib/singleton.hpp>
#include <math.h>

using std::string;
using std::vector;
using namespace eosio;

CONTRACT gftorderbook : public contract
{
    using contract::contract;

  public:

    ACTION setconfig (name gyftiecontract, 
                        name valid_counter_token_contract,
                        string valid_counter_symbol_string,
                        uint8_t valid_counter_symbol_precision);

    ACTION delconfig ();

    ACTION clearstate ();

    ACTION setstate (asset last_price, asset gft_for_sale, asset eos_to_spend);

   ACTION removeorders () ;

   ACTION limitbuygft (name buyer, asset price_per_gft, asset gft_amount);

   ACTION limitsellgft (name seller, asset price_per_gft, asset gft_amount);

    ACTION stack (name account, asset gft_amount, asset eos_amount);

   ACTION marketbuy (name buyer, asset eos_amount);

   ACTION marketsell (name seller, asset gft_amount);

   ACTION stacksell (name seller, asset gft_amount);

   ACTION stackbuy (name buyer, asset eos_amount);

   ACTION processbook ();

   ACTION tradeexec (name buyer, name seller, name market_maker, asset gft_amount, asset price, asset maker_reward);

   ACTION withdraw (name account);

   ACTION delbuyorder (uint64_t buyorder_id);

   ACTION delsellorder (uint64_t sellorder_id);

   ACTION admindelbo (uint64_t buyorder_id);

   ACTION admindelso (uint64_t sellorder_id);

   ACTION transrec (name from, name to, asset quantity, string memo);

   ACTION pause ();
   ACTION unpause ();

  private:

    const uint64_t SCALER = 1000000;
    const string    GYFTIE_SYM_STR  = "GFT";
    const uint8_t   GYFTIE_PRECISION = 8;
    const uint8_t   PAUSED = 1;
    const uint8_t   UNPAUSED = 0;
    const float     MAKER_REWARD = 0.0100000000;

   TABLE Config
   {
       name         gyftiecontract;
       name         valid_counter_token_contract;
       symbol       valid_counter_token_symbol;
       uint8_t      paused;
   };

   typedef singleton<"configs"_n, Config> config_table;
   typedef eosio::multi_index<"configs"_n, Config> config_table_placeholder;

   TABLE State
   {
       asset        last_price;
    //    asset        gft_for_sale;
    //    asset        eos_to_spend;
   };
   typedef singleton<"states"_n, State> state_table;
   typedef eosio::multi_index<"states"_n, State> state_table_placeholder;

   TABLE buyorder
    {
        uint64_t    order_id;
        name        buyer;
        asset       price_per_gft;
        asset       gft_amount;
        asset       order_value;
        uint32_t    created_date;
        uint64_t    primary_key() const { return order_id; }
        uint64_t    by_buyer() const { return buyer.value; }
        uint64_t    by_price() const { return price_per_gft.amount; }
    };

    typedef eosio::multi_index<"buyorders"_n, buyorder,
        indexed_by<"bybuyer"_n,
            const_mem_fun<buyorder, uint64_t, &buyorder::by_buyer>>,
        indexed_by<"byprice"_n,
            const_mem_fun<buyorder, uint64_t, &buyorder::by_price>>
    > buyorder_table;

    TABLE sellorder
    {
        uint64_t    order_id;
        name        seller;
        asset       price_per_gft;
        asset       gft_amount;
        asset       order_value;
        uint32_t    created_date;
        uint64_t    primary_key() const { return order_id; }
        uint64_t    by_seller() const { return seller.value; }
        uint64_t    by_price() const { return price_per_gft.amount; }
    };

    typedef eosio::multi_index<"sellorders"_n, sellorder,
        indexed_by<"byseller"_n,
            const_mem_fun<sellorder, uint64_t, &sellorder::by_seller>>,
        indexed_by<"byprice"_n,
            const_mem_fun<sellorder, uint64_t, &sellorder::by_price>>
    > sellorder_table;

    TABLE balance 
    {
        asset funds;
        name token_contract;
        uint64_t primary_key() const { return funds.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"balances"_n, balance> balance_table;

    bool is_paused () 
    {
        config_table config (get_self(), get_self().value);
        auto c = config.get();
        if (c.paused == PAUSED) {
            return true;
        }
        return false;
    }

    void sendfrombal ( const name token_contract,
                        const name from,
                        const name to,
                        const asset token_amount,
                        const string memo) 
    {
        balance_table bal_table (get_self(), from.value);
        auto it = bal_table.find(token_amount.symbol.code().raw());
        eosio_assert (it != bal_table.end(), "Sender does not have a balance within the contract." );
        eosio_assert (it->funds >= token_amount, "Insufficient balance.");

        bool remove_record = false;
        bal_table.modify (it, get_self(), [&](auto &b) {
            if (b.funds == token_amount) {
                remove_record = true;
            }
            b.funds -= token_amount;
        });

        require_recipient (from);

        paytoken (token_contract, get_self(), to, token_amount, memo);                

        if (remove_record) {
            bal_table.erase (it);
        }
    }

    void paytoken(  const name  token_contract,
                    const name from,
                    const name to,
                    const asset token_amount,
                    const string memo)
    {
        print("---------- Payment -----------\n");
        print("Token Contract   : ", name{token_contract}, "\n");
        print("From             : ", name{from}, "\n");
        print("To               : ", name{to}, "\n");
        print("Amount           : ", token_amount, "\n");
        print("Memo             : ", memo, "\n");

        action(
            permission_level{from, "owner"_n},
            token_contract, "transfer"_n,
            std::make_tuple(from, to, token_amount, memo))
            .send();

        print("---------- End Payment -------\n");
    }

    asset getopenbalance (name account, symbol sym)
    {
        asset total_orders = asset {0, sym};
    
        symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

        config_table config (get_self(), get_self().value);
        auto c = config.get();
        
        if (sym == gft_symbol) {
            sellorder_table s_t (get_self(), get_self().value);
            auto s_index = s_t.get_index<"byseller"_n>();
            auto s_itr = s_index.find (account.value);

            while (s_itr != s_index.end() && s_itr->seller == account) {
                if (s_itr->gft_amount.symbol == sym) {
                    total_orders += s_itr->gft_amount;
                }
                s_itr++;
            }
        } else if (sym == c.valid_counter_token_symbol) {
            buyorder_table b_t (get_self(), get_self().value);
            auto b_index = b_t.get_index<"bybuyer"_n>();
            auto b_itr = b_index.find (account.value);
            while (b_itr != b_index.end() && b_itr->buyer == account) {
                if (b_itr->order_value.symbol == sym) {
                    total_orders += b_itr->order_value;
                }
                b_itr++;
            }            
        }
        return total_orders;
    }

    void set_last_price (asset last_price)
    {
        state_table state (get_self(), get_self().value);
        State s;
        s.last_price = last_price;
        state.set (s, get_self());        
    }

    asset get_last_price () 
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        return s.last_price;
    }

    asset get_highest_buy () 
    {
        buyorder_table b_t (get_self(), get_self().value);
        auto b_index = b_t.get_index<"byprice"_n>();
        auto b_itr = b_index.rbegin();

        eosio_assert (b_itr != b_index.rend(), "No open buy orders; market price cannot be determined.");
        return b_itr->price_per_gft;
    }

    asset adjust_asset (asset original_asset, float adjustment)
    {
        return asset { static_cast<int64_t> (original_asset.amount * adjustment), original_asset.symbol };
    }

    void decrease_sellgft_liquidity (asset gft_sold)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        // s.gft_for_sale -= gft_sold;
        state.set (s, get_self());
    }

    void decrease_buygft_liquidity (asset eos_spent)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        // s.eos_to_spend -= eos_spent;
        state.set (s, get_self());
    }

    void increase_sellgft_liquidity (asset new_gft_added)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        // s.gft_for_sale -= gft_sold;
        state.set (s, get_self());
    }

    void increase_buygft_liquidity (asset new_eos_to_spend)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        // s.eos_to_spend -= new_eos_to_spend;
        state.set (s, get_self());
    }

    void confirm_balance (name account, asset min_balance) 
    {
        balance_table b_t (get_self(), account.value);
        auto b_itr = b_t.find (min_balance.symbol.code().raw());

        asset open_balance =  getopenbalance (account, min_balance.symbol);
        eosio_assert (b_itr->funds - open_balance >= min_balance, "Insufficient funds.");
    }

    void settle_seller_maker (name buyer, name seller, asset price, asset gft_amount)
    {
        asset xfer_to_buyer_gft = adjust_asset (gft_amount, 1 - MAKER_REWARD);
        asset xfer_to_seller_eos = get_eos_order_value (price, gft_amount);
        asset taker_fee_to_seller_gft = gft_amount - xfer_to_buyer_gft;

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        sendfrombal (c.gyftiecontract, seller, buyer, xfer_to_buyer_gft, "Trade minus Taker Fees");
        sendfrombal (c.valid_counter_token_contract, buyer, seller, xfer_to_seller_eos, "Trade");
        sendfrombal (c.gyftiecontract, seller, seller, taker_fee_to_seller_gft, "Market Maker Reward");
        set_last_price (price);
        decrease_sellgft_liquidity (gft_amount);

        action(
            permission_level{get_self(), "owner"_n},
            get_self(), "tradeexec"_n,
            std::make_tuple(buyer, seller, seller, gft_amount, price, taker_fee_to_seller_gft))
        .send();
    }

    void settle_buyer_maker (name buyer, name seller, asset price, asset gft_amount)
    {
        asset eos_order_value = get_eos_order_value (price, gft_amount);
        asset xfer_to_seller_eos = adjust_asset (eos_order_value, 1 - MAKER_REWARD);
        asset taker_fee_to_buyer_eos = eos_order_value - xfer_to_seller_eos;

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        sendfrombal (c.gyftiecontract, seller, buyer, gft_amount, "Trade");
        sendfrombal (c.valid_counter_token_contract, buyer, seller, xfer_to_seller_eos, "Trade minus Taker Fees");
        sendfrombal (c.valid_counter_token_contract, buyer, buyer, taker_fee_to_buyer_eos, "Market Maker Reward");
        set_last_price (price);
        decrease_buygft_liquidity (eos_order_value);

        action(
            permission_level{get_self(), "owner"_n},
            get_self(), "tradeexec"_n,
            std::make_tuple(buyer, seller, buyer, gft_amount, price, taker_fee_to_buyer_eos))
        .send();
    }

    void buygft (uint64_t sellorder_id, name buyer, asset eos_to_spend) 
    {
        require_auth (buyer);
        
        sellorder_table s_t (get_self(), get_self().value);
        auto s_itr = s_t.find (sellorder_id);
        eosio_assert (s_itr != s_t.end(), "Sell Order ID does not exist.");

        asset eos_amount = asset { std::min (eos_to_spend.amount, s_itr->order_value.amount),
                                   eos_to_spend.symbol };

        confirm_balance (buyer, eos_amount);

        settle_seller_maker (buyer, s_itr->seller, s_itr->price_per_gft, 
                             get_gft_amount (s_itr->price_per_gft, eos_amount));

        if (eos_amount == s_itr->order_value) {
            s_t.erase (s_itr);
        } else if (s_itr->order_value > eos_amount) {
            s_t.modify (s_itr, get_self(), [&](auto &s) {
                s.gft_amount -= get_gft_amount (s_itr->price_per_gft, eos_amount);
                s.order_value = get_eos_order_value (s_itr->price_per_gft, s.gft_amount);
            });
        }
    }

    void sellgft (uint64_t buyorder_id, name seller, asset gft_to_sell) 
    {
        require_auth (seller);

        buyorder_table b_t (get_self(), get_self().value);
        auto b_itr = b_t.find (buyorder_id);
        eosio_assert (b_itr != b_t.end(), "Buy Order ID does not exist.");

        asset gft_amount = asset { std::min (gft_to_sell.amount, b_itr->gft_amount.amount),
                                   gft_to_sell.symbol };

        confirm_balance (seller, gft_amount);

        settle_buyer_maker (b_itr->buyer, seller, b_itr->price_per_gft, gft_amount);

        if (gft_amount == b_itr->gft_amount) {
            b_t.erase (b_itr);
        } else if (b_itr->gft_amount > gft_amount) {
            b_t.modify (b_itr, get_self(), [&](auto &b) {
                b.gft_amount -= gft_amount;
                b.order_value = get_eos_order_value (b_itr->price_per_gft, b.gft_amount);
            });
        }
    }

    asset get_eos_order_value (asset price_per_gft, asset gft_amount) 
    {
        // uint64_t gft_quantity = gft_amount.amount; 
        return price_per_gft * gft_amount.amount / pow(10,GYFTIE_PRECISION);
    }

    asset get_gft_amount (asset price_per_gft, asset eos_amount)
    {
        symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        float gft_quantity = pow(10,GYFTIE_PRECISION) * eos_amount.amount / price_per_gft.amount;
        return asset { static_cast<int64_t>(gft_quantity), gft_symbol };
        //return gft_amount;
    }

    void match_order (uint64_t sellorder_id, uint64_t buyorder_id)
    {
        // handle trade between two orders, decrementally the lower of the two appropriately
        sellorder_table s_t (get_self(), get_self().value);
        auto s_itr = s_t.find (sellorder_id);
        eosio_assert (s_itr != s_t.end(), "Sell Order ID does not exist.");

        buyorder_table b_t (get_self(), get_self().value);
        auto b_itr = b_t.find (buyorder_id);
        eosio_assert (b_itr != b_t.end(), "Buy Order ID does not exist.");

        eosio_assert (b_itr->price_per_gft >= s_itr->price_per_gft, "Buyer's price is less than the seller's price.");

        asset gft_amount = asset { std::min (b_itr->gft_amount.amount, s_itr->gft_amount.amount),
                                   b_itr->gft_amount.symbol };
           
        // if seller is market maker
        if (s_itr->created_date < b_itr->created_date) {
            settle_seller_maker (b_itr->buyer, s_itr->seller, s_itr->price_per_gft, gft_amount);
        } else { // buyer is market maker
            settle_buyer_maker(b_itr->buyer, s_itr->seller, b_itr->price_per_gft, gft_amount);
        }

        if (b_itr->gft_amount == s_itr->gft_amount) {
            s_t.erase (s_itr);
            b_t.erase (b_itr);
        } else if (b_itr->gft_amount > s_itr->gft_amount) {
            b_t.modify (b_itr, get_self(), [&](auto &b) {
                b.gft_amount -= s_itr->gft_amount;
                b.order_value = get_eos_order_value (b_itr->price_per_gft, b.gft_amount);
            });
            s_t.erase (s_itr);
        } else if (s_itr->gft_amount > b_itr->gft_amount) {
             s_t.modify (s_itr, get_self(), [&](auto &s) {
                s.gft_amount -= b_itr->gft_amount;
                s.order_value = get_eos_order_value (s_itr->price_per_gft, s.gft_amount);
            });
            b_t.erase (b_itr);
        }
    }
};