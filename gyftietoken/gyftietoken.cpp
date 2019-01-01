#include "gyftietoken.hpp"

ACTION gyftietoken::setconfig (name token_gen) 
{
    require_auth (get_self());
        
    config_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.begin();
    eosio_assert (c_itr == c_t.end(), "Genesis token gen contract has already been set.");

    c_t.emplace (get_self(), [&](auto &c) {
        c.token_gen = token_gen;
    });
}

// TESTING ONLY -- DELETE FOR PRODUCTION
ACTION gyftietoken::reset () 
{
    require_auth (get_self());

    config_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.begin();
    while (c_itr != c_t.end()) {
        c_itr = c_t.erase (c_itr);
    }

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

ACTION gyftietoken::propose (name proposer,
                                name token_gen,
                                string notes) 
{
    require_auth (proposer);
    is_tokenholder (proposer);

    proposal_table p_t (get_self(), get_self().value);

    p_t.emplace (proposer, [&](auto &p) {
        p.proposal_id   = p_t.available_primary_key();
        p.created_date  = now();
        p.proposer      = proposer;
        p.new_token_gen = token_gen;
        p.notes         = notes;
        p.votes_for     = 0;
        p.expiration_date   = now() + (60 * 60);  // 1 hour
    });
}

ACTION gyftietoken::vote (name voter,
                            uint64_t proposal_id) 
{
    require_auth (voter);
    is_tokenholder (voter);
    
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (proposal_id);
    eosio_assert (now() <= p_itr->expiration_date, "Proposal has expired.");

    auto voter_itr = std::find (p_itr->voters.begin(), p_itr->voters.end(), voter);
    eosio_assert (voter_itr == p_itr->voters.end(), "User has already voted.");

    uint32_t votes = 0;

    p_t.modify (p_itr, get_self(), [&](auto &p) {
        p.votes_for++;
        votes = p.votes_for;
        p.voters.push_back (voter);
    });

    config_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.begin();
    eosio_assert (c_itr != c_t.end(), "Configuration has not been set.");

    if (votes > (c_itr->account_count / 2)) {
        c_t.modify (c_itr, get_self(), [&](auto &c) {
            c.token_gen = p_itr->new_token_gen;
        });
    }
}

ACTION gyftietoken::calcgyft (name from, name to) 
{
    require_auth (from);
    is_tokenholder (from);

    config_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.begin();
    eosio_assert (c_itr != c_t.end(), "Configuration has not been set.");

    action(
        permission_level{get_self(), "active"_n},
        c_itr->token_gen, "generate"_n,
        std::make_tuple(get_self(), GYFTIE_SYM_STR, GYFTIE_PRECISION, from, to))
    .send();
}

ACTION gyftietoken::gyft (name from, 
                            name to, 
                            string idhash,
                            string memo) 
{

    require_auth (from);
    is_tokenholder (from);

    config_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.begin();
    eosio_assert (c_itr != c_t.end(), "Configuration has not been set.");

    tokengen_table t_t (c_itr->token_gen, c_itr->token_gen.value);
    auto t_itr = t_t.begin();
    eosio_assert (t_itr != t_t.end(), "Token generation amount not found.");
    eosio_assert (t_itr->from == from, "Token generation calculation -from- address does not match gyft. Recalculate.");
    eosio_assert (t_itr->to == to, "Token generation calculation -to- address does not match gyft. Recalculate.");

    //print ("    Generated Amount:  ", t_itr->generated_amount, "\n");
    
    action (
        permission_level{get_self(), "active"_n},
        get_self(), "issue"_n,
        std::make_tuple(from, t_itr->generated_amount, memo))
    .send();

    symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

    // print ("    After inline action issue\n");
    // print ("    sym.code()  :   ", sym.code(), "\n");
    // print ("    from        :   ", from, "\n");

    accounts a_t (get_self(), from.value);
    auto a_itr = a_t.find (sym.code().raw());
    eosio_assert (a_itr != a_t.end(), "Gyfter does not have a GYFTIE balance.");

    // print ("    Supply of From account:    ", a_itr->balance, "\n");

    paytoken (get_self(), from, to, a_itr->balance, memo);
}


ACTION gyftietoken::create(name issuer)
{
    require_auth(_self);

    symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};

    eosio_assert(sym.is_valid(), "invalid symbol name");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing == statstable.end(), "token with symbol already exists");

    statstable.emplace(_self, [&](auto &s) {
        s.symbol = sym;
        s.supply.symbol = sym;
        s.issuer = issuer;
    });
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

    //statstable.modify(st, eosio::same_payer, [&](auto &s) {
    statstable.modify(st, st.issuer, [&](auto &s) {
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

        if (a.balance.amount == 0 && st.issuer != owner){
            decrement_account_count();
        }
    });
}

void gyftietoken::add_balance(name owner, asset value, name ram_payer)
{
    accounts to_acnts(_self, owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());
    if (to == to_acnts.end())
    {
        increment_account_count ();
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

EOSIO_DISPATCH(gyftietoken, (setconfig)(create)(issue)(transfer)(calcgyft)(gyft)(propose)(vote)(reset)(burnall))