package Handler

import (
	"html/template"
	"log"
	"net/http"
	"strings"

	_ "github.com/Unknwon/goconfig"
)

func List(w http.ResponseWriter, r *http.Request) {

	t, err := template.ParseFiles(
		"wwwroot/tpl/List.html",
		"wwwroot/tpl/public/header.html",
		"wwwroot/tpl/public/nav.html",
		"wwwroot/tpl/public/footer.html")
	if err != nil {
		//log.Fatal(err)
	}

	log.Println(r.RequestURI)
	param := strings.Split(r.RequestURI, "/")
	if len(param) >= 3 {
		log.Println(len(param))
		log.Println(param[2])
	}

	data := PageData
	data.Title = "列表"
	data.Data = param[2]

	err = t.Execute(w, data)
	if err != nil {
		//log.Fatal(err)
	}
	//fmt.Fprintf(w, "%s", t.e)
}
