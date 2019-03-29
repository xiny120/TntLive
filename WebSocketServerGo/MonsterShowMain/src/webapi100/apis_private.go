package webapi100

import (
	"encoding/json"
	"log"
	"net/http"
	"os"
	"ucenter"
	//"github.com/gorilla/mux"
)

var (
	actions_private = map[string](func(http.ResponseWriter, *http.Request, *map[string]interface{})){
		"auth":         f_auth,
		"modipassword": f_modipassword,
		"roomlist":     f_roomlist,
		"authout":      f_authout,
	}
)

func Private(w http.ResponseWriter, r *http.Request) {
	if origin := r.Header.Get("Origin"); origin != "" {
		w.Header().Set("Access-Control-Allow-Origin", "*") //允许访问所有域
		w.Header().Set("Access-Control-Allow-Methods", "POST")
		w.Header().Add("Access-Control-Allow-Headers", "Content-Type,Origin,mster-token") //header的类型
		w.Header().Set("Content-Type", "application/json")                                //返回数据格式是json
	}

	if r.Method == "OPTIONS" {
		return
	}

	var v interface{}
	err := json.NewDecoder(r.Body).Decode(&v)
	log.Println("serveTest - ", r.URL, " - ", v)
	if err != nil {
		w.Write([]byte("\"status\":1,\"msg\":\"提交的json参数解析失败\""))
	} else {
		v1 := v.(map[string]interface{})
		action := v1["action"].(string)
		token := r.Header.Get("mster-token")
		log.Printf("Authenticated Middleware %s\n", token)

		if _, found := sign.SessionsGet(token); found {
			// We found the token in our map
			//log.Printf("Authenticated user %s\n", user)
			// Pass down the request to the next middleware (or final handler)

		} else {

			res := make(map[string]interface{})
			res["t"] = "not sign in"
			res["status"] = 1
			res["msg"] = "请登录后操作"
			rmsg, err := json.Marshal(res)
			if err == nil {
				w.Write(rmsg)
			}
			return
		}

		if f, ok := actions_private[action]; ok {

			f(w, r, &v1)
		} else {
			http.Error(w, "NotFound", http.StatusNotFound)
		}
	}
}

func f_authout(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_auth", *v)
	token := r.Header.Get("mster-token")
	res := make(map[string]interface{})
	sign.SessionsDel(token)
	tfile := "./tokens/" + token
	os.Remove(tfile) //创建文件
	res["t"] = "authout"
	res["status"] = 0
	res["msg"] = "注销成功！"
	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

func f_modipassword(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	account := (*v)["account"].(string)
	password := (*v)["password"].(string)
	password1 := (*v)["password1"].(string)
	msg, status := sign.ModiPassword(account, password, password1)
	res["t"] = "modipassword"
	res["status"] = status
	res["msg"] = msg
	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}
