package webapi100

import (
	"encoding/json"
	"log"
	"net/http"

	"github.com/gorilla/mux"
)

func ServeWebapi100(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", "*")
	vars := mux.Vars(r)
	log.Println("serveTest - ", r.URL, " - ", vars["action"])

	var v interface{}
	err := json.NewDecoder(r.Body).Decode(&v)
	if err != nil {
		// handle error
	}
	log.Println(v)

	switch vars["action"] {
	case "auth":

		break
	case "roomlist":
		break
	}
}
