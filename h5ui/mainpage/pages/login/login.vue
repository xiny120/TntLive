<template>
	<form class='loginView' @submit="formSubmit" @reset="formReset">
		<view class="input-view">
			<view class="label-view">
				<text class="label">账号 </text>
			</view>
			<input class="input" type="text" placeholder="请输入用户名" name="nameValue" />
		</view>
		<view class="input-view">
			<view class="label-view">
				<text class="label">密码</text>
			</view>
			<input class="input" type="password" placeholder="请输入密码" name="passwordValue" />
		</view>
		<view class="button-view">
			<button type="default" class="login" hover-class="hover" formType="submit">登录</button>
		</view>
        <view class="action-row">
            <navigator url="../login/reg/reg" v-show="0">注册账号</navigator>
            <text  v-show="0">|</text>
            <navigator url="../login/pwd/pwd">忘记密码</navigator>
        </view>
        <view class="oauth-row" v-if="hasProvider" v-bind:style="{top: positionTop + 'px'}">
            <view class="oauth-image" v-for="provider in providerList" :key="provider.value">
                <image :src="provider.image" @tap="oauth(provider.value)"></image>
            </view>
        </view>		
		
	</form>
</template>

<script>
	import cj from '../../node_modules/crypto-js/crypto-js.js'
	
	var  graceChecker = require("../../common/graceChecker.js");
	// /帐号/手机号码
    import service from '../../service.js';
    import {
        mapState,
        mapMutations
    } from 'vuex'
    import mInput from '../../components/m-input.vue'	
	
	
	export default {
        components: {
            mInput
        },		
		data() {
			return {
                providerList: [],
                hasProvider: false,
                account: '',
                password: '',
                positionTop: 0				
			};
		},
		computed: mapState(['forcedLogin','hasLogin']),
		watch: {
			"hasLogin":{
				handler(){
					switch(this.hasLogin){
						case 1:

							
						break;
						case 0:
							//uni.showToast({
							//	title: '登录失败，请重试',
							//	complete: function(res){
							//		//this.loginFail();
							//
							//	}
							//});						
						break;
						case -1:
						break;
						default:
					}
				}
			},
			"userName":{
				handler(){
					uni.showToast({
						title:"userName"
					})
				}
				
			}
		},
		
		methods: {
			...mapMutations(['login','loginFail','register00','logout']),
            initProvider() {
                const filters = ['weixin', 'qq', 'sinaweibo'];
				
                uni.getProvider({
                    service: 'oauth',
                    success: (res) => {
                        if (res.provider && res.provider.length) {
                            for (let i = 0; i < res.provider.length; i++) {
                                if (~filters.indexOf(res.provider[i])) {
                                    this.providerList.push({
                                        value: res.provider[i],
                                        image: '../../static/img/' + res.provider[i] + '.png',
										
                                    });
                                }
                            }
                            this.hasProvider = true;
                        }
                    },
                    fail: (err) => {
                        console.error('获取服务供应商失败：' + JSON.stringify(err));
                    }
                });
            },
            initPosition() {
                /**
                 * 使用 absolute 定位，并且设置 bottom 值进行定位。软键盘弹出时，底部会因为窗口变化而被顶上来。
                 * 反向使用 top 进行定位，可以避免此问题。
                 */
                this.positionTop = uni.getSystemInfoSync().windowHeight - 100;
            },			
			formSubmit: function (e) {
				//getApp().websocketsend("{\"t\":\"login hello\"}");
				//将下列代码加入到对应的检查位置
				//定义表单规则
				var rule = [
					{name:"nameValue", checkType : "string", checkRule:"1,30",  errorMsg:"请输入帐号！"},
					{name:"passwordValue", checkType : "string", checkRule:"1,30",  errorMsg:"请输入密码"},
				];
				//进行表单检查
				var formData = e.detail.value;
				var checkRes = graceChecker.check(formData, rule);
				if(checkRes){
					//uni.showToast({title:"验证通过!", icon:"none"});
					//console.log("得到账号:"+ e.detail.value.nameValue + ';得到密码:' + e.detail.value.passwordValue)
					this.logout();
					const data = {
						action:"auth",
						account: e.detail.value.nameValue,
						password: cj.MD5(e.detail.value.passwordValue).toString().substring(8,24),
					}		
								
					this.loginFail();
					
					uni.showToast({
						title: '正在登录，请稍后...',
					});						
					
					uni.request({
						//url: this.$serverUrl + '/api/1.00/auth', //仅为示例，并非真实接口地址。
						url: this.$serverUrl + '/api/1.00/public', //仅为示例，并非真实接口地址。
						method: 'POST',
						data:data,
						dataType:'json',  
						header:{  
							'content-type':'application/json'  
						}, 
						success: (res) => {
							if(res.data.status == 0){
								if(res.data.userinfo.UserId != 0){
									this.login(res.data.userinfo)
									uni.showToast({
										title: '登录成功，准备跳转...',
									});						
									
									setTimeout(() => {
										uni.navigateBack({
												delta: 1
										});						  
									}, 200)								
								}else{
									uni.showToast({
										title: res.data.userinfo.Info,
									});									
								}		
							}else{
								uni.showToast({
									title: res.data.msg,
								});	
							}
							
						},
						fail:(res) =>{
							uni.showToast({
								title: '登录失败，请重试。',
								complete: function(res){
									//this.loginFail();
							
								}
							});								
						},
					});					
					
					
					
					//getApp().websocketsend(JSON.stringify(data))
					
				}else{
					uni.showToast({ title: graceChecker.error, icon: "none" });
				}
			},
			formReset: function (e) {
				console.log("清空数据")
				this.chosen = ''
			},
            oauth(value) {
                uni.login({
                    provider: value,
                    success: (res) => {
                        uni.getUserInfo({
                            provider: value,
                            success: (infoRes) => {
                                /**
                                 * 实际开发中，获取用户信息后，需要将信息上报至服务端。
                                 * 服务端可以用 userInfo.openId 作为用户的唯一标识新增或绑定用户信息。
                                 */
                                this.toMain(infoRes.userInfo.nickName);
                            }
                        });
                    },
                    fail: (err) => {
                        console.error('授权登录失败：' + JSON.stringify(err));
                    }
                });
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
            this.initProvider();
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

    .oauth-row {
        display: flex;
        flex-direction: row;
        justify-content: center;
        position: absolute;
        top: 0;
        left: 0;
        width: 100%;
    }

    .oauth-image {
        width: 100upx;
        height: 100upx;
        border: 1upx solid #dddddd;
        border-radius: 100upx;
        margin: 0 40upx;
        background-color: #ffffff;
    }

    .oauth-image image {
        width: 60upx;
        height: 60upx;
        margin: 20upx;
    }
</style>