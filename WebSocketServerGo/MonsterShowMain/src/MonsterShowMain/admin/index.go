package admin

import (
	"html/template"
	"log"
	"net/http"
	"net/url"
	//"strings"
)

func ServeAdmin(w http.ResponseWriter, r *http.Request) {
	u0, _ := url.Parse(r.URL.Path)
	log.Println(u0.Host)
	log.Println(r.URL.Path)
	log.Println(r.URL.Fragment)
	log.Println(r.URL.Host)
	log.Println(r.URL.Opaque)
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
