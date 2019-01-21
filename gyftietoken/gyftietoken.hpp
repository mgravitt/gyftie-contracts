
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
#include <algorithm>    // std::find
#include <eosiolib/singleton.hpp>

using std::string;
using std::vector;
using namespace eosio;

CONTRACT gyftietoken : public contract
{
    using contract::contract;

  public:

    ACTION setconfig (name token_gen);

    ACTION create();

    ACTION issue(name to, asset quantity, string memo);

    ACTION transfer(name from, name to, asset quantity, string memo);

    ACTION calcgyft (name from, name to);
    
    ACTION gyft (name from, name to, string idhash);

    ACTION propose (name proposer, name token_gen, string notes);

    ACTION vote (name voter, uint64_t   proposal_id);

    // TESTING ONLY -- DELETE FOR PRODUCTION
    ACTION reset () ;

    // TESTING ONLY -- DELETE FOR PRODUCTION
    ACTION burnall (name tokenholder);

  private:

    const string    GYFTIE_SYM_STR  = "GFT";
    const uint8_t   GYFTIE_PRECISION = 8;

    TABLE Config
    {
        name        token_gen;
        uint32_t    account_count = 0;        
    };

    typedef singleton<"configs"_n, Config> config_table;
    typedef eosio::multi_index<"configs"_n, Config> config_table_placeholder;

    TABLE proposal 
    {
        uint64_t        proposal_id;
        uint32_t        created_date;
        name            proposer;
        name            new_token_gen;
        string          notes;
        vector<name>    voters;
        uint32_t        votes_for;
        uint32_t        expiration_date;
        uint64_t        primary_key() const { return proposal_id; }
    };

    typedef eosio::multi_index<"proposals"_n, proposal> proposal_table;

    TABLE tokengen
    {
        uint64_t    pk;
        name        from;
        name        to;
        asset       generated_amount;        
        uint64_t    primary_key() const { return pk; }
    };

    typedef eosio::multi_index<"tokengens"_n, tokengen> tokengen_table;
    
    TABLE account
    {
        asset   balance;
        string  idhash;
        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };

    TABLE currency_stats
    {
        asset supply;
        symbol symbol;
        name issuer;
        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;
    typedef eosio::multi_index<"stat"_n, currency_stats> stats;

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

    bool is_gyftie_account (name account) 
    {
        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        accounts a_t (get_self(), account.value);
        auto a_itr = a_t.find (sym.code().raw());
        if (a_itr == a_t.end()) {
            return false;
        }
        return true;
    }

    void increment_account_count () 
    {
        config_table config (get_self(), get_self().value);
        auto c = config.get();
        c.account_count++;
        config.set (c, get_self());
    }

    void decrement_account_count () 
    {
        config_table config (get_self(), get_self().value);
        auto c = config.get();
        c.account_count--;
        config.set (c, get_self());
    }

    void save_idhash (const name account, const string idhash) 
    {
        accounts a_t (get_self(), account.value);
        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        auto a_itr = a_t.find (sym.code().raw());
        
        if (a_itr == a_t.end()) {
            asset zero_balance = asset { 0, sym };
            a_t.emplace (get_self(), [&](auto &a) {
                a.balance = zero_balance;
                a.idhash = idhash;
            });
        } else {
            a_t.modify (a_itr, get_self(), [&](auto &a) {
                a.idhash = idhash;
            });
        }
    }
};
