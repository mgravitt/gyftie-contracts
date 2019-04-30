
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <string>
#include <algorithm>    // std::find
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp> // include this for transactions

#include <math.h>

using std::string;
using std::vector;
using namespace eosio;

CONTRACT gftorderbook : public contract
{
    using contract::contract;

  public:

   ACTION upperm ();

    ACTION setconfig (name gyftiecontract, 
                        name valid_counter_token_contract,
                        string valid_counter_symbol_string,
                        uint8_t valid_counter_symbol_precision);

    ACTION delconfig ();

   // ACTION primereward ();

    ACTION setrewconfig (uint64_t proximity_weight, uint64_t bucket_size_weight);

    ACTION addbucket (uint64_t prox_bucket_min, uint64_t prox_bucket_max);

    ACTION buildbucket (uint64_t bucket_id);

    ACTION buildbuckets ();

    ACTION payrewbucket (uint64_t bucket_id);
    
    ACTION payrewbucks ();

    ACTION addliqreward (asset liqreward);

    ACTION payliqinfrew ();

    ACTION payrewards ();

    ACTION stacksellrec (name seller, asset orig_gft_amount, 
                    asset cumulative_stacked, 
                    asset order_gft_amount, asset price, 
                    uint32_t next_price_adj, uint32_t next_share_adj);

    ACTION stackbuyrec (name buyer, 
                        asset orig_eos_amount, 
                        asset cumulative_stacked_eos,
                        asset order_eos_amount, 
                        asset price, 
                        uint32_t next_price_adj, 
                        uint32_t next_share_adj);
    
    ACTION clearstate ();

    ACTION setstate (asset last_price);

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
   
   ACTION delbordersv (vector<uint64_t> buyorder_ids);

   ACTION delsellorder (uint64_t sellorder_id);

   ACTION delsorders (uint64_t low_sellorder_id, uint64_t high_sellorder_id);

   ACTION delsordersv (vector<uint64_t> sellorder_ids);

   ACTION admindelbo (uint64_t buyorder_id);

   ACTION admindelso (uint64_t sellorder_id);

   ACTION transrec (name from, name to, asset quantity, string memo);

   ACTION pause ();
   ACTION unpause ();

   ACTION compilestate ();

   ACTION defbuckets ();

   ACTION reassign (name current, name newacct, asset quantity);

  private:

    const uint64_t  SCALER = 1000000;
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
       asset        sell_orderbook_size_gft;
       asset        buy_orderbook_size_gft;
       uint32_t     last_bucket_build_time;
       uint32_t     last_payrewbucks_time;
   };
   typedef singleton<"states"_n, State> state_table;
   typedef eosio::multi_index<"states"_n, State> state_table_placeholder;

   TABLE Rewardconfig
   {
    //   uint64_t        inflation_share_scaled;
       uint64_t        proximity_weight_scaled;
       uint64_t        bucket_size_weight_scaled;
       asset           reward_remainder;
   };
    typedef singleton<"rewconfigs"_n, Rewardconfig> rewardconfig_table;
    typedef eosio::multi_index<"rewconfigs"_n, Rewardconfig> rewardconfig_table_placeholder;

    TABLE SenderID
    {
        uint64_t    last_sender_id;
    };
    typedef singleton<"senderids"_n, SenderID> senderid_table;
    typedef eosio::multi_index<"senderids"_n, SenderID> senderid_table_placeholder;
   
   TABLE Liqreward
   {
       asset        availreward = asset {0, symbol{symbol_code(string{"GFT"}.c_str()), 8}};
   };
   typedef singleton<"liqrewards"_n, Liqreward> liqreward_table;
   typedef eosio::multi_index<"liqrewards"_n, Liqreward> liqreward_table_placeholder;

    TABLE rewarddue 
    {
        name        recipient;
        asset       amount_due;
        uint64_t    primary_key() const { return recipient.value; }
    };

    typedef eosio::multi_index<"rewarddues"_n, rewarddue> rewarddue_table;

   TABLE orderbucket
   {    
       uint64_t     bucket_id;
       uint64_t     prox_bucket_min_scaled;
       uint64_t     prox_bucket_max_scaled;
       asset        bucket_minimum_sell;
       asset        bucket_maximum_sell;
       asset        bucket_maximum_buy;
       asset        bucket_minimum_buy;
       asset        bucket_size;
       asset        reward_due;
       uint64_t     primary_key() const { return bucket_id; }
   };

   typedef eosio::multi_index<"orderbuckets"_n, orderbucket> orderbucket_table;

   TABLE bucketuser
   {
       name         user;
       asset        bucketuser_size;
       uint64_t     primary_key() const { return user.value; }
   };

   typedef eosio::multi_index<"bucketusers"_n, bucketuser> bucketuser_table;

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

    TABLE account
    {
        asset balance;
        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };
    typedef eosio::multi_index<"accounts"_n, account> accounts;

    TABLE profile
    {
        name        account;
        uint32_t    rating_sum;
        uint16_t    rating_count;
        string      idhash;
        string      id_expiration;
        asset       gft_balance;
        asset       staked_balance;
        uint64_t    primary_key() const { return account.value; }
    };
    typedef eosio::multi_index<"profiles"_n, profile> profile_table;

    struct key_weight
    {
        eosio::public_key key;
        uint16_t weight;
    };

    struct permission_level_weight
    {
        permission_level permission;
        uint16_t weight;
    };

    struct wait_weight
    {
        uint32_t wait_sec;
        uint16_t weight;
    };

    struct authority
    {
        uint32_t threshold;
        std::vector<key_weight> keys;
        std::vector<permission_level_weight> accounts;
        std::vector<wait_weight> waits;
    };

    void add_userreward (name user, asset reward) 
    {
        rewarddue_table rd_t (get_self(), get_self().value);
        auto rd_itr = rd_t.find (user.value);

        if (rd_itr == rd_t.end()) {
            rd_t.emplace (get_self(), [&](auto &rd) {
                rd.recipient = user;
                rd.amount_due = reward;
            });
        } else {
            rd_t.modify (rd_itr, get_self(), [&](auto &rd) {
                rd.amount_due += reward;
            });
        }
    }

    void pay_userrewards () 
    {
        config_table config (get_self(), get_self().value);
        auto c = config.get();

        rewarddue_table rd_t (get_self(), get_self().value);
        auto rd_itr = rd_t.begin();

        while (rd_itr != rd_t.end()) {
            if (rd_itr->amount_due.amount > 0) {
                sendfrombal (c.gyftiecontract, c.gyftiecontract, 
                            rd_itr->recipient, rd_itr->amount_due, 
                            "Type 2 financial incentive paid to market makers. See 'How Gyftie Works' document - ask us for link.");
                rd_itr = rd_t.erase (rd_itr);
            } 
        }
    }

    uint64_t get_next_sender_id()
    {
        senderid_table sid (get_self(), get_self().value);
        SenderID s = sid.get();
        uint64_t return_senderid = s.last_sender_id;
        return_senderid++;
        s.last_sender_id = return_senderid;
        sid.set (s, get_self());
        return return_senderid;
    }

    bool is_paused () 
    {
        config_table config (get_self(), get_self().value);
        auto c = config.get();
        if (c.paused == PAUSED) {
            return true;
        }
        return false;
    }

    bool is_gyftie_account (name account) 
    {
        bool is_gyftie = false;

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        accounts a_t (c.gyftiecontract, account.value);
        auto a_itr = a_t.find (sym.code().raw());
        if (a_itr != a_t.end()) {
            is_gyftie = true;
        }

        profile_table p_t (c.gyftiecontract, get_self().value);
        auto p_itr = p_t.find (account.value);
        if (p_itr != p_t.end()) {
            is_gyftie = true;
        }

        return is_gyftie;
    }

    void sendfrombal ( const name token_contract,
                        const name from,
                        const name to,
                        const asset token_amount,
                        const string memo) 
    {
        if (token_amount.amount > 0) {   
            balance_table bal_table (get_self(), from.value);
            auto it = bal_table.find(token_amount.symbol.code().raw());
            eosio::check (it != bal_table.end(), "Sender does not have a balance within the contract." );
            eosio::check (it->funds >= token_amount, "Insufficient balance.");

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

    void payrewbucks_deferred () 
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        if (s.last_payrewbucks_time < current_block_time().to_time_point().sec_since_epoch()) {

            s.last_payrewbucks_time = current_block_time().to_time_point().sec_since_epoch();
            state.set (s, get_self());

            eosio::transaction out{};
            out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
                                    get_self(), "payrewbucks"_n, 
                                    std::make_tuple());
            out.delay_sec = 1;
            out.send(get_next_sender_id(), get_self());            
        }
    }

    void buildbuckets_deferred ()
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        if (s.last_bucket_build_time < current_block_time().to_time_point().sec_since_epoch()) {

            s.last_bucket_build_time = current_block_time().to_time_point().sec_since_epoch();
            state.set (s, get_self());

            eosio::transaction out{};
            out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
                                    get_self(), "buildbuckets"_n, 
                                    std::make_tuple());
            out.delay_sec = 1;
            out.send(get_next_sender_id(), get_self());            
        }
    }

    void processbook_deferred () 
    {
        eosio::transaction out{};
        out.actions.emplace_back(permission_level{_self, "owner"_n}, 
                                _self, "processbook"_n, 
                                std::make_tuple());
        out.delay_sec = 2;
        out.send(get_next_sender_id(), _self);
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

    asset get_available_balance (name account, symbol sym) 
    {
        balance_table b_t (get_self(), account.value);
        auto b_itr = b_t.find (sym.code().raw());
        eosio::check (b_itr != b_t.end(), "User does not have a balance of required asset.");

        asset open_balance =  getopenbalance (account, sym);
        return b_itr->funds - open_balance;
    }

    void confirm_balance (name account, asset min_balance) 
    {
        eosio::check (get_available_balance(account, min_balance.symbol) >= min_balance, "Insufficient funds.");
    }

    void set_last_price (asset last_price)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        s.last_price = last_price;
        state.set (s, get_self());        
    }

    asset get_last_price () 
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        return s.last_price;
    }

    asset get_lowest_sell () 
    {
        sellorder_table s_t (get_self(), get_self().value);
        auto s_index = s_t.get_index<"byprice"_n>();
        auto s_itr = s_index.begin();

        if (s_itr == s_index.end()) {
            config_table config (get_self(), get_self().value);
            auto c = config.get();
           return asset {0, c.valid_counter_token_symbol};
        }
        return s_itr->price_per_gft;
    }

    asset get_highest_buy () 
    {
        buyorder_table b_t (get_self(), get_self().value);
        auto b_index = b_t.get_index<"byprice"_n>();
        auto b_itr = b_index.rbegin();

        if (b_itr == b_index.rend()) {
            config_table config (get_self(), get_self().value);
            auto c = config.get();
            return asset {0, c.valid_counter_token_symbol};
        }
        return b_itr->price_per_gft;
    }

    asset adjust_asset (asset original_asset, float adjustment)
    {
        return asset { static_cast<int64_t> (original_asset.amount * adjustment), original_asset.symbol };
    }

    // void print_state () 
    // {
    //     state_table state (get_self(), get_self().value);
    //     State s = state.get();
    //     print (" \nCurrent State: \n");
    //     print (" Sell orderbook size:   ", s.sell_orderbook_size_gft, "\n");
    //     print (" Buy orderbook size:    ", s.buy_orderbook_size_gft, "\n");
    //     print (" Last Price:            ", s.last_price, "\n");
    // }

    void add_bucket_reward (uint64_t bucket_id, asset reward_amount)
    {
        orderbucket_table ob_t (get_self(), get_self().value);
        auto ob_itr = ob_t.find (bucket_id);
        eosio::check (ob_itr != ob_t.end(), "Bucket ID is not found.");

        liqreward_table l_t (get_self(), get_self().value);
        Liqreward l = l_t.get();
        // control for rounding overflow
        asset actual_reward = asset { std::min(l.availreward.amount, 
                                                    reward_amount.amount), reward_amount.symbol};

        ob_t.modify (ob_itr, get_self(), [&](auto &ob) {
            ob.reward_due += actual_reward;
        });

        l.availreward -= actual_reward;
        l_t.set (l, get_self());
    }

    void clr_bucketuser (uint64_t bucket_id)
    {
        bucketuser_table bu_t (get_self(), bucket_id);
        auto bu_itr = bu_t.begin();
        while (bu_itr != bu_t.end()) {
            bu_itr = bu_t.erase (bu_itr);
        }
    }

    void add_bucketuser (uint64_t bucket_id, name user, asset gft_amount)
    {
        bucketuser_table bu_t (get_self(), bucket_id);
        auto bu_itr = bu_t.find (user.value);
        if (bu_itr == bu_t.end()) {
            bu_t.emplace (get_self(), [&](auto &bu) {
                bu.user = user;
                bu.bucketuser_size = gft_amount;
            });
        } else {
            bu_t.modify (bu_itr, get_self(), [&](auto &bu) {
                bu.bucketuser_size += gft_amount;
            });
        }
    }

    void decrease_sellgft_liquidity (asset gft_sold)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        s.sell_orderbook_size_gft -= gft_sold;
        state.set (s, get_self());
    }

    void decrease_buygft_liquidity (asset gft_bought)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        s.buy_orderbook_size_gft -= gft_bought;
        state.set (s, get_self());
    }

    void increase_sellgft_liquidity (asset new_gft_added)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        s.sell_orderbook_size_gft += new_gft_added;
        state.set (s, get_self());
    }

    void increase_buygft_liquidity (asset new_gft_added)
    {
        state_table state (get_self(), get_self().value);
        State s = state.get();
        s.buy_orderbook_size_gft += new_gft_added;
        state.set (s, get_self());
    }

    void add_limitbuy_order (name buyer, asset price_per_gft, asset gft_amount)
    {
        eosio::check (price_per_gft.amount > 0, "Price must be greater than zero.");
        eosio::check (gft_amount.amount > 0, "GFT amount must be greater than zero.");

        confirm_balance (buyer, get_eos_order_value(price_per_gft, gft_amount));
        increase_buygft_liquidity (gft_amount);

        buyorder_table b_t (get_self(), get_self().value);
        b_t.emplace (get_self(), [&](auto &b) {
            b.order_id = b_t.available_primary_key();
            b.buyer = buyer;
            b.price_per_gft = price_per_gft;
            b.gft_amount = gft_amount;
            b.order_value = get_eos_order_value(price_per_gft, gft_amount);
            b.created_date = current_block_time().to_time_point().sec_since_epoch();
        });
    }

    void add_limitsell_order (name seller, asset price_per_gft, asset gft_amount)
    {
        eosio::check (price_per_gft.amount > 0, "Price must be greater than zero.");
        eosio::check (gft_amount.amount > 0, "GFT amount must be greater than zero.");

        confirm_balance (seller, gft_amount);
        increase_sellgft_liquidity (gft_amount);

        sellorder_table s_t (get_self(), get_self().value);
        s_t.emplace (get_self(), [&](auto &s) {
            s.order_id = s_t.available_primary_key();
            s.seller = seller;
            s.price_per_gft = price_per_gft;
            s.gft_amount = gft_amount;
            s.order_value = get_eos_order_value (price_per_gft, gft_amount);
            s.created_date = current_block_time().to_time_point().sec_since_epoch();
        });
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
        // decrease_buygft_liquidity (gft_amount);

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
        eosio::check (s_itr != s_t.end(), "Sell Order ID does not exist.");

        asset eos_amount = asset { std::min (eos_to_spend.amount, s_itr->order_value.amount),
                                   eos_to_spend.symbol };

        confirm_balance (buyer, eos_amount);
     
        settle_seller_maker (buyer, s_itr->seller, s_itr->price_per_gft, 
                             get_gft_amount (s_itr->price_per_gft, eos_amount));

        if (eos_amount == s_itr->order_value) {
            decrease_sellgft_liquidity (s_itr->gft_amount);
            s_t.erase (s_itr);
        } else if (s_itr->order_value > eos_amount) {
            decrease_sellgft_liquidity (get_gft_amount (s_itr->price_per_gft, eos_amount));
            s_t.modify (s_itr, get_self(), [&](auto &s) {
                s.gft_amount -= get_gft_amount (s_itr->price_per_gft, eos_amount);
                s.order_value = get_eos_order_value (s_itr->price_per_gft, s.gft_amount);
            });
        }

        // buildbuckets_deferred();
    }

    void sellgft (uint64_t buyorder_id, name seller, asset gft_to_sell) 
    {
        // require_auth (seller);

        buyorder_table b_t (get_self(), get_self().value);
        auto b_itr = b_t.find (buyorder_id);
        eosio::check (b_itr != b_t.end(), "Buy Order ID does not exist.");

        asset gft_amount = asset { std::min (gft_to_sell.amount, b_itr->gft_amount.amount),
                                   gft_to_sell.symbol };

        confirm_balance (seller, gft_amount);

        settle_buyer_maker (b_itr->buyer, seller, b_itr->price_per_gft, gft_amount);

        if (gft_amount == b_itr->gft_amount) {
            decrease_buygft_liquidity (b_itr->gft_amount);
            b_t.erase (b_itr);
        } else if (b_itr->gft_amount > gft_amount) {
            decrease_buygft_liquidity (gft_amount);
            b_t.modify (b_itr, get_self(), [&](auto &b) {
                b.gft_amount -= gft_amount;
                b.order_value = get_eos_order_value (b_itr->price_per_gft, b.gft_amount);
            });
        }

        // buildbuckets_deferred();
    }

    asset get_eos_order_value (asset price_per_gft, asset gft_amount) 
    {
        // uint64_t gft_quantity = gft_amount.amount; 
        asset order_value = price_per_gft * gft_amount.amount / pow(10,GYFTIE_PRECISION);
        eosio::check (order_value.amount > 0, "Order value is less than 0.0001 EOS. Increase amount.");
        return order_value;
    }

    asset get_gft_amount (asset price_per_gft, asset eos_amount)
    {
        // print (" Price per GFT: ", price_per_gft, "\n");
        // print (" EOS Amount: ", eos_amount, "\n");

        symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        float gft_quantity = pow(10,GYFTIE_PRECISION) * eos_amount.amount / price_per_gft.amount;
        // print (" GFT Quantity: ", gft_quantity, "\n");
        // print (" GFT Amount: ", asset { static_cast<int64_t>(gft_quantity), gft_symbol }, "\n");
        return asset { static_cast<int64_t>(gft_quantity), gft_symbol };
        //return gft_amount;
    }

    void match_order (uint64_t sellorder_id, uint64_t buyorder_id)
    {
        // handle trade between two orders, decrementally the lower of the two appropriately
        sellorder_table s_t (get_self(), get_self().value);
        auto s_itr = s_t.find (sellorder_id);
        eosio::check (s_itr != s_t.end(), "Sell Order ID does not exist.");

        buyorder_table b_t (get_self(), get_self().value);
        auto b_itr = b_t.find (buyorder_id);
        eosio::check (b_itr != b_t.end(), "Buy Order ID does not exist.");

        eosio::check (b_itr->price_per_gft >= s_itr->price_per_gft, "Buyer's price is less than the seller's price.");

        asset gft_amount = asset { std::min (b_itr->gft_amount.amount, s_itr->gft_amount.amount),
                                   b_itr->gft_amount.symbol };
           
        // if seller is market maker
        if (s_itr->created_date < b_itr->created_date) {
            settle_seller_maker (b_itr->buyer, s_itr->seller, s_itr->price_per_gft, gft_amount);
        } else { // buyer is market maker
            settle_buyer_maker(b_itr->buyer, s_itr->seller, b_itr->price_per_gft, gft_amount);
        }

        if (b_itr->gft_amount == s_itr->gft_amount) {
            decrease_buygft_liquidity (b_itr->gft_amount);
            decrease_sellgft_liquidity (s_itr->gft_amount);
            s_t.erase (s_itr);
            b_t.erase (b_itr); 
        } else if (b_itr->gft_amount > s_itr->gft_amount) {
            decrease_buygft_liquidity (s_itr->gft_amount);
            b_t.modify (b_itr, get_self(), [&](auto &b) {
                b.gft_amount -= s_itr->gft_amount;
                b.order_value = get_eos_order_value (b_itr->price_per_gft, b.gft_amount);
            });
            decrease_sellgft_liquidity (s_itr->gft_amount);
            s_t.erase (s_itr);
        } else if (s_itr->gft_amount > b_itr->gft_amount) {
            decrease_sellgft_liquidity (b_itr->gft_amount);
            s_t.modify (s_itr, get_self(), [&](auto &s) {
                s.gft_amount -= b_itr->gft_amount;
                s.order_value = get_eos_order_value (s_itr->price_per_gft, s.gft_amount);
            });
            decrease_buygft_liquidity (b_itr->gft_amount);
            b_t.erase (b_itr);
        }
    }
};