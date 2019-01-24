import Vue from 'vue'
import Router from 'vue-router'
import ViewBalance from './views/ViewBalance.vue'
import Gyft from './views/Gyft.vue'
import Transfer from './views/Transfer.vue'
import Collaborate from './views/Collaborate.vue'
import BuyGFT from './views/BuyGFT.vue'
import SellGFT from './views/SellGFT.vue'

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
      path: '/buygft',
      name: 'buygft',
      component: BuyGFT
    },
    {
      path: '/sellgft',
      name: 'sellgft',
      component: SellGFT
    },
    {
      path: '/collaborate',
      name: 'collaborate',
      component: Collaborate
    }
  ]
})
