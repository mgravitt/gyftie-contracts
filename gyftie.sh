cleos create account eosio gyftietoken EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio genesisgen EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio secondgen EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio issuer EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio holder1 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio holder2 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio voter1 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio voter2 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio voter3 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn

cleos push action eosio updateauth '{"account":"gyftietoken","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"gyftietoken","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p gyftietoken
cleos push action eosio updateauth '{"account":"genesisgen","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"genesisgen","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p genesisgen

cleos set contract gyftietoken gyftietoken/
cleos set contract genesisgen genesisgen/
cleos set contract secondgen secondgen/

cleos push action gyftietoken setconfig '["genesisgen"]' -p gyftietoken
cleos push action gyftietoken create '["issuer"]' -p gyftietoken
cleos push action gyftietoken issue '["holder1", "100.0000 GYFTIE", "memo"]' -p issuer

cleos push action gyftietoken calcgyft '["holder1", "holder2"]' -p holder1
cleos push action gyftietoken gyft '["holder1", "holder2", "idhash12345", "memo"]' -p holder1

#cleos push action genesisgen generate '["gyftietoken", "GYFTIE", 4, "holder1", "holder2"]' -p gyftietoken
#cleos push action gyftietoken issue '["voter1", "100.0000 GYFTIE", "memo"]' -p issuer
#cleos push action gyftietoken issue '["voter2", "100.0000 GYFTIE", "memo"]' -p issuer

cleos push action gyftietoken gyft '["holder1", "voter1", "idhash54321", "memo"]' -p holder1
cleos push action gyftietoken gyft '["holder2", "voter2", "idhash54321", "memo"]' -p holder2

cleos push action gyftietoken propose '["voter1", "secondgen", "Second generation calculation"]' -p voter1

cleos push action gyftietoken gyft '["voter2", "voter3", "idhash54321", "memo"]' -p voter2
