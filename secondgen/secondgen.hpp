
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <string>

using std::string;
using namespace eosio;

CONTRACT secondgen : public contract
{
    using contract::contract;

  public:

    ACTION generate (name token_contract,
                     string symbol_string,
                     uint8_t    symbol_precision,
                     name from,
                     name to);

  private:

    
    TABLE tokengen
    {
        uint64_t    pk;
        name        from;
        name        to;
        asset       generated_amount;        
        uint64_t    primary_key() const { return pk; }
    };

    typedef eosio::multi_index<"tokengens"_n, tokengen> tokengen_table;
};
