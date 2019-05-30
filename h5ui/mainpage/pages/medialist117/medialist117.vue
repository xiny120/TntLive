<template>
	<view class="index">
		<uni-list>
			<block v-for="(item, index) in lists" :key="index">
				<uni-list-item :title="item.NickName" :note="item.CreateDate" show-extra-icon="true"  @click="goDetail(item)" ></uni-list-item>
			</block>
		</uni-list>		
		<text class="loadMore">{{loadMoreText}}</text>
	</view>
</template>

<script>
	import uniList from '../../components/uni-list/uni-list.vue'
	import uniListItem from '../../components/uni-list-item/uni-list-item.vue'	
    import {
        mapState,
        mapMutations
    } from 'vuex'
		
	export default {
		components: {
			uniList,
			uniListItem
		},		
		data() {
			return {
				refreshing: false,
				loadMoreText:"",
				lists: [],
				id: 0,
				fetchPageNum: 1,
				roomid:"{96518478-be8d-4eee-9fec-69d472ced4dc}"
				
			}
		},
		onLoad(e) {
			this.id = e.id;
			setTimeout(() => { //防止app里由于渲染导致转场动画卡顿
				this.getData();
			}, 50)			
			uni.getProvider({
				service: "share",
				success: (e) => {
					let data = [];
					for (let i = 0; i < e.provider.length; i++) {
						switch (e.provider[i]) {
							case 'weixin':
								data.push({
									name: '分享到微信好友',
									id: 'weixin'
								})
								data.push({
									name: '分享到微信朋友圈',
									id: 'weixin',
									type: 'WXSenceTimeline'
								})
								break;
							case 'qq':
								data.push({
									name: '分享到QQ',
									id: 'qq'
								})
								break;
							default:
								break;
						}
					}
					this.providerList = data;
				},
				fail: (e) => {
					console.log("获取登录通道失败", e);
				}
			});
		},
		onPullDownRefresh() {
			console.log("下拉刷新");
			this.refreshing = true;
			//this.getNew();
		},
		onReachBottom() {
			console.log("上拉加载刷新");
			if(this.fetchPageNum > 30){
				this.loadMoreText = "没有更多了"
				return;
			}else{
				uni.showToast({
					title: "加载中...",
					icon: "none",
				})				
			}
			this.getData();
		},
		computed: mapState(['userInfo','hasLogin']),
		methods: {					
			getData(e) {
				if(this.refreshing) return;
				this.refreshing = true;
				let now0 = new Date();
				now0.setTime(now0.getTime() + 60*60*24*1000);
				let cd = now0.getFullYear() + "-" + (now0.getMonth()+1) + "-" + now0.getDate();
				if(this.lists.length > 0){
					cd = this.lists[this.lists.length-1].CreateDate
				}					
				const data = {
					action:"medialist",
					orderby:"CreateDate desc",
					roomid:this.roomid,
					pageid:this.fetchPageNum,
					CreateDate:cd
				}
				let that = this;
				uni.request({
					url: this.$serverUrl + '/api/1.00/public',
					method: 'POST',
					data:data,
					dataType:'json',  
					header:{  
						'content-type':'application/json',
						'mster-token':this.userInfo.SessionId,
					}, 					
					success: (ret) => {
						if (ret.statusCode !== 200) {
							console.log("请求失败", ret)
							that.refreshing = false;
						} else {
							if(ret.data.status != 0){
								uni.showToast({
									title:ret.data.msg,
								})
								that.refreshing = false;
								return;
							}
							let lists = ret.data.medialist;
							if(lists.length == 0){
								uni.showToast({
									title: "没有更多了！",
									icon: "none",
								})								
							}
							console.log("list页面得到lists", lists);
							that.lists = that.lists.concat(lists);
							that.fetchPageNum += 1;
							that.refreshing = false;

						}
					}
				});
			},
			goDetail(e) {
				const data ={
					cmd:"pulldlghis117",
					data:e,
					ui:this.userInfo,
				}
				alert(JSON.stringify(data));
			},
			share(e) {
				if (this.providerList.length === 0) {
					uni.showModal({
						title: "当前环境无分享渠道!",
						showCancel: false
					})
					return;
				}
				let itemList = this.providerList.map(function (value) {
					return value.name
				})
				uni.showActionSheet({
					itemList: itemList,
					success: (res) => {
						uni.share({
							provider: this.providerList[res.tapIndex].id,
							scene: this.providerList[res.tapIndex].type && this.providerList[res.tapIndex].type === 'WXSenceTimeline' ?
								'WXSenceTimeline' : "WXSceneSession",
							type: 0,
							title: "uni-app模版",
							summary: e.title,
							imageUrl: e.img_src,
							href: "https://uniapp.dcloud.io",
							success: (res) => {
								console.log("success:" + JSON.stringify(res));
							},
							fail: (e) => {
								uni.showModal({
									content: e.errMsg,
									showCancel: false
								})
							}
						});
					}
				})
			}
		}
	}
</script>

<style>

</style>
