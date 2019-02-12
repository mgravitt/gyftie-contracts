
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
#include <algorithm>    // std::find
#include <eosiolib/singleton.hpp>
#include <math.h>

using std::string;
using std::vector;
using namespace eosio;

CONTRACT gyftietoken : public contract
{
    using contract::contract;

  public:

    ACTION setconfig (name token_gen, name gftorderbook, name gyftie_foundation);

    ACTION delconfig ();

    // ACTION sudoprofile (name account);

    ACTION addrating (name rater, name ratee, uint8_t rating);

    ACTION create();

    ACTION issue(name to, asset quantity, string memo);

    ACTION transfer(name from, name to, asset quantity, string memo);

    ACTION calcgyft (name from, name to);
    
    ACTION gyft (name from, name to, string idhash);

    ACTION addgyft (name gyfter, name gyftee, asset gyfter_issue,
                                asset gyftee_issue);

    ACTION propose (name proposer, string notes);

    ACTION votefor (name voter, uint64_t   proposal_id);

    ACTION voteagainst (name voter, uint64_t   proposal_id);

    ACTION removeprop (uint64_t proposal_id);

    ACTION setcounter (uint64_t account_count);

    ACTION pause ();

    ACTION unpause ();

  private:

    const string    GYFTIE_SYM_STR  = "GFT";
    const uint8_t   GYFTIE_PRECISION = 8;

    const uint8_t   PAUSED = 1;
    const uint8_t   UNPAUSED = 0;

    TABLE Config
    {
        name        token_gen;
        name        gftorderbook;
        name        gyftie_foundation;
        uint8_t     paused;
    };

    typedef singleton<"configs"_n, Config> config_table;
    typedef eosio::multi_index<"configs"_n, Config> config_table_placeholder;

    TABLE Counter 
    {
        uint32_t    account_count = 0;
    };

    typedef singleton<"counters"_n, Counter> counter_table;
    typedef eosio::multi_index<"counters"_n, Counter> counter_table_placeholder;

    TABLE proposal 
    {
        uint64_t        proposal_id;
        uint32_t        created_date;
        name            proposer;
        name            new_token_gen;
        string          notes;
        vector<name>    voters_for;
        uint32_t        votes_for;
        vector<name>    voters_against;
        uint32_t        votes_against;
        uint32_t        expiration_date;
        uint64_t        primary_key() const { return proposal_id; }
    };

    typedef eosio::multi_index<"proposals"_n, proposal> proposal_table;

    TABLE tokengen
    {
        name        from;
        name        to;
        asset       generated_amount;        
    };

    typedef singleton<"tokengens"_n, tokengen> tokengen_table;
    typedef eosio::multi_index<"tokengens"_n, tokengen> tokengen_table_placeholder;
    
    TABLE account
    {
        asset   balance;
        string  idhash;
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
        uint64_t    primary_key() const { return account.value; }
    };
    typedef eosio::multi_index<"profiles"_n, profile> profile_table;

    TABLE availrating
    {
        name        ratee;
        uint32_t    rate_deadline;
        uint64_t    primary_key() const { return ratee.value; }
    };
    typedef eosio::multi_index<"availratings"_n, availrating> availrating_table;
    

    // TABLE gyft 
    // {
    //     uint64_t    gyft_id;
    //     name        gyfter;
    //     name        gyftee;
    //     asset       gyfter_issue;
    //     asset       gyftee_issue;
    //     uint32_t    gyft_date;
    //     uint64_t    primary_key() const { return gyft_id; }
    // };

    // typedef eosio::multi_index<"gyfts"_n, gyft> gyft_table;

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

    void sub_balance(name owner, asset value);
    void add_balance(name owner, asset value, name ram_payer);

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

    bool is_tokenholder (name account) 
    {
        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
    
        accounts a_t (get_self(), account.value);
        auto a_itr = a_t.find (sym.code().raw());
        if (a_itr == a_t.end()) {
            return false;
        }

        asset zero_balance = asset { 0, sym };
        if (a_itr->balance <= zero_balance) {
            return false;
        }        
        return true;
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

        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        accounts a_t (get_self(), account.value);
        auto a_itr = a_t.find (sym.code().raw());
        if (a_itr != a_t.end()) {
            is_gyftie = true;
        }

        profile_table p_t (get_self(), get_self().value);
        auto p_itr = p_t.find (account.value);
        if (p_itr != p_t.end()) {
            is_gyftie = true;
        }

        return is_gyftie;
    }

    void insert_profile (const name account, const string idhash)
    {
        profile_table p_t (get_self(), get_self().value);
        auto p_itr = p_t.find (account.value);
        eosio_assert (p_itr == p_t.end(), "Account already has a profile.");

        p_t.emplace (get_self(), [&](auto &p) {
            p.account = account;
            p.rating_count = 0;
            p.rating_sum = 0;
            p.idhash = idhash;
        });
    }

    void increment_account_count () 
    {
        counter_table counter (get_self(), get_self().value);
        auto c = counter.get();
        c.account_count++;
        counter.set (c, get_self());
    }

    // void onboard (const name account, const string idhash) 
    // {
    //     accounts a_t (get_self(), account.value);
    //     symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
    //     auto a_itr = a_t.find (sym.code().raw());
        
    //     if (a_itr == a_t.end()) {
    //         asset zero_balance = asset { 0, sym };
    //         a_t.emplace (get_self(), [&](auto &a) {
    //             a.balance = zero_balance;
    //             a.idhash = idhash;
    //         });
    //     } else {
    //         a_t.modify (a_itr, get_self(), [&](auto &a) {
    //             a.idhash = idhash;
    //         });
    //     }
    // }

    asset getgftbalance (name account)
    {
        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        asset gft_balance = asset { 0, sym };

        accounts a_t (get_self(), account.value);
        auto a_itr = a_t.find (sym.code().raw());
        if (a_itr != a_t.end()) {
            gft_balance += a_itr->balance;
        }

        config_table config (get_self(), get_self().value);
        auto c = config.get();

        balance_table b_t (c.gftorderbook, account.value);
        auto b_itr = b_t.find (sym.code().raw());
        if (b_itr != b_t.end() && b_itr->token_contract == get_self()) {
            gft_balance += b_itr->funds;
        }
        
        return gft_balance;
    }

};
