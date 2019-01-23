
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
                        
   ACTION removeorders () ;

   ACTION limitbuygft (name buyer, asset price_per_gft, asset gft_amount);

   ACTION limitsellgft (name seller, asset price_per_gft, asset gft_amount);

   ACTION buygft (uint64_t sellorder_id, name buyer);

   ACTION sellgft (uint64_t buyorder_id, name seller);

   ACTION delbuyorder (uint64_t buyorder_id);

   ACTION delsellorder (uint64_t sellorder_id);

   ACTION transrec (name from, name to, asset quantity, string memo);

  private:

    const uint64_t SCALER = 1000000000;
    const string    GYFTIE_SYM_STR  = "GFT";
    const uint8_t   GYFTIE_PRECISION = 8;

   TABLE Config
   {
       name         gyftiecontract;
       name         valid_counter_token_contract;
       symbol       valid_counter_token_symbol;
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
        uint64_t    primary_key() const { return order_id; }
        uint64_t    by_buyer() const { return buyer.value; }
    };

    typedef eosio::multi_index<"buyorders"_n, buyorder,
        indexed_by<"bybuyer"_n,
            const_mem_fun<buyorder, uint64_t, &buyorder::by_buyer>>
    > buyorder_table;

    TABLE sellorder
    {
        uint64_t    order_id;
        name        seller;
        asset       price_per_gft;
        asset       gft_amount;
        asset       order_value;
        uint64_t    primary_key() const { return order_id; }
        uint64_t    by_seller() const { return seller.value; }
    };

    typedef eosio::multi_index<"sellorders"_n, sellorder,
        indexed_by<"byseller"_n,
            const_mem_fun<sellorder, uint64_t, &sellorder::by_seller>>
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

        bal_table.modify (it, get_self(), [&](auto &b) {
            b.funds -= token_amount;
        });

        paytoken (token_contract, get_self(), to, token_amount, memo);                
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

        print ("Confirming balance: ", account, "\n");
        print ("Required balance: ", min_balance, "\n");
        print ("Total balance: ", b_itr->funds, "\n");
        print ("Open Balance: ", open_balance, "\n");

        eosio_assert (b_itr->funds - open_balance >= min_balance, "Insufficient funds.");
    }

    // asset match_buy_order (name seller, asset gft_amount, uint64_t order_id)
    // {
    //     buyorder_table b_t (get_self(), get_self().value);
    //     auto b_itr = b_t.find (order_id);
    //     eosio_assert (b_itr != b_t.end(), "Order ID is not found.");

    //     print ("Matching order: ", b_itr->order_id, "\n");
    //     print ("b_itr->gft_amount: ", b_itr->gft_amount, "\n");
    //     print ("gft_amount: ", gft_amount, "\n");

    //     asset remaining_order = gft_amount;
    //     if (b_itr->gft_amount <= gft_amount) {
    //         sendfrombal (get_self(), seller, b_itr->buyer, b_itr->gft_amount, "Trade");

    //         uint64_t SCALER = 1000000000;
    //         uint64_t gft_quantity = SCALER * gft_amount.amount / pow(10,gft_amount.symbol.precision());
   
    //         asset funds_settlement = b_itr->price_per_gft * gft_quantity / SCALER;
            
    //         sendfrombal ("eosio.token"_n, b_itr->buyer, seller, funds_settlement, "Trade");
    //         remaining_order -= b_itr->gft_amount;

    //         b_t.erase (b_itr);
    //     }

    //     return remaining_order;
    // }
};