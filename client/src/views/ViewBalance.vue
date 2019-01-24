<template>
  <div>
    <img src="../../public/Gyftie-Logo.png" height="150">
    <h2>Balance: {{gftbalance}} GFT</h2>

    <section v-if="scatter && account">
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
import { JsonRpc } from "eosjs";
import { network_config, gyftiecontract } from '../config';

ScatterJS.plugins(new ScatterEOS());

const network = Network.fromJson(network_config)
// let eos;
const rpc = new JsonRpc(network.fullhost());
export default {
  data() {
    return {
      sending: false,
      scatter: null,
      result: null,
      gftbalance: "",
      gftbalstr: "1.50000000 GFT"
    };
  },

  mounted() {

    ScatterJS.scatter.connect("Gyftie").then(connected => {
      if (!connected) {
        this.result = 'Could not connect to Scatter. Please install, run, and/or restart.'
        return;
      }
      this.scatter = ScatterJS.scatter;

      rpc
        .get_table_rows({
          json: true,
          code: gyftiecontract, 
          scope: this.scatter.identity.accounts[0].name, 
          table: "accounts", 
          limit: 100
        })
        .then(result => {
          if (result.rows.length == 0) {
            this.gftbalance = "0.00";
          } else {
            this.gftbalance = parseFloat(Math.round(
              parseFloat(
                result.rows[0].balance.substring(
                  0,
                  result.rows[0].balance.length - 4
                ) 
              ) * 100  
            ) / 100
            ).toFixed(2);
          }
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
      // mounted();
    },
    logout() {
      this.scatter.forgetIdentity();
    }
  },
  watch: {
    ["account"]() {
      
    }
  }
};
</script>

<style lang="css">
</style>