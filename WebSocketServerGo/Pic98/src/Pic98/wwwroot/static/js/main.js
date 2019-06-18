var a  = 0;
var Pic98 = {
	ui:{},
	init:function(){
		if($.cookie('token') == null){
			window.localStorage.removeItem("ui");
		}
		ui0 = window.localStorage.getItem("ui");
		if(this.isvalid(ui0)){
			this.ui = JSON.parse(ui0)
		}
	},
	islogin:function(){
		if (typeof(this.ui)=="undefined" || !this.ui){
			return false;
		}		
		if (typeof(this.ui.OnlineKey)=="undefined" || !this.ui.OnlineKey || this.ui.OnlineKey=="00000000-0000-0000-0000-000000000000"){
			return false;
		}
		return true;
	},
	isvalid:function(o){
		if (typeof(o)=="undefined" || o == null || o ==""){
			return false;
		}
		return true;		
	}
	
};

Pic98.init();

