Private key: 5KSQeFjYx7D5s2iFvJNhYkwskJFeKxHsBTLwGQWE1UTp1push6X
Public key: EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj

cleos create account eosio eosio.token  EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio gyftietoken  EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio gftorderbook  EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio genesisgen EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio secondgen EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio gyftiegyftie EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio issuer EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder1 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder2 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder3 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder4 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder5 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj

cleos push action eosio updateauth '{"account":"gyftietoken","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"gyftietoken","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p gyftietoken
cleos push action eosio updateauth '{"account":"genesisgen","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"genesisgen","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p genesisgen
cleos push action eosio updateauth '{"account":"gftorderbook","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"gftorderbook","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p gftorderbook

cleos set contract gyftietoken gyftietoken/
cleos set contract genesisgen genesisgen/
cleos set contract secondgen secondgen/
cleos set contract eosio.token token/
cleos set contract gftorderbook gftorderbook/


#####

cleos push action eosio.token create '["eosio.token", "1000000000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["holder1", "100000.0000 EOS", "memo"]' -p eosio.token

cleos push action genesisgen setconfig '["0.10000000 GFT", 1.500000000000]' -p genesisgen
cleos push action gyftietoken setconfig '["genesisgen", "gftorderbook","gyftiegyftie"]' -p gyftietoken
cleos push action gyftietoken setcounter '[0]' -p gyftietoken
cleos push action gftorderbook setconfig '["gyftietoken", "eosio.token", "EOS", 4]' -p gftorderbook

cleos push action gyftietoken create '[]' -p gyftietoken
cleos push action gyftietoken issue '["gyftietoken", "8.00000000 GFT", "memo"]' -p gyftietoken

cleos push action gyftietoken calcgyft '["gyftietoken", "holder1"]' -p gyftietoken
cleos push action gyftietoken gyft '["gyftietoken", "holder1", "idhash12345"]' -p gyftietoken

cleos push action gyftietoken calcgyft '["holder1", "holder2"]' -p holder1
cleos push action gyftietoken gyft '["holder1", "holder2", "idhash12345"]' -p holder1

cleos push action eosio.token transfer '["holder1", "gftorderbook", "10000.0000 EOS", "should succeed"]' -p holder1
cleos push action gyftietoken transfer '["holder2", "gftorderbook", "6.00000000 GFT", "memo"]' -p holder2

cleos push action gyftietoken calcgyft '["holder2", "holder3"]' -p holder2
cleos push action gyftietoken gyft '["holder2", "holder3", "idhash12345"]' -p holder2

#cleos push action gftorderbook limitbuygft '["holder1", "1.0000 EOS", "0.05000000 GFT"]' -p holder1
cleos push action gftorderbook limitsellgft '["holder2", "5.0000 EOS", "0.10000000 GFT"]' -p holder2
cleos push action gftorderbook limitsellgft '["holder2", "5.1000 EOS", "0.20000000 GFT"]' -p holder2
cleos push action gftorderbook limitsellgft '["holder2", "5.2000 EOS", "0.30000000 GFT"]' -p holder2
cleos push action gftorderbook limitsellgft '["holder2", "5.4000 EOS", "0.20000000 GFT"]' -p holder2
cleos push action gftorderbook limitsellgft '["holder2", "5.5000 EOS", "0.30000000 GFT"]' -p holder2

cleos push action gftorderbook limitbuygft '["holder1", "6.2000 EOS", "0.03000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "6.1000 EOS", "0.02000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "6.0500 EOS", "0.10000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "5.9000 EOS", "0.08000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "5.8000 EOS", "0.02500000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "5.5000 EOS", "0.10000000 GFT"]' -p holder1




##############################################

cleos push action gftorderbook limitsellgft '["holder2", "7.0000 EOS", "0.10000000 GFT"]' -p holder2
cleos push action gftorderbook limitsellgft '["holder2", "6.1000 EOS", "0.20000000 GFT"]' -p holder2
cleos push action gftorderbook limitsellgft '["holder2", "6.8000 EOS", "0.30000000 GFT"]' -p holder2
cleos push action gftorderbook limitsellgft '["holder2", "6.4000 EOS", "0.20000000 GFT"]' -p holder2
cleos push action gftorderbook limitsellgft '["holder2", "8.5000 EOS", "0.30000000 GFT"]' -p holder2

cleos push action gftorderbook limitbuygft '["holder1", "6.2000 EOS", "0.03000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "6.1000 EOS", "0.02000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "6.0500 EOS", "0.10000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "5.9000 EOS", "0.08000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "5.8000 EOS", "0.02500000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "5.5000 EOS", "0.10000000 GFT"]' -p holder1



cleos push action gftorderbook limitbuygft '["holder1", "2.2000 EOS", "0.03000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "1.1000 EOS", "0.02000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "3.0500 EOS", "0.10000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "4.9000 EOS", "0.08000000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "2.8000 EOS", "0.02500000 GFT"]' -p holder1
cleos push action gftorderbook limitbuygft '["holder1", "1.5000 EOS", "0.10000000 GFT"]' -p holder1




cleos push action gftorderbook marketbuy '["holder1", "4.5000 EOS"]' -p holder1

cleos get table gyftietoken holder2 accounts
cleos get table gftorderbook holder2 balances

#########################


cleos push action gftorderbook sellgft '[2, "holder2"]' -p holder2
cleos push action gyftietoken marketselgft '["holder2", "2.00000000 GFT"]' -p holder2
cleos push action eosio.token transfer '["holder1", "gftorderbook", "50000.0000 EOS", "memo"]' -p holder1
cleos push action gyftietoken removeorders '[]' -p gyftietoken

cleos push action gyftietoken calcgyft '["holder2", "holder3"]' -p holder2
cleos push action gyftietoken gyft '["holder2", "holder3", "idhash54321", "memo"]' -p holder2

cleos push action gyftietoken propose '["holder1", "secondgen", "Second generation calculation"]' -p holder1

cleos push action gyftietoken vote '["holder1"]' -p holder1
cleos push action gyftietoken vote '["holder2"]' -p holder2

cleos push action gyftietoken calcgyft '["holder3", "holder4"]' -p holder3
cleos push action gyftietoken gyft '["holder3", "holder4", "idhash54321", "memo"]' -p holder3



