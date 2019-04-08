package admin

import (
	"log"
	"net/http"
	"strings"
)

func ServeAdmin(w http.ResponseWriter, r *http.Request) {
	//file := "static" + r.URL.Path
	file := strings.Replace(r.URL.Path, "/", "www/", 1)
	log.Println(file)

	http.ServeFile(w, r, file)
}
