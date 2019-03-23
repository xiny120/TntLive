import Vue from 'vue'
import Vuex from 'vuex'

Vue.use(Vuex)

const store = new Vuex.Store({
    state: {
		signupStatus:-1,
        /**
         * 是否需要强制登录
         */
        forcedLogin: false,
        hasLogin: -1,
        userName: "",
		userInfo:{}
    },
    mutations: {
        login(state, ui) {
            //state.userName = userName || '新用户';
			state.userInfo = ui;
			console.log(JSON.stringify(ui));
            state.hasLogin = 1;
			uni.setStorage({key: 'userinfo', data: ui});			
        },
        logout(state) {
            state.userInfo = {};
            state.hasLogin = -1;
            uni.removeStorage({  
                key: 'userinfo'  
            })  			
        },
		loginFail(state){
			state.userInfo = {};
			state.hasLogin = 0;
		},
		register00(state, signupStatus){
			state.signupStatus = signupStatus || -1;
		}
    }
})

export default store
