
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

    ACTION generate ( name from,
                      asset gyfter_gft_balance,
                      name to);
    
  private:

    TABLE tokengen
    {
        name        from;
        name        to;
        asset       generated_amount;        
    };

    typedef singleton<"tokengens"_n, tokengen> tokengen_table;
    typedef eosio::multi_index<"tokengens"_n, tokengen> tokengen_table_placeholder;

};
