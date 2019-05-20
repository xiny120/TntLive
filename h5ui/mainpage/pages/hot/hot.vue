<template>
	<view class="index">
		<block v-for="(list, index) in lists" :key="index">
			<view class="row">
				<view class="card card-list2" v-for="(item,key) in list" @click="goDetail(item)" :key="key">
					<image class="card-img card-list2-img" :src="item.corver"></image>
					<view class="card-bottm row">
						<view class="car-title-view column">
							<text class="card-title ">{{item.title}}</text>
							<text class="card-title card-list2-title">关注:{{item.follow}} 在线:{{item.onlines}}</text>
						</view>
						<view @click.stop="share(item)" class="card-share-view"></view>
					</view>
				</view>
			</view>
		</block>
		<text v-if="refreshing" class="loadMore">加载中...</text>
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
				loading:false,
				refreshing: false,
				lists: [],
				fetchPageNum: 1
			}
		},
		onLoad() {
			this.getData();
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
			this.getData();
		},
		onReachBottom() {
			this.getData();
		},
		computed: mapState(['forcedLogin','hasLogin','userInfo','roomid']),
		methods: {
			...mapMutations(['setroomid',]),
			getData() {
				this.loading = true;
				const data = {
					action:"roomlist",
					page:(this.refreshing ? 1 : this.fetchPageNum),
					per_page:10,
				}			
				
				uni.request({
					url: this.$serverUrl + '/api/1.00/public',//?page=' + (this.refreshing ? 1 : this.fetchPageNum) + '&per_page=10',
					method: 'POST',
					data:data,
					dataType:'json',  
					header:{  
						'content-type':'application/json',
						//'mster-token':this.userInfo.SessionId,
					}, 
					fail:(ret)=>{
						console.log(ret);
						this.loading = false;
					},
					success: (ret) => {
						if (ret.statusCode !== 200) {
							console.log("请求失败:", ret)
						} else {
							if (this.refreshing && ret.data[0].id === this.lists[0][0].id) {
								uni.showToast({
									title: "已经最新",
									icon: "none",
								})
								this.refreshing = false;
								uni.stopPullDownRefresh()
								return;
							}
							let list = [],
								lists = [],
								data = ret.data;
							for (let i = 0, length = data.length; i < length; i++) {
								let index = Math.floor(i / 2);
								list.push(data[i]);
								if (i % 2 == 1) {
									lists.push(list);
									list = [];
								}
							}
							console.log("得到lists", lists);
							if (this.refreshing) {
								this.refreshing = false;
								uni.stopPullDownRefresh()
								this.lists = lists;
								this.fetchPageNum = 2;
							} else {
								this.lists = this.lists.concat(lists);
								this.fetchPageNum += 1;
							}
						}
						this.loading = false;
					}
				});
			},
			goDetail(e) {
				console.log(e)
				if(e.type == 1){
					uni.switchTab({
						url:"../tag/tag"
					})
				}else{
					if(this.hasLogin == 1){
						this.setroomid(e.id);
						if(e.id.toLowerCase()  == "{96518478-be8d-4eee-9fec-69d472ced4dc}"){
							uni.switchTab({
								url:"../medialist117/medialist117"
							})							
						}else{
							const data ={
								cmd:"pulldlg",
								data:e,
								ui:this.userInfo,
							}
							alert(JSON.stringify(data));
						}
					}else{
						uni.showModal({
							title: '请先登录哦！',
							//content: '确定切换账户吗？',
							success: function (res) {
								if (res.confirm) {				
									
									uni.navigateTo({
										url:"../login/login"
									})
								} else if (res.cancel) {
									console.log('用户点击取消');
								}
							}
						});	
					}
					//uni.navigateTo({
					//	url:"../detail/detail?data=" + encodeURIComponent(JSON.stringify(e))
					//})
				}
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
							scene: this.providerList[res.tapIndex].type && this.providerList[res.tapIndex].type === 'WXSenceTimeline' ? 'WXSenceTimeline' : "WXSceneSession",
							type: 0,
							title:"uni-app模版",
							summary: e.title,
							imageUrl:e.img_src,
							href:"https://uniapp.dcloud.io",
							success: (res) => {
								console.log("success:" + JSON.stringify(res));
							},
							fail: (e) => {
								uni.showModal({
									content: e.errMsg,
									showCancel:false
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
	.index{
		justify-content:flex-start;
	}
</style>
