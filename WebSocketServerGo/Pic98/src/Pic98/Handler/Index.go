package handler

import (
	"Pic98/cfg"
	"database/sql"
	"encoding/json"
	"html/template"
	"log"
	_ "log"
	"net/http"
	"net/url"
	"strconv"

	_ "github.com/go-sql-driver/mysql"
)

// oPic do
type oPic struct {
	Vcategoryguid string `json:"categoryguid"`
	Vaguid        string `json:"aguid"`
	Vpicurl       string `json:"picurl"`
	Vcreatetime   string `json:"createtime"`
	Vuserguid     string `json:"userguid"`
	Vidolguid     string `json:"idolguid"`
	Vlike         string `json:"like"`
	Vtitle        string `json:"title"`
	Vintro        string `json:"intro"`
	Vtags         string `json:"tags"`
}

type oNav struct {
	Title string
	URL   string
}

// TData do
type TData struct {
	Title       string
	SiteName    string
	Keywords    string
	Description string
	NavActive   string
	Navs        []oNav
	Data        interface{}
	MiitGov     string
}

var (
	// PageData ok
	PageData = TData{
		Title:       "首页 - 直播课堂",
		SiteName:    "Pic98.com [图酒吧]",
		Keywords:    "摄影，街拍，丝袜，美腿，约拍，外拍模特",
		Description: "全网最新，最全街拍美腿，高清图片，丝袜视频，同城交友，约拍，没有看不到！只有想不到！",
		NavActive:   "/",
		Navs:        []oNav{{"街拍美腿", "4dfe0be9-6b9f-4b68-b67b-10fa72671de7"}, {"角色COS", "6bfffed3-cdb3-4c70-908a-80239eb94870"}, {"修长美腿", "f1b9e628-177b-4199-b3d6-8dd771dab2ee"}},
		MiitGov:     "沪ICP备17042145号-3",
	}
)

// Index do
func Index(w http.ResponseWriter, r *http.Request) {
	//time.Sleep(60 * time.Second)
	u, err := url.Parse(r.RequestURI)
	if err == nil {
		filePath := "wwwroot/static/www" + u.Path
		log.Println(filePath)
		if pe, _ := FileExists(filePath); pe == true {
			http.ServeFile(w, r, filePath)
			return
		} else {
			t, err := template.ParseFiles(
				"wwwroot/tpl/Index.html",
				"wwwroot/tpl/public/header.html",
				"wwwroot/tpl/public/nav.html",
				"wwwroot/tpl/public/footer.html")
			data := PageData

			if err != nil {
			} else {
				db, err := sql.Open("mysql", cfg.Cfg["tidb"])
				if err != nil {
				} else {
					defer db.Close()
					stmt, _ := db.Prepare(`SELECT aguid,coverimg,likesum,title FROM topic order by createtime desc limit ?,?`)
					defer stmt.Close()
					rows, err := stmt.Query(0, 20)
					if err == nil {
						defer rows.Close()
						var pics []oPic
						for rows.Next() {
							var pic oPic
							rows.Scan(&pic.Vaguid, &pic.Vpicurl, &pic.Vlike, &pic.Vtitle)
							pics = append(pics, pic)
						}
						data.Data = pics
					}
				}
			}

			err = t.Execute(w, data)
			if err != nil {
				//log.Fatal(err)
			}
		}
	}

}

// Index_Hotidol ok
func Index_Hotidol(w http.ResponseWriter, r *http.Request) {
	pagesize := 20
	pageidx := 0
	errf := r.ParseForm()
	if errf != nil {
		//result := "{\"status\":1,\"msg\":\"WebApi Account/Register/Cmd ParseForm失败\"}"
	} else {
		pageidx, _ = strconv.Atoi(r.FormValue("pageidx"))
	}

	db, err := sql.Open("mysql", cfg.Cfg["tidb"])
	if err != nil {
		//log.Fatal(err)
	}
	defer db.Close()

	startidx := pageidx * pagesize

	stmt, _ := db.Prepare(`SELECT categoryguid, aguid, picurl, createtime, idolguid, likesum from picinfo order by likemonth desc limit ?,?`)
	//log.Println(stmt)
	defer stmt.Close()
	rows, err := stmt.Query(startidx, pagesize)
	//log.Println(rows)
	//log.Println(err)
	if err == nil {
		defer rows.Close()

		var pics []oPic
		for rows.Next() {
			var pic oPic

			rows.Scan(&pic.Vcategoryguid, &pic.Vaguid, &pic.Vpicurl, &pic.Vcreatetime, &pic.Vidolguid, &pic.Vlike)
			pics = append(pics, pic)
		}

		usersBytes, _ := json.Marshal(&pics)
		//log.Println(string(usersBytes))
		w.Write(usersBytes)
	}
}

// Index_Newidol ok
func Index_Newidol(w http.ResponseWriter, r *http.Request) {
	pagesize := 30
	pageidx := 0
	errf := r.ParseForm()
	if errf != nil {
		//result := "{\"status\":1,\"msg\":\"WebApi Account/Register/Cmd ParseForm失败\"}"
	} else {
		pageidx, _ = strconv.Atoi(r.FormValue("pageidx"))
	}

	db, err := sql.Open("mysql", cfg.Cfg["tidb"])
	if err != nil {
		//log.Fatal(err)
	}
	defer db.Close()
	startidx := pageidx * pagesize
	stmt, _ := db.Prepare(`SELECT aguid,coverimg,createtime,likesum,userguid,idolguid,title,intro,tags FROM Pic98.topic order by createtime desc limit ?,?`)
	//log.Println(stmt)
	defer stmt.Close()
	rows, err := stmt.Query(startidx, pagesize)
	//log.Println(rows)
	//log.Println(err)
	if err == nil {
		defer rows.Close()

		var pics []oPic
		for rows.Next() {
			var pic oPic

			rows.Scan(&pic.Vaguid, &pic.Vpicurl, &pic.Vcreatetime, &pic.Vlike, &pic.Vuserguid, &pic.Vidolguid, &pic.Vtitle, &pic.Vintro, &pic.Vtags)
			pics = append(pics, pic)
		}

		usersBytes, _ := json.Marshal(&pics)
		//log.Println(string(usersBytes))
		w.Write(usersBytes)
	}
}
