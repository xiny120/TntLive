<template>
	<view class="center">
		<view class="logo" @click="goLogin" :hover-class="!hasLogin ? 'logo-hover' : ''">
			<image class="logo-img" :src="hasLogin ? userInfo.avatarUrl :avatarUrl"></image>
			<view class="logo-title">
				<text class="uer-name">Hi，{{hasLogin ? userInfo.UserName : '您未登录'}}</text>
				<text class="go-login navigat-arrow" v-if="!hasLogin">&#xe65e;</text>
			</view>
		</view>
		<view class="center-list">
			<view class="center-list-item border-bottom">
				<text class="list-icon">&#xe60c;</text>
				<text class="list-text">收藏图片</text>
				<text class="navigat-arrow">&#xe65e;</text>
			</view>
			<view class="center-list-item">
				<text class="list-icon">&#xe60d;</text>
				<text class="list-text">收藏图集</text>
				<text class="navigat-arrow">&#xe65e;</text>
			</view>
		</view>
		<view class="center-list">
			<view class="center-list-item border-bottom">
				<text class="list-icon">&#xe60b;</text>
				<text class="list-text">管理图片</text>
				<text class="navigat-arrow">&#xe65e;</text>
			</view>
			<view class="center-list-item">
				<text class="list-icon">&#xe61a;</text>
				<text class="list-text">上传图片</text>
				<text class="navigat-arrow">&#xe65e;</text>
			</view>
		</view>
		<view class="center-list">
			<view class="center-list-item border-bottom" @click="goAbout">
				<text class="list-icon">&#xe603;</text>
				<text class="list-text">关于</text>
				<text class="navigat-arrow">&#xe65e;</text>
			</view>
			<view class="center-list-item">
				<text class="list-icon">&#xe609;</text>
				<text class="list-text">帐号管理</text>
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
			goLogin() {
				if(!this.hasLogin){
					uni.navigateTo({
						url:"../login/login"
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
