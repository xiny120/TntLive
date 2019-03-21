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
)

var addr = flag.String("addr", ":8091", "http service address")

func serveHome(w http.ResponseWriter, r *http.Request) {
	log.Println(r.URL)
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
		fmt.Println(string(a))
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
	cfg.Cfg["mssql"] = connString //"sqlserver://hds12204021:@hds12204021.my3w.com?database=hds12204021_db&connection+timeout=30" //connString
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
