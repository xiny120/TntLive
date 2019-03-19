// main.go
package main

import (
	"flag"
	"log"
	"net/http"
)

var addr = flag.String("addr", ":8081", "http service address")

func serveHome(w http.ResponseWriter, r *http.Request) {
	log.Println(r.URL)

	if r.Method != "GET" {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	if r.URL.Path != "/" {
		if r.URL.Path == "/live/h5client/mainpage" {
			http.ServeFile(w, r, "")
		}
	} else {

		http.ServeFile(w, r, "home.html")
	}
}

func main() {
	flag.Parse()
	hub := newHub()
	go hub.run()
	http.HandleFunc("/", serveHome)
	http.HandleFunc("/ws", func(w http.ResponseWriter, r *http.Request) {
		serveWs(hub, w, r)
	})
	log.Println(*addr)
	err := http.ListenAndServe(*addr, nil)
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}

}
