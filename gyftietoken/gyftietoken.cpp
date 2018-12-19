#include "gyftietoken.hpp"

ACTION gyftietoken::setconfig (name token_gen) 
{
    require_auth (get_self());
    config _config;
    _config.token_gen = token_gen;
    _config.account_count = 0;
    
    config_singleton c_singleton (get_self(), get_self().value);

    c_singleton.set( _config, get_self());
}

ACTION gyftietoken::propose (name proposer,
                                name token_gen,
                                string notes) 
{
    require_auth (proposer);
    proposal_table p_t (get_self(), get_self().value);

    p_t.emplace (proposer, [&](auto &p) {
        p.proposal_id   = p_t.available_primary_key();
        p.created_date  = now();
        p.proposer      = proposer;
        p.new_token_gen = token_gen;
        p.votes_for     = 0;
        p.expiration_date   = now() + (60 * 60);  // 1 hour
    });
}

ACTION gyftietoken::vote (name voter,
                            uint64_t proposal_id) 
{
    require_auth (voter);

    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (proposal_id);

    auto voter_itr = std::find (p_itr->voters.begin(), p_itr->voters.end(), voter);
    eosio_assert (voter_itr == p_itr->voters.end(), "User has already voted.");

    p_t.modify (p_itr, eosio::same_payer, [&](auto &p) {
        p.votes_for++;
        p.voters.push_back (voter);
    });
}

ACTION gyftietoken::gyft (name from, 
                            name to, 
                            string idhash,
                            string memo) 
{
    symbol sym = symbol{symbol_code(GYFTIE_SYM_STR.c_str()), GYFTIE_PRECISION};
    
    config_singleton c_singleton (get_self(), get_self().value);
    auto c_s = c_singleton.get();

    action(
        permission_level{from, "active"_n},
        c_s.token_gen, "generate"_n,
        std::make_tuple(get_self(), GYFTIE_SYM_STR, GYFTIE_PRECISION, from, to))
    .send();

    tokengen_singleton t_singleton (get_self(), get_self().value);
    auto t_s = t_singleton.get();

    print (" Generated Amount:  ", t_s.generated_amount, "\n");
    
    issue (to, t_s.generated_amount, "Automatic Generation from Gyft");

    transfer (from, to, get_supply(get_self(), sym.code()), "Transfer via Gyft Event");
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

    require_auth(get_self());
    
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

    from_acnts.modify(from, owner, [&](auto &a) {
        a.balance -= value;
    });
}

void gyftietoken::add_balance(name owner, asset value, name ram_payer)
{
    accounts to_acnts(_self, owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());
    if (to == to_acnts.end())
    {
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

EOSIO_DISPATCH(gyftietoken, (setconfig)(create)(issue)(transfer)(gyft)(propose)(vote))