
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
cleos -u https://eos.greymass.com set account permission gftorderbook active '{"threshold":2,"keys":[],"accounts":[{"permission":{"actor":"danielflora3","permission":"active"},"weight":1},{"permission":{"actor":"zombiejigsaw","permission":"active"},"weight":1}],"waits":[]}' owner -p gftorderbook@owner
cleos -u https://eos.greymass.com set account permission gftorderbook owner '{"threshold":1,"keys":[],"accounts":[{"permission":{"actor":"gftorderbook","permission":"eosio.code"},"weight":1}],"waits":[]}'  -p gftorderbook@owner




# Step 1 - show permission change - https://bloks.io/account/gyftietokgen
cleos -u https://eos.greymass.com set account permission gyftietokgen active '{"threshold":2,"keys":[],"accounts":[{"permission":{"actor":"danielflora3","permission":"active"},"weight":1},{"permission":{"actor":"zombiejigsaw","permission":"active"},"weight":1}],"waits":[]}' owner -p gyftietokgen@owner
cleos -u https://eos.greymass.com set account permission gyftietokgen owner '{"threshold":1,"keys":[],"accounts":[{"permission":{"actor":"gyftietokgen","permission":"eosio.code"},"weight":1}],"waits":[]}'  -p gyftietokgen@owner


# Step 2 - will need Github ID
cleos -u https://eos.greymass.com set contract -sjd -x 86400 gftorderbook gftorderbook/ > trx.json
cleos -u https://eos.greymass.com multisig propose_trx gftpermbug '[{"actor": "zombiejigsaw", "permission": "active"}, {"actor": "danielflora3", "permission": "active"}]' ./trx.json zombiejigsaw
cleos -u https://eos.greymass.com multisig review zombiejigsaw gftpermbug
cleos -u https://eos.greymass.com  push action gyftmultisig addproposal '["zombiejigsaw", "danielflora3", "gftpermbug", "Fix GFT Order Book permissions bug", "https://github.com/digital-scarcity/gyftie/commit/094bbb3e7f31f9a3222fad8dd743d92f1d27ace0"]' -p zombiejigsaw

# Step 3 
Daniel approve via Scatter on UI

# Step 4 - Approve and Execute
cleos -u https://eos.greymass.com multisig approve zombiejigsaw gftpermbug '{"actor": "zombiejigsaw", "permission": "active"}' -p zombiejigsaw
cleos -u https://eos.greymass.com multisig exec zombiejigsaw upgyftben -p zombiejigsaw




###############

cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["zombiejigsaw"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["1sharky3dog5"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["abbybabygirl"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["amandarachel"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["andrewboyle1"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["annewisloves"]' -p gyftietokens


cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["boobooswimmy"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["dabbarrowman"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["danconover13"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["danielflora3"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["decmichael22"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["djohnstonec1"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["dmweisberger"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["ednaggstoken"]' -p gyftietokens



cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["elbamboevic3"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["erinmcdowell"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["exactly12chr"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["fitnessfreak"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["frankmbel135"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["hiltonsupra1"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["ianquah12345"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["jonmaycock12"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["joshjohnpott"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["kunalsoodnyc"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["ladymichelle"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["lamardouglas"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["majkengilmar"]' -p gyftietokens


cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["marcingalily"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["marksuzelted"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["penelopeanns"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["prasanjitdey"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["ratonvaquero"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["rebeccarory1"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["rhzytvxaorvk"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["robertocampo"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["rushmillerkl"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["samchaudhry1"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["scottlazerso"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["timbobjoeboy"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["ttqqqqtt5eos"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["tylerbarth12"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens sudoprofile '["vickiepotter"]' -p gyftietokens













#######

1. pause
2. clearstate
3. DEPLOY
4. setstate
5. unpause