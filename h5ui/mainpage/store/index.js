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

			const data = {
				action:"authout",
				//account: e.detail.value.nameValue,
				//password: cj.MD5(e.detail.value.passwordValue).toString().substring(8,24),
			}
			uni.request({
				//url: this.$serverUrl + '/api/1.00/auth', //仅为示例，并非真实接口地址。
				url: this.$serverUrl + '/api/1.00/private', //仅为示例，并非真实接口地址。
				method: 'POST',
				data:data,
				dataType:'json',  
				header:{  
					'content-type':'application/json',
					'mster-token':state.userInfo.SessionId,
				}, 
				success: (res) => {
						
					
				},

			});		
            state.userInfo = {};
            state.hasLogin = -1;
            uni.removeStorage({  
                key: 'userinfo'  
            });											
											
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
