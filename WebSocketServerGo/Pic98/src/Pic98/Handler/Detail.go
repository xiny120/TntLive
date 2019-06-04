package Handler

import (
	"html/template"
	"log"
	"net/http"
	"strings"

	_ "github.com/Unknwon/goconfig"
)

func Detail(w http.ResponseWriter, r *http.Request) {

	t, err := template.ParseFiles(
		"wwwroot/tpl/Detail.html",
		"wwwroot/tpl/public/header.html",
		"wwwroot/tpl/public/nav.html",
		"wwwroot/tpl/public/footer.html")
	if err != nil {
		//log.Fatal(err)
	}

	imgurl := strings.Replace(r.RequestURI, "/Detail", "", 1)
	log.Println(imgurl)

	cookie, err := r.Cookie("token")
	if err == nil {
		cookievalue := cookie.Value
		log.Println(cookievalue)
	} else {
		log.Println("未认证用户！游客模式！")
	}

	data := struct {
		Title    string
		Listtype string
	}{
		Title:    "列表",
		Listtype: imgurl,
	}

	err = t.Execute(w, data)
	if err != nil {
		//log.Fatal(err)
	}
	//fmt.Fprintf(w, "%s", t.e)
}
