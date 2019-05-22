// Copyright 2019 The MonsterShow Authors. All rights reserved.

package main

import (
	"admin"
	"flag"
	"log"
	"net/http"
	"srs_auth"
	"webapi100"

	"github.com/gorilla/mux"
)

var addr = flag.String("addr", ":8092", "http service address")

func main() {
	log.SetFlags(log.Lshortfile | log.LstdFlags)
	initHubs()
	initDB() //初始化数据库
	flag.Parse()
	r := mux.NewRouter()

	//r.HandleFunc("/", admin.ServeAdmin)
	sr := r.PathPrefix("/admin")
	sr.HandlerFunc(admin.ServeAdmin)
	//sr.Handler(http.StripPrefix("/", http.FileServer(http.Dir("static/"))))
	//sr.Handler(http.StripPrefix("/admin/", http.FileServer(http.Dir("static/"))))
	r.HandleFunc("/ws/{id}", serveWebSocket)             // websocket的handleFunc.其中表示聊天室编号。
	r.HandleFunc("/api/1.00/private", webapi100.Private) // webapi 1.0.0 协议
	r.HandleFunc("/api/1.00/public", webapi100.Public)
	r.HandleFunc("/srs", srs_auth.ServeSrs)
	r.PathPrefix("/").Handler(http.FileServer(http.Dir("www/")))
	log.Println("ListenAndServe Success at: ", *addr)
	err := http.ListenAndServe(*addr, r)
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}
}

func test(w http.ResponseWriter, r *http.Request) {
	log.Println(r.URL.Path)
}
