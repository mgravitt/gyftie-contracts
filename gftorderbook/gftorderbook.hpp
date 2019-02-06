
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

   ACTION removeorders () ;

   ACTION limitbuygft (name buyer, asset price_per_gft, asset gft_amount);

   ACTION limitsellgft (name seller, asset price_per_gft, asset gft_amount);

   ACTION marketbuy (name buyer, asset eos_amount);

   ACTION marketsell (name seller, asset gft_amount);

   ACTION processbook ();

   ACTION withdraw (name account);

   ACTION delbuyorder (uint64_t buyorder_id);

   ACTION delsellorder (uint64_t sellorder_id);

   ACTION admindelbo (uint64_t buyorder_id);

   ACTION admindelso (uint64_t sellorder_id);

   ACTION transrec (name from, name to, asset quantity, string memo);

   ACTION pause ();
   ACTION unpause ();

  private:

    const uint64_t SCALER = 1000000000;
    const string    GYFTIE_SYM_STR  = "GFT";
    const uint8_t   GYFTIE_PRECISION = 8;
    const uint8_t   PAUSED = 1;
    const uint8_t   UNPAUSED = 0;

   TABLE Config
   {
       name         gyftiecontract;
       name         valid_counter_token_contract;
       symbol       valid_counter_token_symbol;
       uint8_t     paused;
   };

   typedef singleton<"configs"_n, Config> config_table;
   typedef eosio::multi_index<"configs"_n, Config> config_table_placeholder;

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
            permission_level{from, "active"_n},
            token_contract, "transfer"_n,
            std::make_tuple(from, to, token_amount, memo))
            .send();

        print("---------- End Payment -------\n");
    }

    asset getopenbalance (name account, symbol sym)
    {
        buyorder_table b_t (get_self(), get_self().value);
        auto b_index = b_t.get_index<"bybuyer"_n>();
        auto b_itr = b_index.find (account.value);

        asset total_orders = asset {0, sym};

        while (b_itr != b_index.end() && b_itr->buyer == account) {
            if (b_itr->order_value.symbol == sym) {
                total_orders += b_itr->order_value;
            }
            b_itr++;
        }

        sellorder_table s_t (get_self(), get_self().value);
        auto s_index = s_t.get_index<"byseller"_n>();
        auto s_itr = s_index.find (account.value);

        while (s_itr != s_index.end() && s_itr->seller == account) {
            if (s_itr->gft_amount.symbol == sym) {
                total_orders += s_itr->gft_amount;
            }
            s_itr++;
        }

        return total_orders;
    }

    void confirm_balance (name account, asset min_balance) 
    {
        balance_table b_t (get_self(), account.value);
        auto b_itr = b_t.find (min_balance.symbol.code().raw());

        asset open_balance =  getopenbalance (account, min_balance.symbol);

        // print ("Confirming balance: ", account, "\n");
        // print ("Required balance: ", min_balance, "\n");
        // print ("Total balance: ", b_itr->funds, "\n");
        // print ("Open Balance: ", open_balance, "\n");

        eosio_assert (b_itr->funds - open_balance >= min_balance, "Insufficient funds.");
    }

    void buygft (uint64_t sellorder_id, name buyer, asset eos_amount) 
    {
        require_auth (buyer);
        
        sellorder_table s_t (get_self(), get_self().value);
        auto s_itr = s_t.find (sellorder_id);
        eosio_assert (s_itr != s_t.end(), "Sell Order ID does not exist.");

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        asset trade_amount = asset { std::min (eos_amount.amount,
                                                s_itr->order_value.amount),
                                      eos_amount.symbol };

        confirm_balance (buyer, eos_amount);
      
        // print ("Executing order, ", std::to_string (sellorder_id).c_str(), "\n");
        // print ("Seller: ", s_itr->seller, "\n");
        // print ("Buyer: ", buyer, "\n");
        // print ("GFT Amount: ", get_gft_amount (s_itr->price_per_gft, trade_amount), "\n");
        // print ("EOS Amount: ", trade_amount, "\n");

        sendfrombal (c.gyftiecontract, s_itr->seller, buyer, get_gft_amount (s_itr->price_per_gft, trade_amount), "Trade");
        sendfrombal (c.valid_counter_token_contract, buyer, s_itr->seller, trade_amount, "Trade");

        if (trade_amount == s_itr->order_value) {
            s_t.erase (s_itr);
        } else if (s_itr->order_value > trade_amount) {
            s_t.modify (s_itr, get_self(), [&](auto &s) {
                s.gft_amount -= get_gft_amount (s_itr->price_per_gft, trade_amount);
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

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        asset trade_amount = asset { std::min (gft_to_sell.amount,
                                                b_itr->gft_amount.amount),
                                      gft_to_sell.symbol };

        confirm_balance (seller, trade_amount);

        sendfrombal (c.gyftiecontract, seller, b_itr->buyer, trade_amount, "Trade");
        sendfrombal (c.valid_counter_token_contract, b_itr->buyer, seller, get_eos_order_value (b_itr->price_per_gft, trade_amount), "Trade");

        if (trade_amount == b_itr->gft_amount) {
            b_t.erase (b_itr);
        } else if (b_itr->gft_amount > trade_amount) {
            b_t.modify (b_itr, get_self(), [&](auto &b) {
                b.gft_amount -= trade_amount;
                b.order_value = get_eos_order_value (b_itr->price_per_gft, b.gft_amount);
            });
        }
    }

    asset get_eos_order_value (asset price_per_gft, asset gft_amount) 
    {
        uint64_t gft_quantity = SCALER * gft_amount.amount / pow(10,gft_amount.symbol.precision());
        return price_per_gft * gft_quantity / SCALER;
    }

    asset get_gft_amount (asset price_per_gft, asset eos_amount)
    {
        // print ("\n\nGET GFT AMOUNT\n");
        // print (" Price per gft: ", price_per_gft, "\n");
        // print (" EOS amount: ", eos_amount, "\n");

        symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

        float gft_quantity = pow(10,GYFTIE_PRECISION) * eos_amount.amount / price_per_gft.amount;

        // uint64_t eos_quantity = SCALER * eos_amount.amount / pow(10,eos_amount.symbol.precision());   
        // print (" EOS quantity : ", std::to_string (eos_quantity).c_str(), "\n");
        
        // uint64_t eos_price_quantity = SCALER * price_per_gft.amount / pow(10,price_per_gft.symbol.precision()); 
        // print (" EOS price quantity : ", std::to_string (eos_price_quantity).c_str(), "\n");

        //uint64_t scaled_gft_amount = static_cast<int64_t>(eos_amount.amount * SCALER / eos_price_quantity.amount);
        // print (" GFT Qty : ", std::to_string (gft_quantity).c_str(), "\n");

        asset gft_amount = asset { static_cast<int64_t>(gft_quantity), gft_symbol };
        // print ("GFT Amount: ", gft_amount, "\n");
        return gft_amount;
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

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        asset trade_amount = asset { std::min (b_itr->gft_amount.amount,
                                                s_itr->gft_amount.amount),
                                      b_itr->gft_amount.symbol };

        // NEED TO DECIDE HOW TO HANDLE DIFFERENCE BETWEEN BUYING AND SELLING PRICE
        asset price = b_itr->price_per_gft;
        //DEPLOY
        if (s_itr->created_date < b_itr->created_date) {
            price = s_itr->price_per_gft;
        }
        
        sendfrombal (c.gyftiecontract, s_itr->seller, b_itr->buyer, trade_amount, "Trade");
        sendfrombal (c.valid_counter_token_contract, b_itr->buyer, s_itr->seller, get_eos_order_value (price, trade_amount), "Trade");

        if (b_itr->gft_amount == s_itr->gft_amount) {
            s_t.erase (s_itr);
            b_t.erase (b_itr);
            // print ("Closing buy order: ", b_itr->order_id, "\n");
            // print ("Closing sell order: ", s_itr->order_id, "\n");
        } else if (b_itr->gft_amount > s_itr->gft_amount) {
            // print ("Decrementing amount on buy order ", b_itr->order_id, " to ", b_itr->gft_amount - s_itr->gft_amount, "\n");
            b_t.modify (b_itr, get_self(), [&](auto &b) {
                b.gft_amount -= s_itr->gft_amount;
                b.order_value = get_eos_order_value (b_itr->price_per_gft, b.gft_amount);
            });
            // print ("Closing sell order: ", s_itr->order_id, "\n");
            s_t.erase (s_itr);
        } else if (s_itr->gft_amount > b_itr->gft_amount) {
            // print ("Decrementing amount on sell order ", s_itr->order_id, " to ", s_itr->gft_amount - b_itr->gft_amount, "\n");
             s_t.modify (s_itr, get_self(), [&](auto &s) {
                s.gft_amount -= b_itr->gft_amount;
                s.order_value = get_eos_order_value (s_itr->price_per_gft, s.gft_amount);
            });
            // print ("Closing buy order: ", b_itr->order_id, "\n");
            b_t.erase (b_itr);
        }
    }
};