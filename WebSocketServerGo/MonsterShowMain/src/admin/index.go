package admin

import (
	"html/template"
	"log"
	"net/http"
	//"strings"
)

func ServeAdmin(w http.ResponseWriter, r *http.Request) {
	log.Println(r.URL.Path)
	if r.URL.Path == "/admin/" {
		t, _ := template.ParseFiles(
			"www/admin/tpl/index.html",
			"www/admin/tpl/home.html",
			"www/admin/tpl/memberrequest.html",
			"www/admin/tpl/memberquery.html",
			"www/admin/tpl/memberadmin.html")
		log.Println(r.URL.Path, t.Name())
		t.Execute(w, "Hello world")
	} else {
		http.ServeFile(w, r, "www"+r.URL.Path)
	}
}
