<template>
   
<div>
    <h2>Create New Monetary Policy Change Proposal</h2>
    <v-flex >
                      
          <v-textarea
            v-model="explanation"
            label="Proposal Explanation"
            hint="Type your proposal in clear language"
            required
          ></v-textarea>
    
    </v-flex>
    
    <br/>

     <v-btn color="primary" @click.prevent="propose">Propose</v-btn>

    <br/>
    <br/>
    <br/>
    <h2>Vote on Open Proposals</h2>
     <v-data-table
    :headers="headers"
    :items="proposals"
    item-key="proposal_id">

    <template slot="items" slot-scope="props">
      <!-- <td>{{ props.item.proposal_id }}</td> -->
      <td class="text-xs-right">{{ props.item.proposer }}</td>
      <td class="text-xs-left">{{ props.item.notes }}</td>
      <td class="text-xs-right">{{ props.item.votes_for }}</td>
      <td class="text-xs-right">{{ props.item.votes_against }}</td>

         <td >
          <v-icon
            @click="vote_up(props.item)"
          >
            thumb_up_alt
          </v-icon>
          <v-icon
            @click="vote_down(props.item)"
          >
            thumb_down_alt
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
  import { Api, JsonRpc} from 'eosjs'
  import { network_config, gyftiecontract } from '../config';

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
        proposer: "",
        explanation: "",
        headers: [
          { text: 'Proposer', value: 'proposer' },
          { text: 'Notes', value: 'notes' },
          { text: 'Votes For', value: 'votes_for' },
          { text: 'Votes Against', value: 'votes_against' },
          { text: 'Vote', value: '', sortable: false },
        ],
        proposals: []
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
            "code": gyftiecontract, 
            "scope": gyftiecontract,  
            "table": "proposals",    
            "limit": 100,
        }).then( result => {
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
      async propose () {
        
        if(this.sending) return
        this.sending = true
      
        const completed = res => {
          this.result = res
          this.sending = false
        }

        try {
          const result = await eos.transact({
            actions: [{
              account: gyftiecontract,
              name: 'propose',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                proposer: this.account.name,
                notes: this.explanation
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
    async vote_up (proposal) {
        if(this.sending) return
        this.sending = true
   
        const completed = res => {
          this.result = res
          this.sending = false
        }

        try {
          const result = await eos.transact({
            actions: [{
              account: gyftiecontract,
              name: 'votefor',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                voter: this.account.name,
                proposal_id: proposal.proposal_id
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
    async vote_down(proposal) {
        if(this.sending) return
        this.sending = true
     
        const completed = res => {
          this.result = res
          this.sending = false
        }

        try {
          const result = await eos.transact({
            actions: [{
              account: gyftiecontract,
              name: 'voteagainst',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                voter: this.account.name,
                proposal_id: proposal.proposal_id
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