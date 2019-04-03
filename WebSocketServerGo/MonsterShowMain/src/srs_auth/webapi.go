package srs_auth

import (
	"encoding/json"
	"errors"
	"log"
	"net/http"
	"ucenter"

	"strings"
)

func ServeSrs(w http.ResponseWriter, r *http.Request) {
	if origin := r.Header.Get("Origin"); origin != "" {
		w.Header().Set("Access-Control-Allow-Origin", "*") //允许访问所有域
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, HEAD, PUT, DELETE")
		w.Header().Add("Access-Control-Allow-Headers", "Cache-Control, X-Proxy-Authorization, X-Requested-With, Content-Type") //header的类型
		//		w.Header().Set("Content-Type", "application/json")                                //返回数据格式是json
	}

	if r.Method == "OPTIONS" {
		return
	}
	var data struct {
		Action   string `json:"action"`
		App      string `json:"app"`
		Hello    string `json:"hello"`
		Stream   string `json:"stream"`
		Param    string `json:"param"`
		ClientId int64  `json:"client_id"`
	}
	err := json.NewDecoder(r.Body).Decode(&data)
	//log.Println(data, err)
	retstr := "1"
	if err == nil {
		switch data.Action {
		case "on_connect":
			retstr = "0"

		case "on_publish":
			pars, err := uri2map(data.Param)
			if err == nil {
				ui, _ := sign.SessionsGet(pars["sessionid"])
				log.Println(ui)
				if ui.Token == pars["token"] {
					retstr = "0"
				}
			}
		case "on_unpublish":
			retstr = "0"
		case "on_close":
			retstr = "0"
		case "on_play":
			retstr = "0"
		case "on_stop":
			retstr = "0"
		}
	}
	w.Write([]byte(retstr))
}

func uri2map(uri string) (map[string]string, error) {
	m := make(map[string]string)
	if len(uri) < 1 {
		return m, errors.New("uri is none")
	}
	if uri[0:1] == "?" {
		uri = uri[1:]
	}

	pars := strings.Split(uri, "&")
	for _, par := range pars {
		parkv := strings.Split(par, "=")
		m[parkv[0]] = parkv[1]
	}
	log.Println(m)
	return m, nil
}
