<template>
	<view class="center">
		<view class="logo" @click="goLogin" :hover-class="!(hasLogin==1) ? 'logo-hover' : ''">
			<image class="logo-img" :src="hasLogin == 1 ? userInfo.avatarUrl :avatarUrl"></image>
			<view class="logo-title">
				<text class="uer-name">Hi，{{hasLogin ==1 ? userInfo.UserName : '您未登录'}}</text>
				<text class="go-login navigat-arrow" v-if="!hasLogin">&#xe65e;</text>
			</view>
		</view>
		
		<view class="center-list">
			<view class="center-list-item border-bottom" @click="goSetup">
				<text class="list-icon">&#xe609;</text>
				<text class="list-text">修改密码</text>
				<text class="navigat-arrow">&#xe65e;</text>
			</view>	
			<view class="center-list-item border-bottom" @click="goSwitch">
				<text class="list-icon">&#xe609;</text>
				<text class="list-text">切换帐号</text>
				<text class="navigat-arrow">&#xe65e;</text>
			</view>						
			<view class="center-list-item border-bottom" @click="goAbout">
				<text class="list-icon">&#xe603;</text>
				<text class="list-text">关于</text>
				<text class="navigat-arrow">&#xe65e;</text>
			</view>

		</view>
	</view>
</template>

<script>
    import {
        mapState,
        mapMutations
    } from 'vuex'	
	export default {
		data() {
			return {
				login:false,
				avatarUrl:"../../static/logo.png",
				uerInfo:{}
			}
		},
		computed: mapState(['forcedLogin','hasLogin','userInfo']),
		watch: {

		},
		methods: {
			...mapMutations(['logout']),
			goLogin() {
				if(this.hasLogin!=1){
					uni.navigateTo({
						url:"../login/login"
					})
				}
			},
			goSwitch() {
				var that = this;
				uni.showModal({
					title: '确定切换账户吗？',
					//content: '确定切换账户吗？',
					success: function (res) {
						if (res.confirm) {				
							that.logout();
							uni.navigateTo({
								url:"../login/login"
							})
						} else if (res.cancel) {
							console.log('用户点击取消');
						}
					}
				});								
				
			},			
			goSetup() {
				if(this.hasLogin==1){
					uni.navigateTo({
						url:"../center/setup/setup"
					})
				}
			},			
			goAbout() {
				// #ifdef APP-PLUS
				uni.navigateTo({
					url:'/platforms/app-plus/about/about'
				});
				// #endif
				// #ifdef H5
				uni.navigateTo({
					url:'/platforms/h5/about/about'
				});
				// #endif
			}
		}
	}
</script>

<style>
</style>
