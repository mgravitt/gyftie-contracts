
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

    ACTION generate (name token_contract,
                     string symbol_string,
                     uint8_t    symbol_precision,
                     name from,
                     name to);

  private:

    
    TABLE tokengen
    {
        asset    generated_amount;        
    };

    typedef singleton<"tokengens"_n, tokengen> tokengen_singleton;
   
};
