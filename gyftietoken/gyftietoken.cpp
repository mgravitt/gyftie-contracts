#include "gyftietoken.hpp"

ACTION gyftietoken::setconfig (name token_gen,
                                name gftorderbook,
                                name gyftie_foundation)
{
    require_auth (get_self());
        
    config_table config (get_self(), get_self().value);
    Config c;
    c.token_gen = token_gen;
    c.gftorderbook = gftorderbook;
    c.gyftie_foundation = gyftie_foundation;
    c.paused = PAUSED;
    config.set (c, get_self());
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

ACTION gyftietoken::setcounter (uint64_t account_count)
{
    require_auth (get_self());

    counter_table counter (get_self(), get_self().value);
    Counter c;
    c.account_count = account_count;
    counter.set(c, get_self());
}

ACTION gyftietoken::delconfig () 
{
    require_auth (get_self());
    config_table config (get_self(), get_self().value);
    config.remove();
}

ACTION gyftietoken::sudoprofile (name account) 
{
    require_auth (get_self());
    eosio_assert (is_tokenholder(account), "Account is not a token holder.");

    symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
    accounts a_t (get_self(), account.value);
    auto a_itr = a_t.find(sym.code().raw());

    insert_profile (account, a_itr->idhash);
}

ACTION gyftietoken::addrating (name rater, name ratee, uint8_t rating)
{
    availrating_table a_t (get_self(), rater.value);
    auto a_itr = a_t.find (ratee.value);
    eosio_assert (a_itr != a_t.end(), "Account is not available for rating.");
    eosio_assert (has_auth (rater), "Only eligible rater can add a rating.");
    eosio_assert (a_itr->rate_deadline >= now(), "Deadline to rate account has passed.");

    profile_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (ratee.value);
    
    if (p_itr == p_t.end()) {
        p_t.emplace (get_self(), [&](auto &p) {
            p.account = ratee;
            p.rating_count = 1;
            p.rating_sum = rating;
        });
    } else {
        p_t.modify (p_itr, get_self(), [&](auto &p) {
            p.rating_count++;
            p.rating_sum += rating;
        });
    }

    a_t.erase (a_itr);
}

ACTION gyftietoken::removeprop (uint64_t proposal_id) 
{
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find(proposal_id);
    eosio_assert (p_itr != p_t.end(), "Proposal ID is not found.");

    eosio_assert (has_auth (get_self()) || has_auth (p_itr->proposer), "Permission denied - only token contract or proposer can remove a proposal.");

    p_t.erase (p_itr);
}

ACTION gyftietoken::propose (name proposer,
                                string notes) 
{
    eosio_assert (! is_paused(), "Contract is paused." );
    require_auth (proposer);
    eosio_assert (is_tokenholder (proposer), "Proposer is not a GFT token holder.");

    proposal_table p_t (get_self(), get_self().value);

    p_t.emplace (proposer, [&](auto &p) {
        p.proposal_id   = p_t.available_primary_key();
        p.created_date  = now();
        p.proposer      = proposer;
      //  p.new_token_gen = token_gen;
        p.notes         = notes;
        p.votes_for     = 0;
        p.votes_against = 0;
        p.expiration_date   = now() + (60 * 60 * 24 * 30);  // 30 days
    });
}

ACTION gyftietoken::votefor (name voter,
                            uint64_t proposal_id) 
{
    eosio_assert (! is_paused(), "Contract is paused." );

    require_auth (voter);
    eosio_assert (is_tokenholder (voter), "Voter is not a GFT token holder.");
    
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (proposal_id);
    eosio_assert (now() <= p_itr->expiration_date, "Proposal has expired.");

    auto voter_for_itr = std::find (p_itr->voters_for.begin(), p_itr->voters_for.end(), voter);
    eosio_assert (voter_for_itr == p_itr->voters_for.end(), "User has already voted.");

    auto voter_against_itr = std::find (p_itr->voters_against.begin(), p_itr->voters_against.end(), voter);
    eosio_assert (voter_against_itr == p_itr->voters_against.end(), "User has already voted.");

    // uint32_t votes = 0;

    p_t.modify (p_itr, get_self(), [&](auto &p) {
        p.votes_for++;
        p.voters_for.push_back (voter);
    });

    // config_table config (get_self(), get_self().value);
    // auto c = config.get();

    // if (votes > (c.account_count / 2)) {
    //     c.token_gen = p_itr->new_token_gen;
    //     config.set (c, get_self());
    // }
}

ACTION gyftietoken::voteagainst (name voter,
                            uint64_t proposal_id) 
{
    eosio_assert (! is_paused(), "Contract is paused." );

    require_auth (voter);
    eosio_assert (is_tokenholder (voter), "Voter is not a GFT token holder.");
    
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (proposal_id);
    eosio_assert (now() <= p_itr->expiration_date, "Proposal has expired.");

    auto voter_for_itr = std::find (p_itr->voters_for.begin(), p_itr->voters_for.end(), voter);
    eosio_assert (voter_for_itr == p_itr->voters_for.end(), "User has already voted.");

    auto voter_against_itr = std::find (p_itr->voters_against.begin(), p_itr->voters_against.end(), voter);
    eosio_assert (voter_against_itr == p_itr->voters_against.end(), "User has already voted.");

    p_t.modify (p_itr, get_self(), [&](auto &p) {
        p.votes_against++;
        p.voters_against.push_back (voter);
    });
}

ACTION gyftietoken::calcgyft (name from, name to) 
{
    eosio_assert (! is_paused(), "Contract is paused." );

    require_auth (from);
    eosio_assert (is_tokenholder (from), "Gyfter is not a GFT token holder.");

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    action(
        permission_level{get_self(), "active"_n},
        c.token_gen, "generate"_n,
        std::make_tuple(from, getgftbalance (from), to))
    .send();
}

ACTION gyftietoken::gyft (name from, 
                            name to, 
                            string idhash,
                            string relationship) 
{
    eosio_assert (! is_paused(), "Contract is paused." );

    require_auth (from);
    eosio_assert (is_tokenholder (from), "Gyfter must be a GFT token holder.");
    eosio_assert (!is_gyftie_account(to), "Receipient must not be a Gyftie account.");

    insert_profile (to, idhash);

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    tokengen_table t_t (c.token_gen, c.token_gen.value);
    tokengen t = t_t.get();
    eosio_assert (t.from == from, "Token generation calculation -from- address does not match gyft. Recalculate.");
    eosio_assert (t.to == to, "Token generation calculation -to- address does not match gyft. Recalculate.");
 
    asset issue_to_gyfter = t.generated_amount;
    asset one_gyftie_token = asset { static_cast<int64_t>(pow(10, t.generated_amount.symbol.precision())), t.generated_amount.symbol};
    asset issue_to_gyftee = one_gyftie_token * 3; //getgftbalance (from);

    string to_gyfter_memo { "To Gyfter" };
    string to_gyftee_memo { "Gyft" };
    string auto_liquidity_memo { "Auto Liquidity Add to Order Book" };

    action (
        permission_level{get_self(), "active"_n},
        get_self(), "issue"_n,
        std::make_tuple(from, issue_to_gyfter, to_gyfter_memo))
    .send();

    action (
        permission_level{get_self(), "active"_n},
        get_self(), "issue"_n,
        std::make_tuple(c.gyftie_foundation, issue_to_gyfter + issue_to_gyftee, to_gyfter_memo))
    .send();

    action (
        permission_level{get_self(), "active"_n},
        get_self(), "issue"_n,
        std::make_tuple(to, issue_to_gyftee, to_gyftee_memo))
    .send();

    // Add 20% of Gyft'ed balance to the order book
    // asset auto_liquidity_add = adjust_asset (issue_to_gyftee, 0.20000000);
    // action (
    //     permission_level{get_self(), "active"_n},
    //     get_self(), "transfer"_n,
    //     std::make_tuple(to, c.gftorderbook, auto_liquidity_add, auto_liquidity_memo))
    // .send();

    // action (
    //     permission_level{get_self(), "active"_n},
    //     c.gftorderbook, "stacksell"_n,
    //     std::make_tuple(to, auto_liquidity_add))
    // .send();

    addgyft (from, to, issue_to_gyfter, issue_to_gyftee, relationship);
}

ACTION gyftietoken::create()
{
    require_auth(get_self());

    symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

    eosio_assert(sym.is_valid(), "invalid symbol name");

    stats statstable(get_self(), sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing == statstable.end(), "token with symbol already exists");

    statstable.emplace(get_self(), [&](auto &s) {
        s.symbol = sym;
        s.supply.symbol = sym;
        s.issuer = get_self();
    });

    insert_profile (get_self(), "ISSUER-HASH-PLACEHOLDER");
}

ACTION gyftietoken::issue(name to, asset quantity, string memo)
{
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto sym_name = sym.code().raw();
    stats statstable(_self, sym_name);
    auto existing = statstable.find(sym_name);
    eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto &st = *existing;

    eosio_assert (has_auth (st.issuer) || has_auth (get_self()), "issue requires authority of issuer or token contract.");
    
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must issue positive quantity");
    eosio_assert(quantity.symbol == st.symbol, "symbol precision mismatch");

    statstable.modify(st, eosio::same_payer, [&](auto &s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer);

    if (to != st.issuer)
    {
        SEND_INLINE_ACTION(*this, transfer, {st.issuer, "active"_n}, {st.issuer, to, quantity, memo});
    }
}


ACTION gyftietoken::transfer(name from, name to, asset quantity, string memo)
{
    eosio_assert (! is_paused(), "Contract is paused." );

    eosio_assert(from != to, "cannot transfer to self");

    config_table config (get_self(), get_self().value);
    auto c = config.get();
    if (to == c.gftorderbook) {
        eosio_assert (has_auth (get_self()) || has_auth (from), "Permission denied - only token contract and token owner can transfer to order book.");
    } else {
        require_auth(from);
    }

    eosio_assert(is_account(to), "to account does not exist");    
    eosio_assert(is_gyftie_account(to) || c.gftorderbook == to || c.gyftie_foundation == to, "Recipient is not a Gyftie account. Must Gyft first.");
   
    if (to != c.gftorderbook || to != c.gyftie_foundation || to != get_self()) {
        availrating_table a_t (get_self(), to.value);
        auto a_itr = a_t.find (from.value);
        if (a_itr != a_t.end()) {
            a_t.modify (a_itr, get_self(), [&](auto &a) {
                a.rate_deadline = now() + (60 * 60 * 24);
            });
        } else {
            a_t.emplace (get_self(),  [&](auto &a) {
                a.ratee = from;
                a.rate_deadline = now() + (60 * 60 * 24);
            });
        }
    }

    auto sym = quantity.symbol.code().raw();
    stats statstable(_self, sym);
    const auto &st = statstable.get(sym);

    require_recipient(from);
    require_recipient(to);

    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must transfer positive quantity");
    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    sub_balance(from, quantity);
    add_balance(to, quantity, get_self());
}

void gyftietoken::sub_balance(name owner, asset value)
{
    accounts from_acnts(_self, owner.value);

    const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
    eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");

    auto sym_name = value.symbol.code().raw();
    stats statstable(_self, sym_name);
    auto existing = statstable.find(sym_name);
    eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto &st = *existing;

    from_acnts.modify(from, get_self(), [&](auto &a) {
        a.balance -= value;
    });
}

void gyftietoken::add_balance(name owner, asset value, name ram_payer)
{
    accounts to_acnts(_self, owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());
    if (to == to_acnts.end())
    {
        auto sym_name = value.symbol.code().raw();
        stats statstable(_self, sym_name);
        auto existing = statstable.find(sym_name);
        eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
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
}


EOSIO_DISPATCH(gyftietoken, (setconfig)(delconfig)(create)(issue)(transfer)(calcgyft)
                            (gyft)(propose)(votefor)(voteagainst)(pause)(unpause)(addrating)
                            (removeprop)(setcounter))
