package handler

import (
	"Pic98/cfg"
	"database/sql"
	"html/template"
	"net/http"
	"strings"

	//goconfig
	_ "github.com/Unknwon/goconfig"
)

// List ok
func List(w http.ResponseWriter, r *http.Request) {
	t, err := template.ParseFiles(
		"wwwroot/tpl/List.html",
		"wwwroot/tpl/public/header.html",
		"wwwroot/tpl/public/nav.html",
		"wwwroot/tpl/public/footer.html")
	if err != nil {
	} else {

	}
	data := PageData
	data.Title = "有错误哦！"
	param := strings.Split(r.RequestURI, "/")
	if len(param) >= 3 {
		ids := strings.Split(param[2], ".")
		data.NavActive = ids[0]
		data.Data = param[2]
		data.Title = "列表"

		if err != nil {
		} else {
			db, err := sql.Open("mysql", cfg.Cfg["tidb"])
			if err != nil {
			} else {
				defer db.Close()
				stmt, _ := db.Prepare(`
				SELECT a.aguid,a.coverimg,a.likesum,a.title 
				FROM topic a, tags2topic b
				where a.aguid = b.topicguid and b.tagguid=?
				order by b.createtime desc
				limit ?, ? 
				
				`)
				defer stmt.Close()
				rows, err := stmt.Query(ids[0], 0, 20)
				if err == nil {
					defer rows.Close()
					var pics []oPic
					for rows.Next() {
						var pic oPic
						rows.Scan(&pic.Vaguid, &pic.Vpicurl, &pic.Vlike, &pic.Vtitle)
						pics = append(pics, pic)
					}
					data.Data = pics
				}
			}
		}

	}
	err = t.Execute(w, data)
	if err != nil {
	}
}
