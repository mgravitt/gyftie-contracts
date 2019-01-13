<template>
  <div>
    <img src="../../public/Gyftie-Logo.png" height="150">
    <h2>Balance:</h2>

    <section v-if="scatter && account">
      <div>{{gftbalance}} GFT</div>
      Logged in with: {{account.name}}
      <v-btn @click="logout">Logout</v-btn>
    </section>

    <br>
    <br>

    <v-btn @click="login" v-if="scatter && !account">Login with Scatter</v-btn>
  </div>
</template>

<script>
import ScatterJS, { Network } from "scatterjs-core";
import ScatterEOS from "scatterjs-plugin-eosjs2";
import { Api, JsonRpc, RpcError, JsSignatureProvider } from "eosjs";

ScatterJS.plugins(new ScatterEOS());

const network = Network.fromJson({
  blockchain: "eos",
  host: "eos.greymass.com",
  port: 443,
  protocol: "https",
  chainId: "aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906"
});

// const network = Network.fromJson({
//   blockchain:'eos',
//   host:'jungle.eosio.cr',
//   port:443,
//   protocol:'https',
//   chainId:'e70aaab8997e1dfce58fbfac80cbbb8fecec7b99cf982a9444273cbc64c41473'
// })

let eos;
const rpc = new JsonRpc(network.fullhost());
export default {
  data() {
    return {
      sending: false,
      scatter: null,
      result: null,
      gftbalance: "",
      gftbalstr: "1.00000000 GFT"
    };
  },

  mounted() {
    this.setEosInstance();

    ScatterJS.scatter.connect("Gyftie").then(connected => {
      if (!connected) {
        console.error("Could not connect to Scatter.");
        return;
      }
      this.scatter = ScatterJS.scatter;

      rpc
        .get_table_rows({
          json: true,
          code: "gyftietokens", // contract who owns the table
          scope: this.scatter.identity.accounts[0].name, // scope of the table
          table: "accounts", // name of the table as specified by the contract abi
          limit: 100
        })
        .then(result => {
          if (result.rows.length == 0) {
            this.gftbalance = "0.00";
          } else {
            this.gftbalance = Math.round(
              parseFloat(
                result.rows[0].balance.substring(
                  0,
                  result.rows[0].balance.length - 4
                ) * 100
              ) / 100
            ).toFixed(2);
          }
          console.log(this.gftbalance);
        });
    });
  },

  computed: {
    account() {
      if (!this.scatter || !this.scatter.identity) return null;
      return this.scatter.identity.accounts[0];
    }
  },

  methods: {
    login() {
      this.scatter.getIdentity({ accounts: [network] });
      mounted();
    },
    logout() {
      this.scatter.forgetIdentity();
    },

    setEosInstance() {
      if (this.account) {
        eos = this.scatter.eos(network, Api, { rpc });
      } else {
        eos = new Api({ rpc });
      }
    }
  },
  watch: {
    ["account"]() {
      this.setEosInstance();
    }
  }
};
</script>

<style lang="css">
</style>