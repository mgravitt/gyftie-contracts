
cleos -u https://eos.greymass.com get table gftorderbook gftorderbook buyorders
cleos -u https://eos.greymass.com get table gftorderbook gftorderbook sellorders

cleos -u https://eos.greymass.com push action gftorderbook delbuyorder '[0]' -p zombiejigsaw
cleos -u https://eos.greymass.com push action gftorderbook delbuyorder '[1]' -p zombiejigsaw
cleos -u https://eos.greymass.com push action gftorderbook delbuyorder '[2]' -p zombiejigsaw
cleos -u https://eos.greymass.com push action gftorderbook delbuyorder '[3]' -p zombiejigsaw

cleos -u https://eos.greymass.com push action gftorderbook delsellorder '[0]' -p zombiejigsaw
cleos -u https://eos.greymass.com push action gftorderbook delsellorder '[2]' -p zombiejigsaw
cleos -u https://eos.greymass.com push action gftorderbook delsellorder '[3]' -p zombiejigsaw

cleos -u https://eos.greymass.com get table gftorderbook gftorderbook buyorders
cleos -u https://eos.greymass.com get table gftorderbook gftorderbook sellorders
cleos -u https://eos.greymass.com get table eosio.token gftorderbook accounts
cleos -u https://eos.greymass.com get table gyftietokens gftorderbook accounts


cleos -u https://eos.greymass.com push action gftorderbook admindelso '[4]' -p gftorderbook
cleos -u https://eos.greymass.com push action gftorderbook admindelso '[5]' -p gftorderbook

# UPDATE DEPLOY ON CONTRACT

cleos -u https://eos.greymass.com set contract gyftietokgen genesisgen/
cleos -u https://eos.greymass.com set contract gyftietokens gyftietoken/
cleos -u https://eos.greymass.com set contract gftorderbook gftorderbook/

cleos -u https://eos.greymass.com push action gftorderbook limitsellgft '["lamardouglas", "450.0000 EOS", "1.00000000 GFT"]' -p gftorderbook
cleos -u https://eos.greymass.com push action gftorderbook limitsellgft '["decmichael22", "750.0000 EOS", "1.40000000 GFT"]' -p gftorderbook

cleos -u https://eos.greymass.com push action gftorderbook delconfig '[]' -p gftorderbook
cleos -u https://eos.greymass.com push action gftorderbook setconfig '["gyftietokens", "eosio.token", "EOS", 4]' -p gftorderbook


#cleos -u https://eos.greymass.com push action gyftietokens delconfig '[]' -p gyftietokens

# add gyftiegyftie to config

#cleos -u https://eos.greymass.com set contract gyftietokens gyftietoken/
#cleos -u https://eos.greymass.com push action gyftietokens setconfig '["gyftietokgen", "gftorderbook", "gyftiegyftie"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens setcounter '[15]' -p gyftietokens



######


cleos -u https://eos.greymass.com set account permission gyftietokgen active '{"threshold":2,"keys":[],"accounts":[{"permission":{"actor":"danielflora3","permission":"active"},"weight":1},{"permission":{"actor":"zombiejigsaw","permission":"active"},"weight":1},{"permission":{"actor":"gyftietokgen","permission":"eosio.code"},"weight":1}],"waits":[]}' owner -p gyftietokgen@owner




######

# INTO MAINTENANCE
cleos -u https://eos.greymass.com push action gyftietokens pause '[]' -p gyftietokens
cleos -u https://eos.greymass.com push action gftorderbook pause '[]' -p gftorderbook

#cleos -u https://eos.greymass.com push action gyftietokens delconfig '[]' -p gyftietokens
cleos -u https://eos.greymass.com set contract gyftietokens gyftietoken/
cleos -u https://eos.greymass.com set contract gftorderbook gftorderbook/

#cleos -u https://eos.greymass.com push action gyftietokens setconfig '["gyftietokgen", "gftorderbook", "gyftiegyftie"]' -p gyftietokens

cleos -u https://eos.greymass.com push action gyftietokgen reset '[]' -p gyftietokgen
cleos -u https://eos.greymass.com set contract gyftietokgen genesisgen/


cleos -u https://eos.greymass.com push action gyftietokens unpause '[]' -p gyftietokens
cleos -u https://eos.greymass.com push action gftorderbook unpause '[]' -p gftorderbook

# OUT OF MAINTENANCE







# Step 1 - show permission change - https://bloks.io/account/gyftietokgen
cleos -u https://eos.greymass.com set account permission gyftietokgen active '{"threshold":2,"keys":[],"accounts":[{"permission":{"actor":"danielflora3","permission":"active"},"weight":1},{"permission":{"actor":"zombiejigsaw","permission":"active"},"weight":1}],"waits":[]}' owner -p gyftietokgen@owner
cleos -u https://eos.greymass.com set account permission gyftietokgen owner '{"threshold":1,"keys":[],"accounts":[{"permission":{"actor":"gyftietokgen","permission":"eosio.code"},"weight":1}],"waits":[]}'  -p gyftietokgen@owner


# Step 2 - will need Github ID
cleos -u https://eos.greymass.com set contract -sjd -x 86400 gyftietokgen genesisgen/ > trx.json
cleos -u https://eos.greymass.com multisig propose_trx multisigtest '[{"actor": "zombiejigsaw", "permission": "active"}, {"actor": "danielflora3", "permission": "active"}]' ./trx.json zombiejigsaw
cleos -u https://eos.greymass.com multisig review zombiejigsaw multisigtest
cleos -u https://eos.greymass.com  push action gyftmultisig addproposal '["zombiejigsaw", "danielflora3", "multisigtest", "Multisig test to update token gen contract", "https://github.com/digital-scarcity/gyftie/commit/f67f38660971514b7cd36d1703b02f51534fa567"]' -p zombiejigsaw

# Step 3 
Daniel approve via Scatter on UI

# Step 4 - Approve and Execute
cleos -u https://eos.greymass.com multisig approve zombiejigsaw multisigtest '{"actor": "zombiejigsaw", "permission": "active"}' -p zombiejigsaw
cleos -u https://eos.greymass.com multisig exec zombiejigsaw multisigtest -p zombiejigsaw

