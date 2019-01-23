<template>
   
<div>
    <h2>Create New Buy Offer</h2>
    <v-flex >                      
        <v-form v-model="valid">
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
       </v-form>
    </v-flex>
    
    <br/>

     <v-btn color="primary" @click.prevent="place_buy_order">Place Buy Order</v-btn>

    <br/>
    <br/>
    <br/>
    <h2>Open Sell Orders</h2>
     <v-data-table
    :headers="headers"
    :items="sellorders"
    item-key="order_id">

    <template slot="items" slot-scope="props">
      <td>{{ props.item.order_id }}</td>
      <td class="text-xs-right">{{ props.item.seller }}</td>
      <td class="text-xs-left">{{ props.item.price_per_gft}}</td>
      <td class="text-xs-right">{{ props.item.gft_amount }}</td>
      <td class="text-xs-right">{{ props.item.order_value }}</td>

         <td >
          <v-icon
            small
            @click="accept_order(props.item)"
          >
            transfer_within_a_station
          </v-icon>
          <v-icon
            small
            @click="remove_order(props.item)"
          >
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
  import { Api, JsonRpc, RpcError, JsSignatureProvider } from 'eosjs'
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
            text: 'Sell Order ID',
            align: 'left',
            sortable: false,
            value: 'order_id'
          },
          { text: 'Seller', value: 'seller' },
          { text: 'Price', value: 'price_per_gft' },
          { text: 'GFT Amount', value: 'gft_amount' },
          { text: 'Order Value', value: 'order_value' },
          { text: 'Actions', value: '' },
        ],
        sellorders: []
      }
    },

    mounted(){
      this.setEosInstance()

      ScatterJS.scatter.connect('Gyftie').then(connected => {
        if(!connected){
          console.error('Could not connect to Scatter.')
          return
        }
        this.scatter = ScatterJS.scatter
      })

       rpc.get_table_rows({
            "json": true,
            "code": gftorderbook, //"gyftietoken1",   // contract who owns the table
            "scope": gftorderbook,  // scope of the table
            "table": "sellorders",    // name of the table as specified by the contract abi
            "limit": 100,
        }).then( result => {
            console.log (result.rows);
            this.sellorders = result.rows;
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
      async place_buy_order () {
        
        if(this.sending) return
        this.sending = true
        const options = { authorization:[`${this.account.name}@${this.account.authority}`] }

        const completed = res => {
          this.result = res
          this.sending = false
        }

        let order_value = Number.parseFloat(this.gft_amount * this.price_per_gft).toFixed(4) + ' EOS'
        console.log (order_value);

        try {
          const result = await eos.transact({
            actions: [
            {
              account: 'eosio.token',
              name: 'transfer',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                from: this.account.name,
                to: gftorderbook,
                quantity: Number.parseFloat(this.gft_amount * this.price_per_gft).toFixed(4) + ' EOS',
                memo: 'Transfer for GFT Buy Order'
              },
            },
            {
              account: gftorderbook,
              name: 'limitbuygft',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                buyer: this.account.name,
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
        const options = { authorization:[`${this.account.name}@${this.account.authority}`] }

        const completed = res => {
          this.result = res
          this.sending = false
        }
        try {
          const result = await eos.transact({
            actions: [
              {
              account: 'eosio.token',
              name: 'transfer',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                from: this.account.name,
                to: gftorderbook,
                quantity: order.order_value,
                memo: 'Transfer for GFT Buy Order'
              },
            },
              {
              account: gftorderbook,
              name: 'buygft',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                sellorder_id: order.order_id,
                buyer: this.account.name
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
        const options = { authorization:[`${this.account.name}@${this.account.authority}`] }

        const completed = res => {
          this.result = res
          this.sending = false
        }
        try {
          const result = await eos.transact({
            actions: [{
              account: gftorderbook,
              name: 'delsellorder',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                sellorder_id: order.order_id
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