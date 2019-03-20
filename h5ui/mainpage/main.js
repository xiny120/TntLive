import Vue from 'vue'
import App from './App'

import store1 from './store'

Vue.config.productionTip = false

Vue.prototype.$store = store1

Vue.prototype.$serverUrl = 'https://unidemo.dcloud.net.cn';

App.mpType = 'app'


const app = new Vue({
	store1,
    ...App
})
app.$mount()