var a  = 0;
var Pic98 = {
	ui:{},
	init:function(){
		if($.cookie('token') == null){
			window.localStorage.removeItem("ui");
		}
		ui = window.localStorage.getItem("ui");
		if(ui != null){
			ui = JSON.parse(ui);
			$("#nav_ul_guest").hide();
			$("#nav_ul_member").show();
			$("#nav_ul_member_name").html(ui.Un);
			
		}
		
	},
	
	
};

Pic98.init();