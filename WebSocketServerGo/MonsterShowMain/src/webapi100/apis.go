package webapi100

import (
	"encoding/json"
	"log"
	"net/http"
	"os"
	"ucenter"

	"github.com/gorilla/mux"
)

var (
	actions = map[string](func(http.ResponseWriter, *http.Request, *map[string]interface{})){
		"auth":         f_auth,
		"modipassword": f_modipassword,
	}
)

func ServeWebapi100(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	log.Println("serveTest - ", r.URL, " - ", vars["action"])
	var v interface{}
	err := json.NewDecoder(r.Body).Decode(&v)
	if err != nil {
		w.Write([]byte("\"status\":1,\"msg\":\"提交的json参数解析失败\""))
	} else {
		if f, ok := actions[vars["action"]]; ok {
			v1 := v.(map[string]interface{})
			f(w, r, &v1)
		} else {
			http.Error(w, "NotFound", http.StatusNotFound)
		}
	}
}

func f_auth(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_auth", *v)
	res := make(map[string]interface{})
	account := (*v)["account"].(string)
	password := (*v)["password"].(string)
	tt := sign.SignIn(account, password)
	sign.Sessions[tt.SessionId] = tt
	wbuf, werr := json.Marshal(tt)
	if werr == nil {
		tfile := "./tokens/" + tt.SessionId
		f, err3 := os.Create(tfile) //创建文件
		if err3 == nil {
			defer f.Close()
			//_, err3 := f.WriteString(string(wbuf)) //写入文件(字节数组)
			_, err3 := f.Write(wbuf) //写入文件(字节数组)
			if err3 != nil {
				os.Remove(tfile)
			}
		}
	}
	res["t"] = "sign in"
	res["status"] = 0
	res["userinfo"] = tt
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
