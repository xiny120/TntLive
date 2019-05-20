<script>
    import {
        mapState,
        mapMutations
    } from 'vuex'
	
	
		
    export default {
		data() {
			return {
				websock: null,
				wsuriidx:0,
				websockopened:false,
				
			};
		},		
		
		
		
        onLaunch: function() {
			let that = this;
			
		uni.getStorage({//获得保存在本地的用户信息  
			key: 'userinfo',
			success:(res) => {  
				that.login(res.data);  
			}  
		});  			
			
			this.websockopened = false;
			this.wsuriidx = 0;
			//this.sign_up_status = -1;
			//this.initWebSocket();
            console.log('App Launch')
        },
        onShow: function() {
            console.log('App Show')
        },
        onHide: function() {
            console.log('App Hide')
        },

		computed: mapState(['signupStatus']),
		mounted() {
					
				},		
　　　　methods: { 
			...mapMutations(['register00','login','loginFail','setroomid']),
　　　}, 		
		
    }
</script>

<style>
	@import './common/uni.css';
    @import './common/common.css';

    page,
    view {
        display: flex;
    }

    page {
        display: flex;
        min-height: 100%;
        background-color: #EFEFEF;
    }

    template {
        display: flex;
        flex: 1;
    }
</style>
