
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <string>

using std::string;
using namespace eosio;

CONTRACT gftmultisig : public contract
{
    using contract::contract;

  public:

    ACTION addproposal (const name proposer,
                        const name required_approver,
                        const string proposal_name,
                        const string proposal_text,
                        const string github_link);

    ACTION approve (const uint64_t proposal_id);

    ACTION delproposal (const uint64_t proposal_id);
    
  private:

    const string STATUS_OPEN = "OPEN";
    const string STATUS_APPROVED = "APPROVED";

    // TABLE Config
    // {
    //     asset       gft_eos_rate;
    //     float       gyfter_payback_rate;      
    // };

    // typedef singleton<"configs"_n, Config> config_table;
    // typedef eosio::multi_index<"configs"_n, Config> config_table_placeholder;
    
    TABLE proposal
    {
        uint64_t    proposal_id;
        name        proposer;
        name        required_approver;
        string      proposal_name;
        string      proposal_text;
        string      github_link;
        string      status;
        uint32_t    created_date;
        uint32_t    approval_date;        
        uint64_t    primary_key() const { return proposal_id; }
    };

    typedef eosio::multi_index<"proposals"_n, proposal> proposal_table;

};
