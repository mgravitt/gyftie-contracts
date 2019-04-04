#include "gyftietoken.hpp"


ACTION gyftietoken::chgthrottle (const uint32_t throttle)
{
    require_auth ("zombiejigsaw"_n);
    throttle_table t_t (get_self(), get_self().value);
    Throttle t;
    t.throttle = throttle;
    t_t.set (t, get_self());
}

// ACTION gyftietoken::copygyfts1 ()
// {
//     require_auth ("zombiejigsaw"_n);
//     gyft_table g_t (get_self(), get_self().value);
//     auto g_itr = g_t.begin();

//     tgyft_table tg_t (get_self(), get_self().value);
    
//     while (g_itr != g_t.end()) {
//         tg_t.emplace (get_self(), [&] (auto& tg) {
//             tg.gyft_id = g_itr->gyft_id;
//             tg.gyfter = g_itr->gyfter;
//             tg.gyftee = g_itr->gyftee;
//             tg.gyfter_issue = g_itr->gyfter_issue;
//             tg.gyftee_issue = g_itr->gyftee_issue;
//             tg.relationship = g_itr->relationship;
//             tg.notes = g_itr->notes;
//             tg.gyft_date = g_itr->gyft_date;
//             tg.likes = g_itr->likes;            
//         });
//        g_itr++;
//     }

//     g_itr = g_t.begin();
//     while (g_itr != g_t.end()) {
//         g_itr = g_t.erase(g_itr);
//     }
// }

// ACTION gyftietoken::deloriggyfts ()
// {
//     require_auth ("zombiejigsaw"_n);
//     gyft_table g_t (get_self(), get_self().value);

//     auto g_itr = g_t.begin();
//     while (g_itr != g_t.end()) {
//         g_itr = g_t.erase(g_itr);
//     }
// }

// ACTION gyftietoken::upperm () 
// {
//     permission_level df3 = permission_level{"danielflora3"_n, "active"_n};
//     permission_level df2 = permission_level{"danielflora2"_n, "active"_n};
//     permission_level zj = permission_level{"zombiejigsaw"_n, "active"_n};

//     permission_level_weight df3_weight = permission_level_weight {df3, 1};
//     permission_level_weight df2_weight = permission_level_weight {df2, 1};
//     permission_level_weight zj_weight = permission_level_weight {zj, 2};

//     vector<permission_level_weight> accounts;
    
//     accounts.push_back (df2_weight);
//     accounts.push_back (df3_weight);
//     accounts.push_back (zj_weight);
    
//     authority auth = authority{3, {}, accounts, {}};

//     auto update_auth_payload = std::make_tuple(get_self(), "active"_n, "owner"_n, auth);

//     action(
//         permission_level{get_self(), "owner"_n},
//         "eosio"_n,
//         "updateauth"_n,
//         update_auth_payload)
//     .send();
// }

// ACTION gyftietoken::copybal1 ()
// {
//     //require_auth ("zombiejigsaw"_n);

//     config_table config (get_self(), get_self().value);
//     Config c = config.get();

//     profile_table p_t (get_self(), get_self().value);
//     symbol gft_symbol = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

//     accounts a_t (get_self(), c.gftorderbook.value);
//     auto a_itr = a_t.find (gft_symbol.code().raw());

//     tprofile_table tp_t (get_self(), get_self().value); 
//     tp_t.emplace (get_self(), [&](auto &t) {
//         t.account = c.gftorderbook;
//         t.gft_balance = a_itr->balance;
//         t.idhash = "GFT Order Book Account";
//     });

//     a_t = accounts (get_self(), c.gyftie_foundation.value);
//     a_itr = a_t.find (gft_symbol.code().raw());

//     tp_t.emplace (get_self(), [&](auto &t) {
//         t.account = c.gyftie_foundation;
//         t.gft_balance = a_itr->balance;
//         t.idhash = "Gyftie Limited Account";
//     });

//     auto p_itr = p_t.begin();
//     while (p_itr != p_t.end()) {
//         if (p_itr->account == c.gftorderbook || p_itr->account == c.gyftie_foundation) {

//         } else {
//             a_t = accounts (get_self(), p_itr->account.value);
//             a_itr = a_t.find (gft_symbol.code().raw());

//             tp_t.emplace (get_self(), [&](auto& t) {
//                 t.account = p_itr->account;
//                 t.rating_sum = p_itr->rating_sum;
//                 t.rating_count = p_itr->rating_count;
//                 t.idhash = p_itr->idhash;
//                 t.id_expiration = p_itr->id_expiration;
//                 t.gft_balance = a_itr->balance;
//             });
//         }
        
//         p_itr++;
//     }
// }

// ACTION gyftietoken::addlockperm ()
// {
    
//     permission_level df3 = permission_level{"danielflora3"_n, "active"_n};
//     permission_level df2 = permission_level{"danielflora2"_n, "active"_n};
//     permission_level zj = permission_level{"zombiejigsaw"_n, "active"_n};

//     permission_level_weight df3_weight = permission_level_weight {df3, 1};
//     permission_level_weight df2_weight = permission_level_weight {df2, 1};
//     permission_level_weight zj_weight = permission_level_weight {zj, 1};

//     vector<permission_level_weight> accounts;
    
//     accounts.push_back (df2_weight);
//     accounts.push_back (df3_weight);
//     accounts.push_back (zj_weight);
    
//     authority auth = authority{1, {}, accounts, {}};

//     auto update_auth_payload = std::make_tuple(get_self(), "suspectlock"_n, "active"_n, auth);

//     action(
//         permission_level{get_self(), "owner"_n},
//         "eosio"_n,
//         "updateauth"_n,
//         update_auth_payload)
//     .send();

    // auto link_auth_payload = std::make_tuple(get_self(), get_self(), "addlock"_n, "suspectlock"_n);

    // action(
    //     permission_level{get_self(), "owner"_n},
    //     "eosio"_n,
    //     "linkauth"_n,
    //     link_auth_payload)
    // .send();

    // auto link_auth_payload2 = std::make_tuple(get_self(), get_self(), "unlock"_n, "suspectlock"_n);

    // action(
    //     permission_level{get_self(), "owner"_n},
    //     "eosio"_n,
    //     "linkauth"_n,
    //     link_auth_payload2)
    // .send();
// }

ACTION gyftietoken::addlock (const name account_to_lock) 
{
    require_any_signatory();

    lock_table l_t (get_self(), get_self().value);
    auto l_itr = l_t.find (account_to_lock.value);
    eosio::check (l_itr == l_t.end(), "Account is already locked.");

    l_t.emplace (get_self(), [&](auto &l) {
        l.account = account_to_lock;
    });
}

ACTION gyftietoken::unlock (const name account_to_unlock) 
{
    require_any_signatory();

    lock_table l_t (get_self(), get_self().value);
    auto l_itr = l_t.find (account_to_unlock.value);
    eosio::check (l_itr != l_t.end(), "Account is not locked.");

    l_t.erase (l_itr);
}

ACTION gyftietoken::addsig (const name new_signatory) 
{
    require_auth (get_self());

    signatory_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.find (new_signatory.value);
    eosio::check (s_itr == s_t.end(), "Account is already a signatory.");

    s_t.emplace (get_self(), [&](auto &s) {
        s.account = new_signatory;
    });
}

ACTION gyftietoken::remsig (const name existing_signatory) 
{
    require_auth (get_self());

    signatory_table s_t (get_self(), get_self().value);
    auto s_itr = s_t.find (existing_signatory.value);
    eosio::check (s_itr != s_t.end(), "Signatory not found.");

    s_t.erase (s_itr);
}

ACTION gyftietoken::dchallenge (const name challenged_account) 
{
    require_auth (get_self());
    challenge_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.find (challenged_account.value);
    eosio::check (c_itr != c_t.end(), "Active challenge for account does not exist.");

    c_t.erase (c_itr);
}

ACTION gyftietoken::setconfig (const name token_gen,
                                const name gftorderbook,
                                const name gyftie_foundation)
{
    require_auth (get_self());
        
    config_table config (get_self(), get_self().value);
    Config c;
    c.token_gen = token_gen;
    c.gftorderbook = gftorderbook;
    c.gyftie_foundation = gyftie_foundation;
    c.paused = PAUSED;
    config.set (c, get_self());

    profile_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (gftorderbook.value);
    if (p_itr == p_t.end()) {
        insert_profile (gftorderbook, "GFT Order Book Account", "NO EXPIRATION");
    }

    p_itr = p_t.find (gyftie_foundation.value);
    if (p_itr == p_t.end()) {
        insert_profile (gyftie_foundation, "Gyftie Limited Account", "NO EXPIRATION");
    }
}

ACTION gyftietoken::pause () 
{
    require_auth (get_self());
    config_table config (get_self(), get_self().value);
    Config c = config.get();
    c.paused = PAUSED;
    config.set (c, get_self());
}

ACTION gyftietoken::unpause () 
{
    require_auth (get_self());
    config_table config (get_self(), get_self().value);
    Config c = config.get();
    c.paused = UNPAUSED;
    config.set (c, get_self());
}

ACTION gyftietoken::setstate (const uint32_t account_count,
                                const uint32_t prior_step_user_count,
                                const uint32_t pol_user_count_decayx100,  // 2%
                                const uint32_t pol_step_increasex100)  // 1%
{
    require_auth (get_self());
    
    state_table state (get_self(), get_self().value);
    State s;
    s.user_count = account_count;
    s.prior_step_user_count = prior_step_user_count;
    s.pol_scaled_user_count_decay = pol_user_count_decayx100 * SCALER / 100;
    s.pol_scaled_step_increase = pol_step_increasex100 * SCALER / 100;
    state.set(s, get_self());
}

ACTION gyftietoken::delconfig () 
{
    require_auth (get_self());
    config_table config (get_self(), get_self().value);
    config.remove();
}

ACTION gyftietoken::nchallenge (const name challenger_account, const name challenged_account, const string note)
{
    require_auth (challenger_account);
    permit_account(challenger_account);
    eosio::check (challenger_account != challenged_account, "Account cannot challenge itself.");
    eosio::check (is_tokenholder (challenger_account), "Challenger is not a GFT token holder.");
    eosio::check (is_tokenholder (challenged_account), "Challenged account is not a GFT token holder.");
    eosio::check (! is_paused(), "Contract is paused." );

    asset challenge_stake = adjust_asset (getgftbalance (challenged_account), (float) 0.10000000);
    stake (challenger_account, challenge_stake);

    challenge_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.find(challenged_account.value);
    eosio::check (c_itr == c_t.end(), "Account has already been challenged.");

    c_t.emplace (get_self(), [&](auto &c) {
        c.challenged_account = challenged_account;
        c.challenger_account = challenger_account;
        c.challenged_time = current_block_time().to_time_point().sec_since_epoch();
        c.challenge_notes.push_back (note);
        c.challenge_stake = challenge_stake;
    });
}

ACTION gyftietoken::addcnote (const name scribe, const name challenged_account, const string note)
{
    require_auth (scribe);
    permit_account (scribe);
    eosio::check (! is_paused(), "Contract is paused." );

    challenge_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.find (challenged_account.value);
    eosio::check (c_itr != c_t.end(), "Account does not have an active challenge.");

    c_t.modify (c_itr, get_self(), [&] (auto &c) {
        c.challenge_notes.push_back (note);
    });
}

ACTION gyftietoken::validate (const name validator, const name account, const string idhash, const string id_expiration)
{
    permit_account(validator);
    permit_validator(validator, account);
    require_auth (validator);
    eosio::check (is_tokenholder (validator), "Validator is not a GFT token holder.");
    eosio::check (! is_paused(), "Contract is paused." );

    profile_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (account.value);
    eosio::check (p_itr != p_t.end(), "Account not found.");

    eosio::check (p_itr->idhash.compare(idhash) == 0, "ID hash provided does not match records. Account not validated.");
      
    challenge_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.find (account.value);
    eosio::check (c_itr != c_t.end(), "Account does not have an active challenge.");

    unstake (c_itr->challenger_account, c_itr->challenge_stake);

    asset validator_amount = adjust_asset (c_itr->challenge_stake, (float) 0.200000000);
    asset challenged_amount = adjust_asset (c_itr->challenge_stake, (float) 0.400000000);
    asset redistribution_amount = c_itr->challenge_stake - validator_amount - challenged_amount;

    string to_validator_memo = string { "GFT-reward to the Validator. See 'How Gyftie Works' document - ask us for link." };
    string to_challenged_memo = string { "GFT-reward to the Challenged-then-Validated Account. See 'How Gyftie Works' document - ask us for link." };   
    string redistribution_memo = string { "Seized asset redistribution. See 'How Gyftie Works' document - ask us for link." };
    
    action (
        permission_level{get_self(), "owner"_n},
        get_self(), "transfer"_n,
        std::make_tuple(c_itr->challenger_account, validator, validator_amount, to_validator_memo))
    .send();

    action (
        permission_level{get_self(), "owner"_n},
        get_self(), "transfer"_n,
        std::make_tuple(c_itr->challenger_account, c_itr->challenged_account, challenged_amount, to_challenged_memo))
    .send();

    action (
        permission_level{get_self(), "owner"_n},
        get_self(), "transfer"_n,
        std::make_tuple(c_itr->challenger_account, get_self(), redistribution_amount, redistribution_memo))
    .send();

    c_t.erase (c_itr);    
}

ACTION gyftietoken::addrating (const name rater, const name ratee, const uint8_t rating)
{
    permit_account(rater);
    availrating_table a_t (get_self(), rater.value);
    auto a_itr = a_t.find (ratee.value);
    eosio::check (a_itr != a_t.end(), "Account is not available for rating.");
    eosio::check (has_auth (rater), "Only eligible rater can add a rating.");
    eosio::check (a_itr->rate_deadline >= current_block_time().to_time_point().sec_since_epoch(), "Deadline to rate account has passed.");

    profile_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (ratee.value);
    eosio::check (p_itr != p_t.end(), "Account to rate is not found.");
    
    p_t.modify (p_itr, get_self(), [&](auto &p) {
        p.rating_count++;
        p.rating_sum += rating;
    });
    
    a_t.erase (a_itr);
}

ACTION gyftietoken::removeprop (const uint64_t proposal_id) 
{
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find(proposal_id);
    eosio::check (p_itr != p_t.end(), "Proposal ID is not found.");

    eosio::check (  has_auth (get_self()) || 
                    has_auth (p_itr->proposer) ||
                    has_auth ("danielflora3"_n) ||
                    has_auth ("zombiejigsaw"_n),    
        "Permission denied - only Gyftie signatory or proposer can remove a proposal.");

    p_t.erase (p_itr);
}

ACTION gyftietoken::propose (const name proposer,
                                const string notes) 
{
    permit_account(proposer);
    eosio::check (! is_paused(), "Contract is paused." );
    require_auth (proposer);
    eosio::check (is_tokenholder (proposer), "Proposer is not a GFT token holder.");

    proposal_table p_t (get_self(), get_self().value);

    p_t.emplace (proposer, [&](auto &p) {
        p.proposal_id   = p_t.available_primary_key();
        p.created_date  = current_block_time().to_time_point().sec_since_epoch();
        p.proposer      = proposer;
      //  p.new_token_gen = token_gen;
        p.notes         = notes;
        p.votes_for     = 0;
        p.votes_against = 0;
        p.expiration_date   = current_block_time().to_time_point().sec_since_epoch() + (60 * 60 * 24 * 30);  // 30 days
    });
}

ACTION gyftietoken::votefor (const name voter,
                            const uint64_t proposal_id) 
{
    eosio::check (! is_paused(), "Contract is paused." );

    require_auth (voter);
    eosio::check (is_tokenholder (voter), "Voter is not a GFT token holder.");
    
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (proposal_id);
    eosio::check (current_block_time().to_time_point().sec_since_epoch() <= p_itr->expiration_date, "Proposal has expired.");

    auto voter_for_itr = std::find (p_itr->voters_for.begin(), p_itr->voters_for.end(), voter);
    eosio::check (voter_for_itr == p_itr->voters_for.end(), "User has already voted.");

    auto voter_against_itr = std::find (p_itr->voters_against.begin(), p_itr->voters_against.end(), voter);
    eosio::check (voter_against_itr == p_itr->voters_against.end(), "User has already voted.");

    p_t.modify (p_itr, get_self(), [&](auto &p) {
        p.votes_for++;
        p.voters_for.push_back (voter);
    });
}

ACTION gyftietoken::voteagainst (const name voter,
                            const uint64_t proposal_id) 
{
    eosio::check (! is_paused(), "Contract is paused." );

    require_auth (voter);
    eosio::check (is_tokenholder (voter), "Voter is not a GFT token holder.");
    
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (proposal_id);
    eosio::check (current_block_time().to_time_point().sec_since_epoch() <= p_itr->expiration_date, "Proposal has expired.");

    auto voter_for_itr = std::find (p_itr->voters_for.begin(), p_itr->voters_for.end(), voter);
    eosio::check (voter_for_itr == p_itr->voters_for.end(), "User has already voted.");

    auto voter_against_itr = std::find (p_itr->voters_against.begin(), p_itr->voters_against.end(), voter);
    eosio::check (voter_against_itr == p_itr->voters_against.end(), "User has already voted.");

    p_t.modify (p_itr, get_self(), [&](auto &p) {
        p.votes_against++;
        p.voters_against.push_back (voter);
    });
}

ACTION gyftietoken::calcgyft (name from, name to) 
{

}

ACTION gyftietoken::gyft (const name from, 
                            const name to, 
                            const string idhash,
                            const string relationship)
{
    gyft2 (from, to, idhash, relationship, string {"No expiration date provided."});
}

ACTION gyftietoken::gyft2 (const name from, 
                            const name to, 
                            const string idhash,
                            const string relationship,
                            const string id_expiration)
{
    throttle_gyfts();
    eosio::check (! is_paused(), "Contract is paused." );
    permit_account(from);

    require_auth (from);
    eosio::check (is_tokenholder (from), "Gyfter must be a GFT token holder.");

    asset creation_fee = get_one_gft() * 0;
    if ( !is_account (to)) {
        creation_fee = create_account_from_request (from, to);
    }

    eosio::check (!is_gyftie_account(to), "Receipient must not be a Gyftie account.");

    insert_profile (to, idhash, id_expiration);
    
    config_table config (get_self(), get_self().value);
    auto c = config.get();
   
    asset issue_to_gyfter = get_gyfter_reward(from);
    asset issue_to_gyftee = get_recipient_reward(); 

    string to_gyfter_memo { "Vouch-Gyft-Earn GFT-creation to Gyfter. See 'How Gyftie Works' document - ask us for link." };
    string to_gyftee_memo { "Vouch-Gyft-Earn GFT-creation to new user. See 'How Gyftie Works' document - ask us for link." };
    string to_gyftiegyftie {"Vouch-Gyft-Earn GFT-creation to Gyftie venture (gyftiegyftie). See 'How Gyftie Works' document - ask us for link."};
    string auto_liquidity_memo { "Vouch-Gyft-Earn GFT-creation to liquidity providers. See 'How Gyftie Works' document - ask us for link." };
 
    float share_for_liquidity_reward = 0.100000000000;
    asset gyft_inflation = issue_to_gyfter + issue_to_gyftee;
    asset amount_to_gyftiegyftie = asset { static_cast<int64_t> (gyft_inflation.amount * (1 - share_for_liquidity_reward)), issue_to_gyfter.symbol };
    asset liquidity_reward = gyft_inflation - amount_to_gyftiegyftie;

    action (
        permission_level{get_self(), "owner"_n},
        get_self(), "issuetostake"_n,
        std::make_tuple(from, issue_to_gyfter - creation_fee, to_gyfter_memo))
    .send();

    action (
        permission_level{get_self(), "owner"_n},
        get_self(), "issue"_n,
        std::make_tuple(c.gyftie_foundation, amount_to_gyftiegyftie, to_gyftiegyftie))
    .send();

    action (
        permission_level{get_self(), "owner"_n},
        get_self(), "issue"_n,
        std::make_tuple(c.gftorderbook, liquidity_reward, auto_liquidity_memo))
    .send();

    action (
        permission_level{get_self(), "owner"_n},
        c.gftorderbook, "addliqreward"_n,
        std::make_tuple(liquidity_reward))
    .send();

    action (
        permission_level{get_self(), "owner"_n},
        get_self(), "issuetostake"_n,
        std::make_tuple(to, issue_to_gyftee, to_gyftee_memo))
    .send();

    // Add 20% of Gyft'ed balance to the order book
    // asset auto_liquidity_add = adjust_asset (issue_to_gyftee, 0.20000000);
    // action (
    //     permission_level{get_self(), "owner"_n},
    //     get_self(), "transfer"_n,
    //     std::make_tuple(to, c.gftorderbook, auto_liquidity_add, auto_liquidity_memo))
    // .send();

    // action (
    //     permission_level{get_self(), "owner"_n},
    //     c.gftorderbook, "stacksell"_n,
    //     std::make_tuple(to, auto_liquidity_add))
    // .send();

    addgyft (from, to, issue_to_gyfter, issue_to_gyftee, relationship);
}

ACTION gyftietoken::ungyft(const name account)
{
    require_auth (get_self());

    gyft_table g_t (get_self(), get_self().value);
    // auto gyftee_index = g_t.get_index<"bygyftee"_n>();
    // auto gyftee_itr = gyftee_index.find (account.value);
    // eosio::check (gyftee_itr != gyftee_index.end(), "Gyft event not found for account.");

    // auto gyfter_index = g_t.get_index<"bygyfter"_n>();
    // auto gyfter_itr = gyfter_index.find (account.value);
    // eosio::check (gyfter_itr == gyfter_index.end(), "Account issued a gyft; must ungyft downstream accounts first.");

    auto g_itr = g_t.begin();
    while (g_itr != g_t.end()) {
        eosio::check (g_itr->gyfter != account, "Account issued a gyft; must ungyft downstream accounts first.");
        g_itr++;
    }

    bool found = false;
    g_itr = g_t.begin();
    while (g_itr != g_t.end() and !found) {
        if (g_itr->gyftee == account) {
            found = true;
        } else {
            g_itr++;
        }
    }

    // burn tokens for gyfter, gyftee, and gyftiegyftie
    burn (account, g_itr->gyftee_issue);
    burn (g_itr->gyfter, g_itr->gyfter_issue);

    config_table config (get_self(), get_self().value);
    auto c = config.get();
    burn (c.gyftie_foundation, g_itr->gyftee_issue + g_itr->gyfter_issue);

    profile_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (account.value);
    eosio::check (p_itr != p_t.end(), "Account profile is not found.");
    p_t.erase (p_itr);

    accounts a_t(get_self(), account.value);
    auto a_itr = a_t.find(g_itr->gyftee_issue.symbol.code().raw());
    eosio::check (a_itr != a_t.end(), "Gyftee not found in accounts table under GFT symbol.");
    a_t.erase (a_itr);

    g_t.erase (g_itr);
}

ACTION gyftietoken::create()
{
    require_auth(get_self());

    symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

    eosio::check(sym.is_valid(), "invalid symbol name");

    stats statstable(get_self(), sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio::check(existing == statstable.end(), "token with symbol already exists");

    statstable.emplace(get_self(), [&](auto &s) {
        s.symbol = sym;
        s.supply.symbol = sym;
        s.issuer = get_self();
    });

    insert_profile (get_self(), "ISSUER-HASH-PLACEHOLDER", "NO EXPIRATION");
}

ACTION gyftietoken::issue(const name to, const asset quantity, const string memo)
{
    auto sym = quantity.symbol;
    eosio::check(sym.is_valid(), "invalid symbol name");
    eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

    auto sym_name = sym.code().raw();
    stats statstable(_self, sym_name);
    auto existing = statstable.find(sym_name);
    eosio::check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto &st = *existing;

    eosio::check (has_auth (st.issuer) || has_auth (get_self()), "issue requires authority of issuer or token contract.");
    
    eosio::check(quantity.is_valid(), "invalid quantity");
    eosio::check(quantity.amount > 0, "must issue positive quantity");
    eosio::check(quantity.symbol == st.symbol, "symbol precision mismatch");

    statstable.modify(st, eosio::same_payer, [&](auto &s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer);

    if (to != st.issuer)
    {
        //transfer (st.issuer, to, quantity, memo);
        SEND_INLINE_ACTION(*this, transfer, {st.issuer, "owner"_n}, {st.issuer, to, quantity, memo});
    }
}

ACTION gyftietoken::transfer(const name from, const name to, const asset quantity, const string memo)
{
    eosio::check (! is_paused(), "Contract is paused." );
    permit_account(from);
    
    eosio::check(from != to, "cannot transfer to self");
    eosio::check (has_auth (get_self()) || has_auth (from), "Permission denied - cannot transfer.");

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    eosio::check(is_account(to), "to account does not exist");    
    eosio::check(is_gyftie_account(to) || c.gftorderbook == to || c.gyftie_foundation == to, "Recipient is not a Gyftie account. Must Gyft first.");
   
    if (to != c.gftorderbook || to != c.gyftie_foundation || to != get_self()) {
        availrating_table a_t (get_self(), to.value);
        auto a_itr = a_t.find (from.value);
        if (a_itr != a_t.end()) {
            a_t.modify (a_itr, get_self(), [&](auto &a) {
                a.rate_deadline = current_block_time().to_time_point().sec_since_epoch() + (60 * 60 * 24);
            });
        } else {
            a_t.emplace (get_self(),  [&](auto &a) {
                a.ratee = from;
                a.rate_deadline = current_block_time().to_time_point().sec_since_epoch() + (60 * 60 * 24);
            });
        }
    }

    auto sym = quantity.symbol.code().raw();
    stats statstable(_self, sym);
    const auto &st = statstable.get(sym);

    require_recipient(from);
    require_recipient(to);

    eosio::check(quantity.is_valid(), "invalid quantity");
    eosio::check(quantity.amount > 0, "must transfer positive quantity");
    eosio::check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

    sub_balance(from, quantity);
    add_balance(to, quantity, get_self());
}

ACTION gyftietoken::xfertostake(const name from, const name to, const asset quantity, const string memo)
{
    transfer (from, to, quantity, memo);
    stake (to, quantity);
}

ACTION gyftietoken::issuetostake (const name to, const asset quantity, const string memo)
{
    issue (to, quantity, memo);
    stake (to, quantity);
}

ACTION gyftietoken::unstaked (const name user, const asset quantity) 
{
    require_auth (get_self());
    unstake (user, quantity);
}

ACTION gyftietoken::requnstake (const name user, const asset quantity)
{
    require_auth (user);

    profile_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (user.value);
    eosio::check (p_itr != p_t.end(), "Account profile not found.");

    eosio::check (p_itr->staked_balance >= quantity, "Requested unstake quantity exceeds staked balance.");

    asset remaining_stake = quantity;

    uint32_t    delay_increment = 60 * 60 * 24;  // one day
    uint32_t    delay = delay_increment;
    float       stake_increment = 0.05000000000;

    while (remaining_stake.amount > 0) {
        asset current_stake = asset {std::min(remaining_stake.amount, 
                                              adjust_asset (quantity, stake_increment).amount),
                                     remaining_stake.symbol};

        defer_unstake(user, current_stake, delay);
        delay += delay_increment;
        remaining_stake -= current_stake;
    }
}

EOSIO_DISPATCH(gyftietoken, (setconfig)(delconfig)(create)(issue)(transfer)(calcgyft) //(copygyfts1)(copygyfts2)(deloriggyfts)
                            (gyft)(propose)(votefor)(voteagainst)(pause)(unpause)(addrating)(requnstake)(unstaked)(remsig)(addsig)
                            (removeprop)(ungyft)(gyft2)(setstate)(dchallenge)(chgthrottle)(issuetostake)(xfertostake)(addlock)(unlock)
                            (nchallenge)(validate)(addcnote))
