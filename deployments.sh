

cleos -u https://eos.greymass.com set contract gyftietokgen genesisgen/
cleos -u https://eos.greymass.com set contract gyftietokens gyftietoken/
cleos -u https://eos.greymass.com set contract gftorderbook gftorderbook/

cleos -u https://eos.greymass.com push action gyftietokens delconfig '[]' -p gyftietokens

# add gyftiegyftie to config

cleos -u https://eos.greymass.com set contract gyftietokens gyftietoken/
cleos -u https://eos.greymass.com push action gyftietokens setconfig '["gyftietokgen", "gftorderbook", "gyftiegyftie"]' -p gyftietokens
cleos -u https://eos.greymass.com push action gyftietokens setcounter '[15]' -p gyftietokens

