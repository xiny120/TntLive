package admin

import (
	"Pic98/member"
	"html/template"
	"net/http"
	//"strings"
)

// TAdminData do
type TAdminData struct {
	Title       string
	SiteName    string
	Keywords    string
	Description string
	UserAvatar  string
	UserName    string
	UserGroup   string

	YourMessage     string
	YourMessageList interface{}
	Data            interface{}
}

var (
	// AdminPageDate ok
	AdminPageDate = TAdminData{
		Title: "首页", SiteName: "怪秀后台管理", Keywords: "怪秀视频直播！", Description: "秀出你的风采",
		UserAvatar: "https://s3.amazonaws.com/uifaces/faces/twitter/jsa/48.jpg", UserName: "test", UserGroup: "test",
		YourMessage: "您有5条信息尚未阅读!"}
)

// ServeAdmin ok
func ServeAdmin(w http.ResponseWriter, r *http.Request) {
	ui, err := member.LoadUserinfo(r)
	if err != nil || ui.IsAdmin != 1 {
		http.Error(w, "StatusUnauthorized", http.StatusUnauthorized)
		return
	}
	if r.URL.Path == "/admin/" {
		if t, err0 := template.ParseFiles(
			"wwwroot/admin/tpl/index.html",
			"wwwroot/admin/tpl/home.html",
			"wwwroot/admin/tpl/article/article-original-review.html",
			"wwwroot/admin/tpl/article/article-original-search.html",
			"wwwroot/admin/tpl/article/article-search.html",
			"wwwroot/admin/tpl/article/article-review-search.html",
			"wwwroot/admin/tpl/memberrequest.html",
			"wwwroot/admin/tpl/memberquery.html",
			"wwwroot/admin/tpl/memberadmin.html"); err0 == nil {
			data := AdminPageDate
			data.Title = "用户中心"
			data.UserName = ui.Un
			data.UserAvatar = ui.Avatar
			err := t.Execute(w, data)
			if err != nil {
			}
			return
		}
		http.Error(w, "template.ParseFiles error", http.StatusInternalServerError)
		return
	}
	http.ServeFile(w, r, "wwwroot"+r.URL.Path)
}
