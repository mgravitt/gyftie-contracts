
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <string>

using std::string;
using namespace eosio;

CONTRACT genesisgen : public contract
{
    using contract::contract;

  public:

    ACTION setconfig (const asset gft_eos_rate,
                        const float gyfter_payback_rate);

    ACTION generate (   name from,
                            asset gyfter_gft_balance,
                            name to);

    ACTION reset ();
    
  private:

    TABLE Config
    {
        asset       gft_eos_rate;
        float       gyfter_payback_rate;      
    };

    typedef singleton<"configs"_n, Config> config_table;
    typedef eosio::multi_index<"configs"_n, Config> config_table_placeholder;

    
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

    typedef eosio::multi_index<"accounts"_n, account> accounts;
};
