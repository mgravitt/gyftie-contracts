
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

    ACTION create(name issuer);

    ACTION issue(name to, asset quantity, string memo);

    ACTION transfer(name from, name to, asset quantity, string memo);
    
    ACTION gyft (name from, name to, string idhash, string memo);

    ACTION propose (name proposer, name token_gen, string notes);

    ACTION vote (name voter, uint64_t   proposal_id);

    static asset get_supply(name gyftietoken_contract_account, symbol_code sym)
    {
        stats statstable(gyftietoken_contract_account, sym.raw());
        const auto &st = statstable.get(sym.raw());
        return st.supply;
    }

    static asset get_balance(name gyftietoken_contract_account, name owner, symbol_code sym)
    {
        accounts accountstable(gyftietoken_contract_account, owner.value);
        const auto &ac = accountstable.get(sym.raw());
        return ac.balance;
    }

  private:

    const string    GYFTIE_SYM_STR  = "GYFTIE";
    const uint8_t   GYFTIE_PRECISION = 4;

    TABLE config
    {
        name        token_gen;
        uint32_t    account_count;        
    };

    typedef singleton<"configs"_n, config> config_singleton;

    TABLE proposal 
    {
        uint64_t        proposal_id;
        uint32_t        created_date;
        name            proposer;
        name            new_token_gen;
        vector<name>    voters;
        uint32_t        votes_for;
        uint32_t        expiration_date;
        uint64_t        primary_key() const { return proposal_id; }
    };

    typedef eosio::multi_index<"proposals"_n, proposal> proposal_table;

    TABLE tokengen
    {
        asset    generated_amount;        
    };

    typedef singleton<"tokengens"_n, tokengen> tokengen_singleton;
    
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
};
