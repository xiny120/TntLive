// Copyright 2013 The Gorilla WebSocket Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"bufio"
	"cfg"
	"flag"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strconv"
	"webapi100"

	"github.com/gorilla/mux"
)

var addr = flag.String("addr", ":8091", "http service address")

func serveHome(w http.ResponseWriter, r *http.Request) {
	log.Println("serveHome - %s", r.URL)
	if r.URL.Path != "/" {
		http.Error(w, "Not found", http.StatusNotFound)
		return
	}
	if r.Method != "GET" {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}
	http.ServeFile(w, r, "home.html")
}

func hello_abc(w http.ResponseWriter, r *http.Request) {
	log.Println("hello_abc - %s", r.URL)
	if r.URL.Path != "/" {
		http.Error(w, "Not found", http.StatusNotFound)
		return
	}
	if r.Method != "GET" {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}
	http.ServeFile(w, r, "home.html")
}

func main() {
	fi, err0 := os.Open("conf/mssql.dat")
	if err0 != nil {
		fmt.Printf("Error: %s\n", err0)
		return
	}
	defer fi.Close()

	br := bufio.NewReader(fi)
	var conf []string

	for {
		a, _, c := br.ReadLine()
		if c == io.EOF {
			break
		}
		conf = append(conf, string(a))
	}
	fmt.Println(conf)
	var isdebug = true

	//连接字符串
	//connString := fmt.Sprintf("server=%s;port%d;database=%s;user id=%s;password=%s", server, port, database, user, password)
	connString := fmt.Sprintf("Provider=SQLOLEDB;Data Source=%s;Initial Catalog=%s;user id=%s;password=%s", conf[0], conf[2], conf[3], conf[4])

	if isdebug {
		fmt.Println(connString)
	}

	cfg.Cfg["tidb"] = "pic98:vck123456@tcp(106.14.145.51:4000)/Pic98"
	cfg.Cfg["mssql"] = connString
	flag.Parse()
	//hub := newHub()
	//go hub.run()
	var hubs [999]*Hub

	r := mux.NewRouter()

	r.HandleFunc("/", serveHome)
	r.HandleFunc("/ws/{id}", func(w http.ResponseWriter, r *http.Request) {
		vars := mux.Vars(r)

		id, _ := strconv.Atoi(vars["id"])
		if id > 0 && id < 1000 {
			if hubs[id] == nil {
				log.Println("hubs[id] is nil")
				hubs[id] = newHub()
				go hubs[id].run()
			}
			log.Println("serveTest - ", r.URL, " - ", vars["id"], id, hubs[id])
			serveWs(hubs[id], w, r)
		} else {
			http.Error(w, "NotFound", http.StatusNotFound)
		}

	})

	// webapi 1.0.0 协议
	r.HandleFunc("/api/1.00/private", webapi100.Private)
	r.HandleFunc("/api/1.00/public", webapi100.Public)
	//s := r.PathPrefix("/api/1.00").Subrouter()
	//s0 := s.PathPrefix("/hello").Subrouter()
	//s0.HandleFunc("/abc", hello_abc)
	//s.HandleFunc("/{action}", webapi100.ServeWebapi100)
	//amw := webapi100.AuthenticationMiddleware{}
	//amw.Populate()
	//s.Use(amw.Middleware)

	log.Println(*addr)
	err := http.ListenAndServe(*addr, r)
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}
}
