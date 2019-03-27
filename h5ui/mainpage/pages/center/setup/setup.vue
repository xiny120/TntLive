<template>
	<form class='loginView' @submit="formSubmit" @reset="formReset">
		<view class="input-view">
			<text class="title">当前账号：</text>
			<input type="text" disabled="true" v-model="account" placeholder="请填写新账号" readonly="readonly"></m-input>
		</view>
		<view class="input-view">
			<text class="title">当前密码：</text>
			<m-input type="password" focus displayable v-model="password" placeholder="请填写当前密码"></m-input>
		</view>
		<view class="input-view">
			<text class="title">新密码&emsp;：</text>
			<m-input type="password" displayable v-model="password1" placeholder="请填写新密码"></m-input>
		</view>
		<view class="input-view">
			<text class="title">新密码&emsp;：</text>
			<m-input type="password" displayable v-model="password2" placeholder="请再填写一遍新密码"></m-input>
		</view>				
		<view class="button-view">
			<button type="default" class="login" hover-class="hover" formType="submit">修改密码</button>
		</view>
		
	</form>
</template>

<script>
	import cj from '../../../node_modules/crypto-js/crypto-js.js'
    import mInput from '../../../components/m-input.vue'		
    import service from '../../../service.js';
    import {
        mapState,
        mapMutations
    } from 'vuex'

	
	
	export default {
        components: {
            mInput
        },		
		data() {
			return {
                account: '',
                password: '',
                positionTop: 0				
			};
		},
		computed: mapState(['forcedLogin','hasLogin','userInfo']),
		methods: {
			...mapMutations(['login','loginFail','register00','logout']),

            initPosition() {
                this.positionTop = uni.getSystemInfoSync().windowHeight - 100;
            },			
			formSubmit: function (e) {
                if (this.password.length < 6) {
                    uni.showToast({
                        icon: 'none',
                        title: '密码最短为 6 个字符'
                    });
                    return;
                }
				
                if (this.password1.length < 6) {
                    uni.showToast({
                        icon: 'none',
                        title: '密码最短为 6 个字符'
                    });
                    return;
                }				
				
				if(this.password1 != this.password2){
                    uni.showToast({
                        icon: 'none',
                        title: '两次填写的新密码不一致，请重新输入！'
                    });
                    return;
				}
				
			
				//uni.showToast({title:"验证通过!", icon:"none"});
				//console.log("得到账号:"+ e.detail.value.nameValue + ';得到密码:' + e.detail.value.passwordValue)
				//this.logout();
				const data = {
					action:"modipassword",
					account: this.account,
					password: cj.MD5(this.password).toString().substring(8,24),
					password1: cj.MD5(this.password1).toString().substring(8,24),
				}		
				// 	
				uni.showToast({
					icon: 'none',
					title: '提交密码修改申请...'
				});				
				uni.request({
					url: this.$serverUrl + '/api/1.00/private', //仅为示例，并非真实接口地址。
					method: 'POST',
					data:data,
					dataType:'json',  
					header:{  
						'content-type':'application/json',
						'mster-token':this.userInfo.SessionId,
					}, 
					success: (res) => {
						console.log(res);
						if(res.data.status == 0){
							uni.showToast({
								title: res.data.msg,
							});	
							this.logout();
							setTimeout(() => {
								uni.navigateBack({
										delta: 1
								});						  
							}, 200)							
						}else{
							uni.showToast({
								title: res.data.msg,
							});								
						}
						
					},
					fail:(res) =>{
						uni.showToast({
							title: '密码修改失败，请重试。',
							complete: function(res){
								//this.loginFail();
						
							}
						});								
					},
				});					
				
				

			},
			formReset: function (e) {
				console.log("清空数据")
				this.chosen = ''
			},

            toMain(userName) {
                this.login(userName);
                /**
                 * 强制登录时使用reLaunch方式跳转过来
                 * 返回首页也使用reLaunch方式
                 */
                if (this.forcedLogin) {
                    uni.reLaunch({
                        url: '../main/main',
                    });
                } else {
                    uni.navigateBack();
                }

            }			
		},
        onLoad() {
            this.initPosition();
			this.account = this.userInfo.UserName;
        }		
	}
</script>

<style>
    .action-row {
        display: flex;
        flex-direction: row;
        justify-content: center;
    }

    .action-row navigator {
        color: #007aff;
        padding: 0 20upx;
    }

</style>
