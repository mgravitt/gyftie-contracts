// import { Api, JsonRpc, RpcError } from 'eosjs';

// import JsSignatureProvider from 'eosjs/dist/eosjs-jssig'; // development only

const { Api, JsonRpc, RpcError, JsSignatureProvider } = require('eosjs');
//const JsSignatureProvider = require('eosjs/dist/eosjs-jssig');  // development only
const fetch = require('node-fetch');                            // node only; not needed in browsers
const { TextEncoder, TextDecoder } = require('util');           // node only; native TextEncoder/Decoder
//const { TextEncoder, TextDecoder } = require('text-encoding');  // React Native, IE11, and Edge Browsers only

const defaultPrivateKey = "5KAunQfsUCTrfy3cCdNyW5mMNnMSkH5EMP45oEjF2B6xKacoC3x"; // useraaaaaaaa
const signatureProvider = new JsSignatureProvider([defaultPrivateKey]);

const rpc = new JsonRpc('https://jungle.eosio.cr:443', { fetch });

const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() });


try {
    (async () => {
        const result = await api.transact({
            actions: [{
                account: 'gyftietoken1',
                name: 'propose',
                authorization: [{
                  actor: 'gyftieuser11',
                  permission: 'active',
                }],
                data: {
                  proposer: 'gyftieuser11',
                  token_gen: 'gygenesisgen',
                  notes: 'Back to genesis gen'
                },
              }]
          }, {
              blocksBehind: 3,
              expireSeconds: 30,
          });
        console.dir(result);
      })();
  } catch (e) {
    console.log('\nCaught exception: ' + e);
    if (e instanceof RpcError)
      console.log(JSON.stringify(e.json, null, 2));
  }

