<template>
	<view class="index">
		<uni-list>
			<block v-for="(item, index) in lists" :key="index">
				<uni-list-item :title="item.stitle" :note="item.createdate" show-extra-icon="true"  @click="goDetail(item)" ></uni-list-item>
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
			}
		},
		onLoad(e) {
			this.id = e.roomid;
			console.log("shitshitshitshitshit"+e.roomid);
			setTimeout(() => { //防止app里由于渲染导致转场动画卡顿
				this.getData();
			}, 150)			
		},
		onPullDownRefresh() {
			console.log("下拉刷新");
			this.getNew()();
		},
		onReachBottom() {
			console.log("上拉加载刷新");
			if(this.fetchPageNum > 20){
				this.loadMoreText = "没有更多了"
				return;
			}
			this.getData();
		},
		computed: mapState(['userInfo','hasLogin']),
		methods: {
			getNew(e) {
				if(this.refreshing) return;
				this.refreshing = true;
				let now0 = new Date();
				let cd = now0.getFullYear() + "-" + (now0.getMonth()+1) + "-" + now0.getDate();
				if(this.lists.length > 0){
					cd = this.lists[0].createdate
				}
				const data = {
					action:"medialistnew",
					roomid:this.id,
					orderby:"CreateDate desc",
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
							uni.showToast({
								title: "请求失败",
								icon: "none",
							})
							that.refreshing = false;
							uni.stopPullDownRefresh();							
						} else {
							if(ret.data.status != 0){
								uni.showToast({
									title:ret.data.msg,
								})
								uni.stopPullDownRefresh();
								that.refreshing = false;
								return;
							}
							let mls = ret.data.medialist;
							that.lists = mls.concat(that.lists);//that.lists.concat(mls);
							uni.stopPullDownRefresh();
							that.refreshing = false;
						}
					}
				});
			},			
			getData(e) {
				if(this.refreshing) return;
				this.refreshing = true;
				let now0 = new Date();
				now0.setTime(now0.getTime() + 60*60*24*1000);
				let cd = now0.getFullYear() + "-" + (now0.getMonth()+1) + "-" + now0.getDate();
				if(this.lists.length > 0){
					cd = this.lists[this.lists.length-1].createdate
				}				
				const data = {
					action:"medialist",
					roomid:this.id,
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
							uni.showToast({
								title: "请求失败",
								icon: "none",
							})
							that.refreshing = false;
						} else {
							if(ret.data.status != 0){
								uni.showToast({
									title:ret.data.msg,
								})
								that.refreshing = false;
								return;
							}
							let mls = ret.data.medialist;
							console.log("list页面得到lists", mls);
							if(mls.length <= 0){
								uni.showToast({
									title: '没有更多数据了！',
									mask: false,
									duration: 1500
								});
							}else{								
								that.lists = that.lists.concat(mls);
								that.fetchPageNum += 1;
							}
							that.refreshing = false;
						}
					}
				});
			},
			goDetail(e) {
				//uni.navigateTo({
				//	url: "../detail/detail?data=" + encodeURIComponent(JSON.stringify(e))
				//})
				if(this.hasLogin == 1){
					const data ={
						cmd:"pulldlghis",
						data:e,
						ui:this.userInfo,
					}
					alert(JSON.stringify(data));
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
