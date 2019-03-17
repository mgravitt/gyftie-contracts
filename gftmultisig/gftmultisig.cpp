#include "gftmultisig.hpp"

ACTION gftmultisig::addrequest (const name gyfter,
                                const name recipient,
                                const string owner_public_key,
                                const string active_public_key)
{
    gyftrequest_table g_t (get_self(), get_self().value);
    g_t.emplace (get_self(), [&](auto &g) {
        g.recipient = recipient;
        g.gyfter = gyfter;
        g.owner_public_key = owner_public_key;
        g.active_public_key = active_public_key;
        g.requested_date = now();
    });
}

ACTION gftmultisig::delrequest (const name recipient)
{
    gyftrequest_table g_t (get_self(), get_self().value);
    auto g_itr = g_t.find (recipient.value);
    eosio_assert (g_itr != g_t.end(), "Gyft request not found.");

    g_t.erase (g_itr);
}

ACTION gftmultisig::addproposal (const name proposer,
                                const name required_approver,
                                const string proposal_name,
                                const string proposal_text,
                                const string github_link)
{
    require_auth (proposer);

    proposal_table p_t (get_self(), get_self().value);
    p_t.emplace (get_self(), [&](auto &p) {
        p.proposal_id = p_t.available_primary_key();
        p.proposer = proposer;
        p.required_approver = required_approver;
        p.proposal_name = proposal_name;
        p.proposal_text = proposal_text;
        p.github_link = github_link;
        p.status = STATUS_OPEN;
        p.created_date = now();
    });
}

ACTION gftmultisig::delproposal (const uint64_t proposal_id)
{
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (proposal_id);
    eosio_assert (p_itr != p_t.end(), "Proposal ID is not found.");

    eosio_assert (  has_auth (get_self()) || 
                    has_auth(p_itr->proposer), "Permission to delete proposal denied.");

    p_t.erase (p_itr);
}

ACTION gftmultisig::approve(const uint64_t proposal_id) 
{
    proposal_table p_t (get_self(), get_self().value);
    auto p_itr = p_t.find (proposal_id);
    eosio_assert (p_itr != p_t.end(), "Proposal ID is not found.");

    require_auth (p_itr->required_approver);

    p_t.modify (p_itr, get_self(), [&](auto &p) {
        p.status = STATUS_APPROVED;
        p.approval_date = now();
    });
}

EOSIO_DISPATCH(gftmultisig, (addproposal)(approve)(delproposal)
                            (addrequest)(delrequest))