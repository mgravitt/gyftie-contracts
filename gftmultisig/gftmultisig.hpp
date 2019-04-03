
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
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

    ACTION clrproposals ();

    ACTION clnrequests ();

    ACTION addrequest (const name gyfter,
                        const name recipient,
                        const string owner_public_key,
                        const string active_public_key) ;

    ACTION delrequest (const name recipient);

    ACTION remrequest (const name recipient);
    
  private:

    const string STATUS_OPEN = "OPEN";
    const string STATUS_APPROVED = "APPROVED";

    TABLE gyftrequest
    {
      name        recipient;
      name        gyfter;
      string      owner_public_key;
      string      active_public_key;
      uint32_t    requested_date;
      uint64_t    primary_key() const { return  recipient.value; }
      uint64_t    by_gyfter () const { return gyfter.value; }
    };

    typedef eosio::multi_index<"gyftrequests"_n, gyftrequest,
      indexed_by<"bygyfter"_n,
        const_mem_fun<gyftrequest, uint64_t, &gyftrequest::by_gyfter>>
    > gyftrequest_table;
    
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
