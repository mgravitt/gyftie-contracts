#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <string>
#include <algorithm> // std::find
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>
//#include <eosio/chain/authority.hpp>
#include <math.h>

#include "includes/abieos_numeric.hpp"

using std::string;
using std::vector;
using std::tie;
using namespace eosio;

CONTRACT gyftietoken : public contract
{
    using contract::contract;

  public:

    // TEMPORARY actions needed for deployments
    ACTION copyprofs (const name account);
    //ACTION fixstakes (const name account);
    ACTION fixstake (const name account);
    // ACTION sigupdate ();
    ACTION copyback (const name account);
    ACTION removeprofs (const name account);
    //ACTION upperm();

    ACTION chgthrottle(const uint32_t throttle);

    ACTION setconfig(const name token_gen, const name gftorderbook, const name gyftie_foundation);

    ACTION delconfig();

    ACTION setstate (const uint32_t account_count,
                                const uint32_t prior_step_user_count,
                                const uint32_t pol_user_count_decayx100,  // 2%
                                const uint32_t pol_step_increasex100); 

    ACTION addrating(const name rater, const name ratee, const uint8_t rating);

    ACTION create();

    ACTION issue(const name to, const asset quantity, const string memo);
    ACTION issuetostake (const name to, const asset quantity, const string memo);

    ACTION transfer(const name from, const name to, const asset quantity, const string memo);
    ACTION xfertostake(const name from, const name to, const asset quantity, const string memo);
    
    ACTION requnstake (const name user, const asset quantity);
    ACTION unstaked (const name user, const asset quantity); 
    ACTION unstaked2 (const name user, const asset quantity); 
    ACTION stake (const name account, const asset quantity);

    ACTION calcgyft(name from, name to);

    ACTION gyft2 (const name from, 
                    const name to, 
                    const string idhash,
                    const string relationship,
                    const string id_expiration);

    ACTION gyft(const name from, const name to, const string idhash, const string relationship);

    ACTION ungyft(const name account);

    ACTION propose(const name proposer, const string notes);

    ACTION votefor(const name voter, const uint64_t proposal_id);

    ACTION voteagainst(const name voter, const uint64_t proposal_id);

    ACTION removeprop(const uint64_t proposal_id);

    ACTION nchallenge (const name challenger_account, const name challenged_account, const string notes);

    ACTION validate (const name validator, const name account, const string idhash, const string id_expiration);

    ACTION dchallenge (const name challenged_account);

    ACTION addcnote (const name scribe, const name challenged_account, const string note);

    ACTION addlock (const name account_to_lock, const string note);
    ACTION addlockchain (const name account_to_lock, const string note );
    ACTION addlocknote (const name account_to_lock, const string note);

    ACTION unlockchain (const name account_to_unlock, const string note );
    ACTION unlock (const name account_to_unlock, const string note);
    ACTION addsig (const name new_signatory);
    ACTION remsig (const name existing_signatory);

    ACTION pause();

    ACTION unpause();

  private:
    const string GYFTIE_SYM_STR = "GFT";
    const uint8_t GYFTIE_PRECISION = 8;
    const string symbol_name = "EOS";
    const symbol network_symbol = symbol(symbol_name, 4);

    const uint8_t PAUSED = 1;
    const uint8_t UNPAUSED = 0;

    const uint8_t NEW_CHALLENGE=1;
    const uint8_t VALIDATED=2;

    const uint64_t  SCALER = 100000000;

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

    TABLE State 
    {
        uint32_t    user_count = 0;
        uint32_t    prior_step_user_count;   

        uint64_t    pol_scaled_user_count_decay;
        uint64_t    pol_scaled_step_increase;

        uint64_t    scaled_user_count_factor=100000000;
    };

    typedef singleton<"states"_n, State> state_table;
    typedef eosio::multi_index<"states"_n, State> state_table_placeholder;

    // TABLE Throttle 
    // {
    //     uint64_t    throttle_id;
    //     uint16_t    allowed_gyfts = 100;
    //     uint32_t    per_seconds = 100;
    // };

    TABLE Throttle 
    {
        uint32_t throttle = 0;
    };

    typedef singleton<"throttles"_n, Throttle> throttle_table;
    typedef eosio::multi_index<"throttles"_n, Throttle> throttle_table_placeholder;

    TABLE SenderID
    {
        uint64_t    last_sender_id;
    };
    typedef singleton<"senderids"_n, SenderID> senderid_table;
    typedef eosio::multi_index<"senderids"_n, SenderID> senderid_table_placeholder;
   

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

    TABLE lock
    {
        name            account;
        vector<string>  lock_notes;
        uint64_t    primary_key() const { return account.value; }
    };
    typedef eosio::multi_index<"locks"_n, lock> lock_table;

    TABLE signatory
    {
        name        account;
        uint64_t    primary_key() const { return account.value; }
    };
    typedef eosio::multi_index<"signatories"_n, signatory> signatory_table;

    TABLE account
    {
        asset balance;
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
        asset       gft_balance;
        asset       staked_balance;

        // DEPLOY
        asset       unstaking_balance; // = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
        uint64_t    primary_key() const { return account.value; }
    };
    typedef eosio::multi_index<"profiles"_n, profile> profile_table;

    TABLE tprofile
    {
        name        account;
        uint32_t    rating_sum;
        uint16_t    rating_count;
        string      idhash;
        string      id_expiration;
        asset       gft_balance;
        asset       staked_balance;
        uint64_t    primary_key() const { return account.value; }
    };
    typedef eosio::multi_index<"tprofiles"_n, tprofile> tprofile_table;

    TABLE challenge 
    {
        name            challenged_account;
        name            challenger_account;
        vector<string>  challenge_notes;
        asset           challenge_stake;
        uint32_t        challenged_time;
        uint64_t        primary_key () const { return challenged_account.value; }
        uint64_t        by_challenger() const { return challenger_account.value; }
    };
    typedef eosio::multi_index<"challenges"_n, challenge,
        indexed_by<"bychallenger"_n,
            const_mem_fun<challenge, uint64_t, &challenge::by_challenger>>
    > challenge_table;

    TABLE availrating
    {
        name        ratee;
        uint32_t    rate_deadline;
        uint64_t    primary_key() const { return ratee.value; }
    };
    typedef eosio::multi_index<"availratings"_n, availrating> availrating_table;

    TABLE gyftevent
    {
        uint64_t    gyft_id;
        name        gyfter;
        name        gyftee;
        asset       gyfter_issue;
        asset       gyftee_issue;
        string      relationship;
        string      notes;
        uint32_t    gyft_date;
        uint16_t    likes;
        uint64_t    primary_key() const { return gyft_id; }
        uint64_t    by_gyfter() const { return gyfter.value; }
        uint64_t    by_gyftee() const { return gyftee.value; }
        uint64_t    by_gyftdate() const { return gyft_date; }
    };

    typedef eosio::multi_index<"gyfts"_n, gyftevent,
        indexed_by<"bygyfter"_n,
            const_mem_fun<gyftevent, uint64_t, &gyftevent::by_gyfter>>,
        indexed_by<"bygyftee"_n,
            const_mem_fun<gyftevent, uint64_t, &gyftevent::by_gyftee>>,
        indexed_by<"bygyftdate"_n, 
            const_mem_fun<gyftevent, uint64_t, &gyftevent::by_gyftdate>>
        >
    gyft_table;

    // TABLE tgyftevent
    // {
    //     uint64_t    gyft_id;
    //     name        gyfter;
    //     name        gyftee;
    //     asset       gyfter_issue;
    //     asset       gyftee_issue;
    //     string      relationship;
    //     string      notes;
    //     uint32_t    gyft_date;
    //     uint16_t    likes;
    //     uint64_t    primary_key() const { return gyft_id; }
    //     uint64_t    by_gyfter() const { return gyfter.value; }
    //     uint64_t    by_gyftee() const { return gyftee.value; }
    // };

    // typedef eosio::multi_index<"tgyfts"_n, tgyftevent,
    //                            indexed_by<"bygyfter"_n,
    //                                const_mem_fun<tgyftevent, uint64_t, &tgyftevent::by_gyfter>>,
    //                            indexed_by<"bygyftee"_n,
    //                                const_mem_fun<tgyftevent, uint64_t, &tgyftevent::by_gyftee>>
    //                            >
    //     tgyft_table;

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

    // void remove_profile (const name account)
    // {
    //     profile_table p_t (get_self(), get_self().value);
    //     auto p_itr = p_t.find (account.value);
    //     eosio::check (p_itr != p_t.end(), "Profile to remove cannot be found.");
    //     eosio::check (p_itr->gft_balance.amount <= 0, "Profile has a positive GFT balance. Cannot remove.");
    //     eosio::check (p_itr->staked_balance.amount <= 0, "Profile has a positive staked GFT balance. Cannot remove.");

    //     p_t.erase (p_itr);

    //     symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

    //     accounts a_t (get_self(), account.value);
    //     auto a_itr = a_t.find(sym.code().raw());
    //     if (a_itr != a_t.end()) {
    //         eosio::check (a_itr->balance.amount <= 0, "Accounts table does not have a zero balance for account. Cannot remove.");
    //         a_t.erase (a_itr);
    //     }
    // }

    uint64_t get_next_sender_id()
    {
        senderid_table sid (get_self(), get_self().value);
        SenderID s = sid.get_or_create(get_self(), SenderID());
        uint64_t return_senderid = s.last_sender_id;
        return_senderid++;
        s.last_sender_id = return_senderid;
        sid.set (s, get_self());
        return return_senderid;
    }

    void xfer_account (const name old_account, const name new_account)
    {
        profile_table p_t (get_self(), get_self().value);
        auto old_profile_itr = p_t.find (old_account.value);
        eosio::check (old_profile_itr != p_t.end(), "Profile to remove cannot be found.");
        eosio::check (old_profile_itr->staked_balance.amount <= 0, "Profile has staked GFT. Cannot transfer.");
        
        auto new_profile_itr = p_t.find (new_account.value);
        eosio::check (new_profile_itr != p_t.end(), "Profile of new account cannot be found.");
        
        config_table config(get_self(), get_self().value);
        auto c = config.get();

        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        balance_table b_t(c.gftorderbook, old_account.value);
        auto b_itr = b_t.find(sym.code().raw());
        if (b_itr != b_t.end() && b_itr->token_contract == get_self())
        {
            eosio::check (true, "Profile to remove has open orders or a committed balance. Delete orders and/or withdraw funds.");
        }

        string s { "Transfer from account via Gyftie signatories"};
        transfer (old_account, new_account, old_profile_itr->gft_balance, s);

        p_t.erase (old_profile_itr);
    }

    void require_any_signatory () 
    {
        if (has_auth (get_self())) {
            return;
        }

        bool signed_by_signatory = false;
        signatory_table s_t (get_self(), get_self().value);
        auto s_itr = s_t.begin();

        while (!signed_by_signatory && s_itr != s_t.end()) {
            signed_by_signatory = has_auth (s_itr->account);
            s_itr++;
        }

        eosio::check (signed_by_signatory, "Transaction requires the approval of a signatory.");
    }

    void throttle_gyfts () 
    {
        throttle_table t_t (get_self(), get_self().value);
        Throttle t = t_t.get_or_create(get_self(), Throttle());

        state_table state (get_self(), get_self().value);
        auto s = state.get();
        
        if (t.throttle < s.user_count && t.throttle > 0) {
            gyft_table g_t (get_self(), get_self().value);
            auto gyftdate_index = g_t.get_index<"bygyftdate"_n>();
            auto gyftdate_itr = gyftdate_index.rbegin();

            for (int i=0; i< t.throttle; i++) {
                gyftdate_itr++;
            }

            uint32_t throttled_gyfts_ago = gyftdate_itr->gyft_date;
            uint32_t throttled_time_period = 60 * 60 * 24; // 24 hours
            eosio::check (  throttled_gyfts_ago < 
                                current_block_time().to_time_point().sec_since_epoch() - 
                                throttled_time_period, 
                            "Gyfts are throttled. Please wait a few hours and try again.");
        }
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
            g.gyft_date = current_block_time().to_time_point().sec_since_epoch();
        });
    }

    // void stake (name account, asset quantity) 
    // {
    //     //sub_balance (account, quantity);

    //     profile_table p_t (get_self(), get_self().value);
    //     auto p_itr = p_t.find (account.value);
    //     eosio::check (p_itr != p_t.end(), "Account profile not found.");
    //     eosio::check (p_itr->gft_balance >= quantity, "Liquid balance is less than quanitity staking.");

    //     p_t.modify (p_itr, get_self(), [&](auto &p) {
    //         p.gft_balance -= quantity;
    //         p.staked_balance += quantity;
    //     });
    // }

    void unstake (name account, asset quantity) 
    {
        //add_balance (account, quantity, get_self());

        profile_table p_t (get_self(), get_self().value);
        auto p_itr = p_t.find (account.value);
        eosio::check (p_itr != p_t.end(), "Account profile not found.");
        
        // DEPLOY
        eosio::check (p_itr->unstaking_balance >= quantity, "Unstaking balance is less than quantity unstaking.");
                
        p_t.modify (p_itr, get_self(), [&](auto &p) {
            p.gft_balance += quantity;
            // DEPLOY
            p.unstaking_balance -= quantity;
            //p.staked_balance -= quantity;
        });
    }

    asset adjust_asset(asset original_asset, float adjustment)
    {
        return asset{static_cast<int64_t>(original_asset.amount * adjustment), original_asset.symbol};
    }

    void permit_account (name account)
    {
        eosio::check ( is_gyftie_account (account), "Account is not a GFT token holder.");

        lock_table l_t (get_self(), get_self().value);
        auto l_itr = l_t.find(account.value);
        eosio::check (l_itr == l_t.end(), "Account has been locked out of Gyftie activities."); 

        challenge_table c_t (get_self(), get_self().value);
        auto c_itr = c_t.find(account.value);

        if (c_itr != c_t.end()) {
            eosio::check (c_itr->challenged_time <= current_block_time().to_time_point().sec_since_epoch() + 60 * 60 * 24 * 7, "Account is locked until it is re-validated.");
        }       
    }

    void permit_validator (name validator, name challenged_account) 
    {
        gyft_table g_t (get_self(), get_self().value);
        auto gyfter_index = g_t.get_index<"bygyfter"_n>();
        auto gyfter_itr = gyfter_index.find(validator.value);

        while (gyfter_itr != gyfter_index.end() && gyfter_itr->gyfter == validator) {
            eosio::check (gyfter_itr->gyftee != challenged_account, "Validator cannot validate an account they gyfted.");
            gyfter_itr++;
        }

        auto gyftee_index = g_t.get_index<"bygyftee"_n>();
        auto gyftee_itr = gyftee_index.find(validator.value);

        while (gyftee_itr != gyftee_index.end() && gyftee_itr->gyftee == validator) {
            eosio::check (gyftee_itr->gyfter != challenged_account, "Validator cannot validate their gyfter.");
            gyftee_itr++;
        }
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

    void insert_profile(const name account, 
                        const string idhash,
                        const string id_expiration)
    {
        profile_table p_t(get_self(), get_self().value);
        auto p_itr = p_t.find(account.value);
        eosio::check(p_itr == p_t.end(), "Account already has a profile.");

        p_t.emplace(get_self(), [&](auto &p) {
            p.account = account;
            p.rating_count = 0;
            p.rating_sum = 0;
            p.idhash = idhash;
            p.id_expiration = id_expiration;
            p.gft_balance = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
            // DEPLOY
            p.unstaking_balance = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
            p.staked_balance = asset {0, symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION}};
        });
    }

    void increment_account_count()
    {
        state_table state (get_self(), get_self().value);
        auto s = state.get();
        s.user_count++;
        state.set (s, get_self());
    }

    void burn(name account, asset quantity)
    {
        stats statstable(get_self(), quantity.symbol.code().raw());
        auto existing = statstable.find(quantity.symbol.code().raw());
        eosio::check(existing != statstable.end(), "token with symbol does not exist");
        const auto &st = *existing;

        eosio::check(quantity.is_valid(), "invalid quantity");
        eosio::check(quantity.amount > 0, "must issue positive quantity");
        eosio::check(quantity.symbol == st.symbol, "symbol precision mismatch");

        statstable.modify(st, eosio::same_payer, [&](auto &s) {
            s.supply -= quantity;
        });

        sub_balance(account, quantity);
    }

    asset getgftbalance(name account)
    {
        symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        asset gft_balance = asset{0, sym};

        profile_table p_t (get_self(), get_self().value);
        auto p_itr = p_t.find (account.value);
        if (p_itr != p_t.end()) {
            gft_balance += p_itr->gft_balance;
            gft_balance += p_itr->staked_balance;
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
        eosio::check(g_itr != g_t.end(), "No gyft request matching this recipient.");
        eosio::check (g_itr->gyfter == gyfter, "Gyft request does not match this gyfter.");

        // New account resources
        asset ram = asset(1700, network_symbol); // 0.3
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
        eosio::check (reimbursement <= get_gyfter_reward(gyfter), "Cost of creating account is not covered by Gyft reward.");

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

        action (
            permission_level{get_self(), "owner"_n},
            "gyftmultisig"_n, "remrequest"_n,
            std::make_tuple(recipient))
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
        state_table state = state_table (get_self(), get_self().value);
        auto s = state.get();
        
        float increase_since_last_step = (float) (s.user_count - s.prior_step_user_count) / (float) s.prior_step_user_count;

        if (increase_since_last_step >= (float) s.pol_scaled_step_increase / SCALER) {
            float decay_percentage = (float) 1.00000000  - ( (float) s.pol_scaled_user_count_decay / (float) SCALER);
            s.scaled_user_count_factor *= (float) (decay_percentage);
            s.prior_step_user_count = s.user_count;
            state.set (s, get_self());
        }

        return (float) s.scaled_user_count_factor / (float) SCALER;
    }

    asset get_one_gft()
    {
        symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
        return asset { static_cast<int64_t>(pow(10, GYFTIE_PRECISION)), gft_symbol};
    }

    asset get_recipient_reward ()
    {
        return adjust_asset (get_one_gft(), get_usercount_factor());
    }

    asset get_gyfter_reward (name gyfter)
    {
        asset one_gyftie_token = get_one_gft();
        asset gyfter_gft_balance = getgftbalance (gyfter);

        asset gyft_benefit_amount = one_gyftie_token;
        float usercount_factor = get_usercount_factor();

        // print (" Gyfter GFT Balance: ", gyfter_gft_balance, "\n");
        // print (" user count factor: ", std::to_string(usercount_factor), "\n");

        asset adjusted_one_gft = adjust_asset (one_gyftie_token, usercount_factor);
        // print (" Adjusted 1 GFT: ", adjusted_one_gft, "\n");

        if (gyfter_gft_balance  <= (adjusted_one_gft * 3)) {
            gyft_benefit_amount = adjusted_one_gft * 3;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 10 )) {
            gyft_benefit_amount = adjusted_one_gft * 4;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 20)) {
            gyft_benefit_amount = adjusted_one_gft * 5;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 50)) {
            gyft_benefit_amount = adjusted_one_gft * 6;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 100)) {
            gyft_benefit_amount = adjusted_one_gft * 7;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 200)) {
            gyft_benefit_amount = adjusted_one_gft * 8;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 500)) {
            gyft_benefit_amount = adjusted_one_gft * 9;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 1000)) {
            gyft_benefit_amount = adjusted_one_gft * 10;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 2000)) {
            gyft_benefit_amount = adjusted_one_gft * 11;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 5000)) {
            gyft_benefit_amount = adjusted_one_gft * 12;
        } else if (gyfter_gft_balance  <= (adjusted_one_gft * 10000)) {
            gyft_benefit_amount = adjusted_one_gft * 15;
        } else {
            gyft_benefit_amount = adjusted_one_gft * 20;
        }

        // print (" Benefit amount: ", gyft_benefit_amount, "\n" );
        return gyft_benefit_amount;
    }

    void defer_unstake (const name user, const asset quantity, const uint32_t delay) 
    {
        eosio::transaction out{};
        // DEPLOY
        out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
            get_self(), "unstaked2"_n, 
            std::make_tuple(user, quantity));
        out.delay_sec = delay;
        out.send(current_block_time().to_time_point().sec_since_epoch() + user.value + delay, get_self());    
    }

    void sub_balance(const name owner, const asset value)
    {
        accounts from_acnts(_self, owner.value);

        const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
        eosio::check(from.balance.amount >= value.amount, "overdrawn balance");

        auto sym_name = value.symbol.code().raw();
        stats statstable(_self, sym_name);
        auto existing = statstable.find(sym_name);
        eosio::check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
        const auto &st = *existing;

        from_acnts.modify(from, get_self(), [&](auto &a) {
            a.balance -= value;
        });

        profile_table p_t (get_self(), get_self().value);
        auto p_itr = p_t.find (owner.value);
        eosio::check (p_itr != p_t.end(), "Account not found.");
        eosio::check (p_itr->gft_balance >= value, "overdrawn balance - GFT is staked");

        p_t.modify (p_itr, get_self(), [&](auto &p) {
            p.gft_balance -= value;
        });
    }

    void add_balance(const name owner, const asset value, const name ram_payer)
    {
        accounts to_acnts(_self, owner.value);
        auto to = to_acnts.find(value.symbol.code().raw());
        if (to == to_acnts.end())
        {
            auto sym_name = value.symbol.code().raw();
            stats statstable(_self, sym_name);
            auto existing = statstable.find(sym_name);
            eosio::check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
            const auto &st = *existing;

            if (owner != st.issuer) {  // do not increment account count for issuer
                increment_account_count ();
            }
            to_acnts.emplace(ram_payer, [&](auto &a) {
                a.balance = value;
            });
        }
        else
        {
            to_acnts.modify(to, eosio::same_payer, [&](auto &a) {
                a.balance += value;
            });
        }
        profile_table p_t (get_self(), get_self().value);
        auto p_itr = p_t.find (owner.value);
        eosio::check (p_itr != p_t.end(), "Account not found.");

        p_t.modify (p_itr, get_self(), [&](auto &p) {
            p.gft_balance += value;
        });
    }


};
