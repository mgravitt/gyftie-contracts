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

// TESTING ONLY -- DELETE FOR PRODUCTION
ACTION gyftietoken::reset () 
{
    require_auth (get_self());

    delconfig();

    symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

    stats s_t(_self, sym.code().raw());
    auto s_itr = s_t.find(sym.code().raw());
    s_t.erase (s_itr);

    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.begin();
    while (p_itr != p_t.end()) {
        p_itr = p_t.erase (p_itr);
    }
}

// TESTING ONLY -- DELETE FOR PRODUCTION
ACTION gyftietoken::burnall (name tokenholder) 
{
    symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

    accounts a_t (get_self(), tokenholder.value);
    auto a_itr = a_t.find (sym.code().raw());
    eosio_assert (a_itr != a_t.end(), "Gyfter does not have a GYFTIE balance.");

    a_t.erase (a_itr);
}

ACTION gyftietoken::removeprop (uint64_t proposal_id) 
{
    require_auth (get_self());
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find(proposal_id);
    eosio_assert (p_itr != p_t.end(), "Proposal ID is not found.");

    p_t.erase (p_itr);
}

ACTION gyftietoken::propose (name proposer,
                               // name token_gen,
                                string notes) 
{
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
                            string idhash) 
{
    require_auth (from);
    eosio_assert (is_tokenholder (from), "Gyfter must be a GFT token holder.");
    eosio_assert (!is_gyftie_account(to), "Receipient must not be a Gyftie account.");

    save_idhash (to, idhash);

    config_table config (get_self(), get_self().value);
    auto c = config.get();

    tokengen_table t_t (c.token_gen, c.token_gen.value);
    auto t_itr = t_t.begin();
    eosio_assert (t_itr != t_t.end(), "Token generation amount not found.");
    eosio_assert (t_itr->from == from, "Token generation calculation -from- address does not match gyft. Recalculate.");
    eosio_assert (t_itr->to == to, "Token generation calculation -to- address does not match gyft. Recalculate.");
   
    asset issue_to_gyfter = t_itr->generated_amount;
    asset issue_to_gyftee = getgftbalance (from);

    string to_gyfter_memo { "To Gyfter" };
    string to_gyftee_memo { "Gyft" };

    action (
        permission_level{get_self(), "active"_n},
        get_self(), "issue"_n,
        std::make_tuple(from, issue_to_gyfter, to_gyfter_memo))
    .send();

    action (
        permission_level{get_self(), "active"_n},
        get_self(), "issue"_n,
        std::make_tuple(c.gyftie_foundation, issue_to_gyftee, to_gyftee_memo))
    .send();

    action (
        permission_level{get_self(), "active"_n},
        get_self(), "issue"_n,
        std::make_tuple(to, issue_to_gyftee, to_gyftee_memo))
    .send();
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

    save_idhash (get_self(), "ISSUER-HASH-PLACEHOLDER");
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
    eosio_assert(from != to, "cannot transfer to self");
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");

    config_table config (get_self(), get_self().value);
    auto c = config.get();
    eosio_assert(is_gyftie_account(to) || c.gftorderbook == to || c.gyftie_foundation == to, "Recipient is not a Gyftie account. Must Gyft first.");
   
    auto sym = quantity.symbol.code().raw();
    stats statstable(_self, sym);
    const auto &st = statstable.get(sym);

    require_recipient(from);
    require_recipient(to);

    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must transfer positive quantity");
    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto payer = has_auth(to) ? to : from;

    sub_balance(from, quantity);
    add_balance(to, quantity, payer);
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

    from_acnts.modify(from, owner, [&](auto &a) {
        a.balance -= value;

        // if (a.balance.amount == 0 && st.issuer != owner){
        //     decrement_account_count();
        // }
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
                            (gyft)(propose)(votefor)(voteagainst)(reset)
                            (burnall)(removeprop)(setcounter))
