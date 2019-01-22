import Vue from 'vue'
import Router from 'vue-router'
import Home from './views/Home.vue'
import ViewBalance from './views/ViewBalance.vue'
import Gyft from './views/Gyft.vue'
import Issue from './views/Issue.vue'
import Transfer from './views/Transfer.vue'
import Propose from './views/Propose.vue'
import ListProposals from './views/ListProposals.vue'
import Collaborate from './views/Collaborate.vue'
import Vote from './views/Vote.vue'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      name: 'home',
      component: ViewBalance
    },
    {
      path: '/viewbal',
      name: 'viewbal',
      component: ViewBalance
    },
    {
      path: '/issue',
      name: 'issue',
      component: Issue
    },
    {
      path: '/transfer',
      name: 'transfer',
      component: Transfer
    },
    {
      path: '/gyft',
      name: 'gyft',
      component: Gyft
    },
    {
      path: '/propose',
      name: 'propose',
      component: Propose
    },
    {
      path: '/collaborate',
      name: 'collaborate',
      component: Collaborate
    },
    {
      path: '/listproposals',
      name: 'listproposals',
      component: ListProposals
    },
    {
      path: '/vote',
      name: 'vote',
      component: Vote
    }
  ]
})
