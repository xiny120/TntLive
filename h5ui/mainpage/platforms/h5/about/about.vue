<template>
	<view class="about">
		<view class="content">
			<view class="qrcode">
				<text class="tip">课件直播系统（ver 1.0.0）</text>
			</view>
		</view>
	</view>
</template>

<script>
	export default {
		data() {
			return {
				sourceLink: 'https://github.com/dcloudio/uni-template-picture'
			};
		},
		methods:{
			openLink() {
				window.open(this.sourceLink);
			}
		}
	}
</script>

<style>
	page,
	view {
		display: flex;
	}

	page {
		min-height: 100%;
		background-color: #FFFFFF;
	}

	image {
		width: 360upx;
		height: 360upx;
	}

	.about {
		flex-direction: column;
		flex: 1;
	}

	.content {
		flex: 1;
		padding: 30upx;
		flex-direction: column;
		justify-content: center;
	}

	.qrcode {
		display: flex;
		align-items: center;
		flex-direction: column;
	}

	.qrcode .tip {
		margin-top: 20upx;
	}

	.desc {
		margin-top: 30upx;
		display: block;
	}

	.code {
		color: #e96900;
		background-color: #f8f8f8;
	}
	
	.source {
		margin-top: 30upx;
		flex-direction: column;
	}
	
	.source-list {
		flex-direction: column;
	}
	.link {
		color: #007AFF;
	}
</style>
