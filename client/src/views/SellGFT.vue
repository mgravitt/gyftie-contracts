<template>
<div>
    <h2>Create New Sell Offer</h2>
    <v-flex >
      <v-text-field
        v-model="price_per_gft"
        :rules="[rules.required, rules.asset_format]"
        label="Offer Price per GFT (in EOS)"
        suffix="EOS"
        required
      ></v-text-field>
      
      <v-text-field
        v-model="gft_amount"
        :rules="[rules.required, rules.asset_format]"
        label="GFT Amount"
        suffix="GFT"
        required
      ></v-text-field>

    </v-flex>
    
    <br/>

    <v-btn color="primary" @click.prevent="place_sell_order">Place Sell Order</v-btn>

    <br/>
    <br/>
    <br/>

    <h2>Open Buy Orders</h2>
    <v-data-table
      :headers="headers"
      :items="buyorders"
      item-key="order_id">

      <template slot="items" slot-scope="props">
        <td>{{ props.item.order_id }}</td>
        <td class="text-xs-right">{{ props.item.buyer }}</td>
        <td class="text-xs-left">{{ props.item.price_per_gft}}</td>
        <td class="text-xs-right">{{ props.item.gft_amount }}</td>
        <td class="text-xs-right">{{ props.item.order_value }}</td>

        <td >
          <v-icon @click="accept_order(props.item)">
            transfer_within_a_station
          </v-icon>
          <v-icon @click="remove_order(props.item)">
            delete
          </v-icon>
        </td>
    
      </template>
    </v-data-table>

    <br/>
    <br/>
    <v-btn @click="login" v-if="scatter && !account">Login with Scatter</v-btn>
    <section v-if="scatter && account">
        Logged in with: {{account.name}}
        <v-btn @click="logout">Logout</v-btn>
    </section>
    <section v-if="result">
            <br>
            {{result}}
    </section>
</div>
</template>

<script>
  import ScatterJS, {Network} from 'scatterjs-core'
  import ScatterEOS from 'scatterjs-plugin-eosjs2'
  import { Api, JsonRpc } from 'eosjs'
  import { network_config, gyftiecontract, gftorderbook } from '../config';

  ScatterJS.plugins( new ScatterEOS() )

  const network = Network.fromJson(network_config);
  let eos
  const rpc = new JsonRpc(network.fullhost())
  export default {
  
    data () {
      return {
        sending:false,
        scatter:null,
        result:null,
        price_per_gft: "",
        gft_amount: "",
        rules: {
          required: value => !!value || 'Required.',
          asset_format: value => {
            const pattern = /^(([0-9]*)|(([0-9]*)\.([0-9]*)))$/
            return pattern.test(value) || 'Amount must be numeric. Do not include symbol.'
          }
        },
        headers: [
          {
            text: 'Buy Order ID',
            align: 'left',
            sortable: false,
            value: 'order_id'
          },
          { text: 'Buyer', value: 'buyer' },
          { text: 'Price', value: 'price_per_gft' },
          { text: 'GFT Amount', value: 'gft_amount' },
          { text: 'Order Value', value: 'order_value' },
          { text: 'Actions', value: '', sortable: false },
        ],
        buyorders: []
      }
    },

    mounted(){
      this.setEosInstance()

      ScatterJS.scatter.connect('Gyftie').then(connected => {
        if(!connected){
          this.result = 'Could not connect to Scatter. Please install, run, and/or restart.'
          return
        }
        this.scatter = ScatterJS.scatter
      })

       rpc.get_table_rows({
            "json": true,
            "code": gftorderbook, //"gyftietoken1",   // contract who owns the table
            "scope": gftorderbook,  // scope of the table
            "table": "buyorders",    // name of the table as specified by the contract abi
            "limit": 100,
        }).then( result => {
            this.buyorders = result.rows;
        });            
    },

    computed:{
      account(){
        if(!this.scatter || !this.scatter.identity) return null
        return this.scatter.identity.accounts[0]
      },
    },

    methods: {
      login(){
        this.scatter.getIdentity({accounts:[network]})
      },
      logout(){
        this.scatter.forgetIdentity()
      },
      async place_sell_order () {
        
        if(this.sending) return
        this.sending = true
       
        const completed = res => {
          this.result = res
          this.sending = false
        }

        try {
          const result = await eos.transact({
            actions: [
            {
              account: gyftiecontract,
              name: 'transfer',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                from: this.account.name,
                to: gftorderbook,
                quantity: Number.parseFloat(this.gft_amount).toFixed(8) + ' GFT',
                memo: 'Transfer for GFT Sell Order'
              },
            },
            {
              account: gftorderbook,
              name: 'limitsellgft',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                seller: this.account.name,
                price_per_gft: Number.parseFloat(this.price_per_gft).toFixed(4) + ' EOS',
                gft_amount: Number.parseFloat(this.gft_amount).toFixed(8) + ' GFT'
              },
            }
            
            ]
          }, {
              blocksBehind: 3,
              expireSeconds: 30,
          })
          completed(result)
        } catch (e) {
          completed(e)
        }
    },
    async accept_order (order) {
      if(this.sending) return
        this.sending = true
        
        const completed = res => {
          this.result = res
          this.sending = false
        }
        try {
          const result = await eos.transact({
            actions: [
              {
              account: gyftiecontract,
              name: 'transfer',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                from: this.account.name,
                to: gftorderbook,
                quantity: order.gft_amount,
                memo: 'Transfer for GFT Sell Order'
              },
            },
            {
              account: gftorderbook,
              name: 'sellgft',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                buyorder_id: order.order_id,
                seller: this.account.name
              },
            }
            ]
          }, {
              blocksBehind: 3,
              expireSeconds: 30,
          })
          completed(result)
        } catch (e) {
          completed(e)
        }
    },
    async remove_order(order) {
        if(this.sending) return
        this.sending = true
        
        const completed = res => {
          this.result = res
          this.sending = false
        }
        try {
          const result = await eos.transact({
            actions: [{
              account: gftorderbook,
              name: 'delbuyorder',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                buyorder_id: order.order_id
              },
            }]
          }, {
              blocksBehind: 3,
              expireSeconds: 30,
          })
          completed(result)
        } catch (e) {
          completed(e)
        }
    },
    setEosInstance(){
      if(this.account){
        eos = this.scatter.eos(network, Api, {rpc})
      } else {
        eos = new Api({ rpc })
      }
    },
  },
  watch:{
    ['account'](){
      this.setEosInstance()
    }
  }
}
</script>

<style lang="css">

</style>