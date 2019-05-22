<template>
	<view class="index">
		<uni-list>
			<block v-for="(item, index) in lists" :key="index">
				<uni-list-item :title="item.RoomName" :note="item.CreateDate" show-extra-icon="true"  @click="goDetail(item)" ></uni-list-item>
			</block>
			<!--<uni-list-item title="标题文字" note="描述信息" show-extra-icon="true" :extra-icon="{color: '#4cd964',size: '22',type: 'spinner'}"></uni-list-item>-->
		</uni-list>		
		<!--
		* 
			<view class="row">
				<view class="card card-list2"  @click="goDetail(item)" >
					<image class="card-img card-list2-img" :src="item.img_src"></image>
                    <text class="card-num-view card-list2-num-view">{{item.img_num}}P</text>
					<view class="card-bottm row">
						<view class="car-title-view row">
							<text class="card-title card-list2-title">{{item.NickName}}</text>
						</view>
						<view @click.stop="share(item)" class="card-share-view"></view>
					</view>
				</view>
			</view>
			-->
		
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
				roomid:"{2b7e7bfc-2730-49fe-ba43-a3e1043fcc13}"
				
			}
		},
        onShow: function() {
            console.log('medialist Show')
        },
        onHide: function() {
		
        },		

		onLoad(e) {
			this.fetchPageNum = 1;
			this.id = e.id;
			setTimeout(() => { //防止app里由于渲染导致转场动画卡顿
				this.getData();
			}, 150)			

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
			this.fetchPageNum = 1;
			//this.lists = [];
			this.getData();
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
			getData(e) {
				const data = {
					action:"medialist",
					roomid:this.roomid,
					orderby:"CreateDate desc",
					pageid:this.fetchPageNum
				}
				let that = this;
				uni.request({
					url: this.$serverUrl + '/api/1.00/private',
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
								uni.showToast({
									title: "请求失败",
									icon: "none",
								})							
						
						} else {
							if(ret.data.status != 0){
								uni.showToast({
									title:ret.data.msg,
								})
								return;
							}
							if (that.refreshing && ret.data.medialist[0].id === that.lists[0].id) {
								uni.showToast({
									title: "已经最新",
									icon: "none",
								})
								that.refreshing = false;
								uni.stopPullDownRefresh()
								return;
							}
							
							
							let mls = ret.data.medialist;
							console.log("list页面得到lists", mls);
							if (that.refreshing) {
								that.refreshing = false;
								uni.stopPullDownRefresh();
								that.lists = mls;
								that.fetchPageNum = 2;
								that.loadMoreText="下拉刷新";
								
							} else {
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
							}
							//that.fetchPageNum += 1;
						}
					}
				});
			},
			goDetail(e) {
				if(this.hasLogin == 1){
					const data ={
						cmd:"pulldlghisgp",
						data:e,
						ui:this.userInfo,
					}
					alert(JSON.stringify(data));
				}else{
					uni.showModal({
						title: '请先登录哦！',
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
