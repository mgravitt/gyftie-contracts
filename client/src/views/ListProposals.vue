<template>
<div>
    <h2>Current Proposals</h2>

<v-data-table
    :headers="headers"
    :items="proposals"
    item-key="proposal_id"
    
  >
    <!-- <template slot="headerCell" slot-scope="props">
      <v-tooltip bottom>
        <span slot="activator">
          {{ props.header.text }}
        </span>
        <span>
          {{ props.header.text }}
        </span>
      </v-tooltip>
    </template> -->
    <template slot="items" slot-scope="props">
     <tr @click="props.expanded = !props.expanded">
      <td>{{ props.item.proposal_id }}</td>
      <td class="text-xs-right">{{ props.item.proposer }}</td>
      <!-- <td class="text-xs-right">{{ props.item.new_token_gen }}</td> -->
      <td class="text-xs-right">{{ props.item.votes_for }}</td>
      <td class="text-xs-right">{{ new Date(props.item.created_date*1000).toISOString().split('T')[0]}}</td>
      </tr>
    </template>
     <template slot="expand" slot-scope="props">
   
            Voters                  : {{props.item.voters}} <br/>
            Notes                : {{props.item.notes}} <br/>
            Expiration            : {{ new Date(props.item.expiration_date*1000).toISOString().split('T')[0]}} <br/>
      
    </template>
  </v-data-table>

  <br/><br/>
     <v-btn color="primary" @click.prevent="issueToken">Save Profile</v-btn><br/>
     <v-btn @click="login" v-if="scatter && !account">Login with Scatter</v-btn>
     <section v-if="scatter && account">
          Logged in with: {{account.name}}
          <v-btn @click="logout">Logout</v-btn>
     </section>
</div>
</template>

<script>
  import ScatterJS, {Network} from 'scatterjs-core'
  import ScatterEOS from 'scatterjs-plugin-eosjs2'
  import { Api, JsonRpc, RpcError, JsSignatureProvider } from 'eosjs'

  ScatterJS.plugins( new ScatterEOS() )

  const network = Network.fromJson({
    blockchain:'eos',
    host:'jungle.eosio.cr',
    port:443,
    protocol:'https',
    chainId:'e70aaab8997e1dfce58fbfac80cbbb8fecec7b99cf982a9444273cbc64c41473' 
  })

  let eos
  const rpc = new JsonRpc(network.fullhost())
  export default {
  
    data () {
      return {
        sending:false,
        scatter:null,
        result:null,
        headers: [
          {
            text: 'Proposal ID',
            align: 'left',
            sortable: false,
            value: 'proposal_id'
          },
          { text: 'Proposer', value: 'proposer' },
          { text: 'New Contract', value: 'new_token_gen' },
          { text: 'Votes', value: 'votes_for' },
          { text: 'Created Date', value: 'created_date' },
        ],
        bids: []
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
            "code": "gyftietoken1",   // contract who owns the table
            "scope": "gyftietoken1",  // scope of the table
            "table": "proposals",    // name of the table as specified by the contract abi
            "limit": 100,
        }).then( result => {
            console.log (result.rows);
            this.proposals = result.rows;
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