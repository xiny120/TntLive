
var hotidol;
var hotidolitem;
var num = 0;
var page = 1;
var off_on = true;
var online_key="00000000-0000-0000-0000-000000000000";
var column_count = parseInt($("#hotidol").css("column-count"));

$(document).ready(function(){
	$(".card-columns").on("click",".card-img-top",function(){
		
	});
	
	ui = JSON.parse(window.localStorage.getItem("ui"));
	if(ui != null && ui != undefined){
		online_key = ui.Online_key;
	}
	
	hotidol = $("#hotidol").children(0).clone();
	hotidolitem = $("#hotidol").children(0).children(0).clone();
	$("#hotidol").children(0).empty();
	//setTimeout(gethotidol(), 30);
	//setTimeout(getnewidol(0), 30);
	
    $(window).scroll(function(){
        var windowTop = parseInt($(window).scrollTop());
        var top = parseInt($(document).scrollTop());
        var height = $(window).height();
        var height1 = $(document).height();
        var totalHeight = windowTop + height;

        if((height1 <= (totalHeight + 100) ) && page<= 30){
            if (off_on) {
				off_on = false;			
				setTimeout(getnewidol(page), 30);
	            page++;
	            num++;
	            console.log(page,num);
			}
        }
    });
}); 


var hotidolurl = "/Index/Hotidol";

function gethotidol(){
	$.post(hotidolurl, { cpid:null},
	function (data,status) {
		if(data != null){
			//listappend($("#hotidol"),data);
		}
	},'json');
}


var newidolurl = "/Index/Newidol";

function getnewidol(pageidx){
	$.post(newidolurl, { pageidx:pageidx},
	function (data,status) {
		if(data != null){
			listappend($("#newidol"),data);
			off_on = true;
		}
	},'json');
}

function listappend(container,data){
	var hotidol0 = $(hotidol).clone();
	$(hotidol0).empty();
	$(data).each(function(idx,item){
		var hotidolitem0 = $(hotidolitem).clone();
		$(hotidolitem0).find(".card-img-top").attr("src","/thumbnail" + item.picurl);
		$(hotidolitem0).find(".card_a").attr("href","/topic/" + item.aguid + ".html");
		$(hotidolitem0).find(".card_a").attr("alt",item.title);
		$(hotidol0).append($(hotidolitem0));
	});	
	$(container).append(hotidol0);
}
