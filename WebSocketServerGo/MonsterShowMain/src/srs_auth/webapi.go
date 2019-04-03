package srs_auth

import (
	"encoding/json"
	"log"
	"net/http"
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

	//log.Println(r.URL)
	var v interface{}
	err := json.NewDecoder(r.Body).Decode(&v)
	if err == nil {
		v1 := v.(map[string]interface{})
		action := ""
		app := ""
		stream := ""
		param := "?"
		client_id := 0
		//action := v1["action"]
		if action_, ok := v1["action"]; ok {
			action = action_.(string)
		}
		if app_, ok := v1["app"]; ok {
			app = app_.(string)
		}
		if stream_, ok := v1["stream"]; ok {
			stream = stream_.(string)
		}
		if client_id_, ok := v1["client_id"]; ok {
			//client_id = int(client_id_.(float64))
			client_id = int(client_id_.(int))
		}
		if param_, ok := v1["param"]; ok {
			param = param_.(string)
		}

		log.Println(action, app, stream, client_id, param)
		switch action {
		case "on_connect":

		case "on_publish":
		case "on_unpublish":
		case "on_close":
		case "on_play":
		case "on_stop":
		}
		//http.Error(w, "NotFound", http.StatusNotFound)
		w.Write([]byte("0"))
	} else {
		w.Write([]byte("1"))
	}
}
