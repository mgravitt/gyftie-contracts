#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
#include <algorithm> // std::find
#include <eosiolib/singleton.hpp>
#include <math.h>

#include "includes/abieos_numeric.hpp"

using std::string;
using std::vector;
using namespace eosio;

CONTRACT gyftietoken : public contract
{
    using contract::contract;

  public:
    // ACTION copybal1();
    // ACTION remaccounts();
    // ACTION repopaccts();
    // ACTION remtemp();
    ACTION setcount (uint32_t count);

    ACTION setconfig(name token_gen, name gftorderbook, name gyftie_foundation);

    ACTION delconfig();

    // ACTION sudoprofile (name account);

    ACTION addrating(name rater, name ratee, uint8_t rating);

    ACTION create();

    ACTION issue(name to, asset quantity, string memo);

    ACTION transfer(name from, name to, asset quantity, string memo);

    ACTION calcgyft(name from, name to);

    ACTION gyft(name from, name to, string idhash, string relationship);

    ACTION ungyft(name account);

    ACTION propose(name proposer, string notes);

    ACTION votefor(name voter, uint64_t proposal_id);

    ACTION voteagainst(name voter, uint64_t proposal_id);

    ACTION removeprop(uint64_t proposal_id);

    ACTION setcounter(uint64_t account_count);

    ACTION pause();

    ACTION unpause();

  private:
    const string GYFTIE_SYM_STR = "GFT";
    const uint8_t GYFTIE_PRECISION = 8;
    const string symbol_name = "EOS";
    symbol network_symbol = symbol(symbol_name, 4);

    const uint8_t PAUSED = 1;
    const uint8_t UNPAUSED = 0;

    TABLE Config
    {
        name token_gen;
        name gftorderbook;
        name gyftie_foundation;
        uint8_t paused;
    };

    typedef singleton<"configs"_n, Config> config_table;
    typedef eosio::multi_index<"configs"_n, Config> config_table_placeholder;

    TABLE Counter
    {
        uint32_t account_count = 0;
    };

    typedef singleton<"counters"_n, Counter> counter_table;
    typedef eosio::multi_index<"counters"_n, Counter> counter_table_placeholder;

    TABLE proposal
    {
        uint64_t proposal_id;
        uint32_t created_date;
        name proposer;
        name new_token_gen;
        string notes;
        vector<name> voters_for;
        uint32_t votes_for;
        vector<name> voters_against;
        uint32_t votes_against;
        uint32_t expiration_date;
        uint64_t primary_key() const { return proposal_id; }
    };

    typedef eosio::multi_index<"proposals"_n, proposal> proposal_table;

    TABLE tokengen
    {
        name from;
        name to;
        asset generated_amount;
    };

    typedef singleton<"tokengens"_n, tokengen> tokengen_table;
    typedef eosio::multi_index<"tokengens"_n, tokengen> tokengen_table_placeholder;

    TABLE account
    {
        asset balance;
        string idhash;
        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };
    typedef eosio::multi_index<"accounts"_n, account> accounts;

    TABLE tempacc
    {
        name account;
        asset balance;
        uint64_t primary_key() const { return account.value; }
    };

    typedef eosio::multi_index<"tempaccts"_n, tempacc> tempacct_table;

    TABLE profile
    {
        name account;
        uint32_t rating_sum;
        uint16_t rating_count;
        string idhash;
        string id_expiration;
        uint64_t primary_key() const { return account.value; }
    };
    typedef eosio::multi_index<"profiles"_n, profile> profile_table;

    TABLE availrating
    {
        name ratee;
        uint32_t rate_deadline;
        uint64_t primary_key() const { return ratee.value; }
    };
    typedef eosio::multi_index<"availratings"_n, availrating> availrating_table;

    TABLE gyftevent
    {
        uint64_t gyft_id;
        name gyfter;
        name gyftee;
        asset gyfter_issue;
        asset gyftee_issue;
        string relationship;
        string notes;
        uint32_t gyft_date;
        uint16_t likes;
        uint64_t primary_key() const { return gyft_id; }
        // uint64_t    by_gyfter() const { return gyfter.value; }
        // uint64_t    by_gyftee() const { return gyftee.value; }
    };

    typedef eosio::multi_index<"gyfts"_n, gyftevent
                               // indexed_by<"bygyfter"_n,
                               //     const_mem_fun<gyftevent, uint64_t, &gyftevent::by_gyfter>>,
                               // indexed_by<"bygyftee"_n,
                               //     const_mem_fun<gyftevent, uint64_t, &gyftevent::by_gyftee>>
                               >
        gyft_table;

    TABLE currency_stats
    {
        asset supply;
        symbol symbol;
        name issuer;
        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"stat"_n, currency_stats> stats;

    TABLE balance
    {
        asset funds;
        name token_contract;
        uint64_t primary_key() const { return funds.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"balances"_n, balance> balance_table;

    TABLE buyorder
    {
        uint64_t order_id;
        name buyer;
        asset price_per_gft;
        asset gft_amount;
        asset order_value;
        uint32_t created_date;
        uint64_t primary_key() const { return order_id; }
        uint64_t by_buyer() const { return buyer.value; }
        uint64_t by_price() const { return price_per_gft.amount; }
    };

    typedef eosio::multi_index<"buyorders"_n, buyorder,
                               indexed_by<"bybuyer"_n,
                                          const_mem_fun<buyorder, uint64_t, &buyorder::by_buyer>>,
                               indexed_by<"byprice"_n,
                                          const_mem_fun<buyorder, uint64_t, &buyorder::by_price>>>
        buyorder_table;

    TABLE gyftrequest
    {
        name recipient;
        name gyfter;
        string owner_public_key;
        string active_public_key;
        uint32_t requested_date;
        uint64_t primary_key() const { return recipient.value; }
        uint64_t by_gyfter() const { return gyfter.value; }
    };

    typedef eosio::multi_index<"gyftrequests"_n, gyftrequest,
                               indexed_by<"bygyfter"_n,
                                          const_mem_fun<gyftrequest, uint64_t, &gyftrequest::by_gyfter>>>
        gyftrequest_table;

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

    void sub_balance(name owner, asset value);
    void add_balance(name owner, asset value, name ram_payer);

    authority keystring_authority(string key_str)
    {

        // Convert string to key type
        const abieos::public_key key = abieos::string_to_public_key(key_str);

        // Setup authority
        authority ret_authority;

        // Array to hold public key
        std::array<char, 33> key_char;

        // Copy key to char array
        std::copy(key.data.begin(), key.data.end(), key_char.begin());

        key_weight kweight{
            .key = {(uint8_t)abieos::key_type::k1, key_char},
            .weight = (uint16_t)1};

        // Authority
        ret_authority.threshold = 1;
        ret_authority.keys = {kweight};
        ret_authority.accounts = {};
        ret_authority.waits = {};

        return ret_authority;
    }

    void paytoken(const name token_contract,
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

    void addgyft(name gyfter, name gyftee, asset gyfter_issue,
                 asset gyftee_issue, string relationship)
    {
        gyft_table g_t(get_self(), get_self().value);
        g_t.emplace(get_self(), [&](auto &g) {
            g.gyft_id = g_t.available_primary_key();
            g.gyfter = gyfter;
            g.gyftee = gyftee;
            g.gyfter_issue = gyfter_issue;
            g.gyftee_issue = gyftee_issue;
            g.relationship = relationship;
            // g.notes = notes;
            g.gyft_date = now();
        });
    }

    asset adjust_asset(asset original_asset, float adjustment)
    {
        return asset{static_cast<int64_t>(original_asset.amount * adjustment), original_asset.symbol};
    }

    bool is_tokenholder(name account)
    {
        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

        accounts a_t(get_self(), account.value);
        auto a_itr = a_t.find(sym.code().raw());
        if (a_itr == a_t.end())
        {
            return false;
        }

        asset zero_balance = asset{0, sym};
        if (a_itr->balance <= zero_balance)
        {
            return false;
        }
        return true;
    }

    bool is_paused()
    {
        config_table config(get_self(), get_self().value);
        auto c = config.get();
        if (c.paused == PAUSED)
        {
            return true;
        }
        return false;
    }

    bool is_gyftie_account(name account)
    {
        bool is_gyftie = false;

        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        accounts a_t(get_self(), account.value);
        auto a_itr = a_t.find(sym.code().raw());
        if (a_itr != a_t.end())
        {
            is_gyftie = true;
        }

        profile_table p_t(get_self(), get_self().value);
        auto p_itr = p_t.find(account.value);
        if (p_itr != p_t.end())
        {
            is_gyftie = true;
        }

        return is_gyftie;
    }

    void insert_profile(const name account, const string idhash)
    {
        profile_table p_t(get_self(), get_self().value);
        auto p_itr = p_t.find(account.value);
        eosio_assert(p_itr == p_t.end(), "Account already has a profile.");

        p_t.emplace(get_self(), [&](auto &p) {
            p.account = account;
            p.rating_count = 0;
            p.rating_sum = 0;
            p.idhash = idhash;
        });
    }

    void increment_account_count()
    {
        counter_table counter(get_self(), get_self().value);
        auto c = counter.get();
        c.account_count++;
        counter.set(c, get_self());
    }

    void burn(name account, asset quantity)
    {
        stats statstable(get_self(), quantity.symbol.code().raw());
        auto existing = statstable.find(quantity.symbol.code().raw());
        eosio_assert(existing != statstable.end(), "token with symbol does not exist");
        const auto &st = *existing;

        eosio_assert(quantity.is_valid(), "invalid quantity");
        eosio_assert(quantity.amount > 0, "must issue positive quantity");
        eosio_assert(quantity.symbol == st.symbol, "symbol precision mismatch");

        statstable.modify(st, eosio::same_payer, [&](auto &s) {
            s.supply -= quantity;
        });

        sub_balance(account, quantity);
    }

    asset getgftbalance(name account)
    {
        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        asset gft_balance = asset{0, sym};

        accounts a_t(get_self(), account.value);
        auto a_itr = a_t.find(sym.code().raw());
        if (a_itr != a_t.end())
        {
            gft_balance += a_itr->balance;
        }

        config_table config(get_self(), get_self().value);
        auto c = config.get();

        balance_table b_t(c.gftorderbook, account.value);
        auto b_itr = b_t.find(sym.code().raw());
        if (b_itr != b_t.end() && b_itr->token_contract == get_self())
        {
            gft_balance += b_itr->funds;
        }

        return gft_balance;
    }

    asset create_account_from_request(name gyfter, name recipient)
    {
        gyftrequest_table g_t("gyftmultisig"_n, "gyftmultisig"_n.value);
        auto g_itr = g_t.find(recipient.value);
        eosio_assert(g_itr != g_t.end(), "No gyft request matching this recipient.");
        eosio_assert (g_itr->gyfter == gyfter, "Gyft request does not match this gyfter.");

        // New account resources
        asset ram = asset(3000, network_symbol); // 0.3
        asset cpu = asset(900, network_symbol);  // 0.09
        asset net = asset(100, network_symbol);  // 0.01

        authority owner_auth = keystring_authority(g_itr->owner_public_key);
        authority active_auth = keystring_authority(g_itr->active_public_key);

        // Create account
        action(
            permission_level{get_self(), name("owner")},
            name("eosio"), name("newaccount"),
            std::make_tuple(get_self(), recipient, owner_auth, active_auth))
            .send();

        // Buy ram
        action(
            permission_level{get_self(), name("owner")},
            name("eosio"), name("buyram"),
            std::make_tuple(get_self(), recipient, ram))
            .send();

        // Delegate CPU/NET
        action(
            permission_level{get_self(), name("owner")},
            name("eosio"), name("delegatebw"),
            std::make_tuple(get_self(), recipient, net, cpu, 1))
            .send();

        asset reimbursement = get_market_gft_requirement (ram + cpu + net);
        string market_sell_for_reimbursement { "Market Sell for Account Creation Reimbursement"};

        config_table config(get_self(), get_self().value);
        auto c = config.get();

        action (
            permission_level{get_self(), "owner"_n},
            get_self(), "issue"_n,
            std::make_tuple(c.gftorderbook, reimbursement, market_sell_for_reimbursement))
        .send();

        action (
            permission_level{get_self(), "owner"_n},
            c.gftorderbook, "marketsell"_n,
            std::make_tuple(get_self(), reimbursement))
        .send();   

        return reimbursement;
    }

    asset get_gft_amount(asset price_per_gft, asset eos_amount)
    {
        symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        float gft_quantity = pow(10, GYFTIE_PRECISION) * eos_amount.amount / price_per_gft.amount;
        return asset{static_cast<int64_t>(gft_quantity), gft_symbol};
    }

    asset get_market_gft_requirement(asset eos_amount)
    {
        config_table config(get_self(), get_self().value);
        auto c = config.get();

        buyorder_table b_t(c.gftorderbook, c.gftorderbook.value);
        auto b_index = b_t.get_index<"byprice"_n>();
        auto b_itr = b_index.rbegin();

        eos_amount = adjust_asset(eos_amount, 1.01); // account for trade fees
        asset gft_requirement = asset{0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};

        while (eos_amount.amount > 0 && b_itr != b_index.rend())
        {
            asset trade_amount_eos = asset{std::min(b_itr->order_value.amount, eos_amount.amount), eos_amount.symbol};
            asset trade_amount_gft = get_gft_amount(b_itr->price_per_gft, trade_amount_eos);
            gft_requirement += trade_amount_gft;
            eos_amount -= trade_amount_eos;
            b_itr++;
        }
        return gft_requirement;
    }

    asset get_replenish_minimum_eos_req(name account)
    {
        asset minimum_eos = asset{10100, symbol{"EOS", 4}};
        asset eos_needed = minimum_eos * 0;

        accounts eos_a_t("eosio.token"_n, account.value);
        auto eos_a_itr = eos_a_t.find(minimum_eos.symbol.code().raw());
        if (eos_a_itr == eos_a_t.end())
        {
            eos_needed = minimum_eos;
        }
        else
        {
            eos_needed = asset{std::max((int64_t)minimum_eos.amount - eos_a_itr->balance.amount, (int64_t)0), minimum_eos.symbol};
        }

        return get_market_gft_requirement(eos_needed);
    }

    float get_usercount_factor ()
    {
        uint16_t usercount_baseline = 195;
        counter_table counter(get_self(), get_self().value);
        auto c = counter.get();
        float percent_increase_user_count = (float) (c.account_count - usercount_baseline) / (float) usercount_baseline;

        float double_decrease = -1 * 2 * percent_increase_user_count;
        return double_decrease;
    }

    asset get_one_gft()
    {
        symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        return asset { static_cast<int64_t>(pow(10, GYFTIE_PRECISION)), gft_symbol};
    }

    asset get_recipient_reward ()
    {
        //print (" \nDecrease recipient reward by : ", get_usercount_factor(), "\n");
        return get_one_gft();
    }

    asset get_gyfter_reward (name gyfter)
    {
        //print (" \nDecrease gyfter reward by : ", get_usercount_factor(), "\n");

        asset one_gyftie_token = get_one_gft();
        asset gyfter_gft_balance = getgftbalance (gyfter);

        //double gyft_benefit= 0.0;
        asset gyft_benefit_amount = one_gyftie_token;

        if (gyfter_gft_balance  <= (one_gyftie_token * 3)) {
            gyft_benefit_amount = one_gyftie_token * 3;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 10)) {
            gyft_benefit_amount = one_gyftie_token * 4;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 20)) {
            gyft_benefit_amount = one_gyftie_token * 5;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 50)) {
            gyft_benefit_amount = one_gyftie_token * 6;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 100)) {
            gyft_benefit_amount = one_gyftie_token * 7;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 200)) {
            gyft_benefit_amount = one_gyftie_token * 8;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 500)) {
            gyft_benefit_amount = one_gyftie_token * 9;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 1000)) {
            gyft_benefit_amount = one_gyftie_token * 10;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 2000)) {
            gyft_benefit_amount = one_gyftie_token * 11;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 5000)) {
            gyft_benefit_amount = one_gyftie_token * 12;
        } else if (gyfter_gft_balance  <= (one_gyftie_token * 10000)) {
            gyft_benefit_amount = one_gyftie_token * 15;
        } else {
            gyft_benefit_amount = one_gyftie_token * 20;
        }

        return gyft_benefit_amount;
    }
};
