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
		"roomlist":     f_roomlist,
		"authout":      f_authout,
	}
)

func ServeWebapi100_(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	//log.Println("serveTest - ", r.URL, " - ", vars["action"])
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

func ServeWebapi100(w http.ResponseWriter, r *http.Request) {
	//vars := mux.Vars(r)

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
		if action == "auth" or action="authout" {
			if f, ok := actions[action]; ok {

				f(w, r, &v1)
			} else {
				http.Error(w, "NotFound", http.StatusNotFound)
			}
			return
		}

		token := r.Header.Get("mster-token")
		log.Printf("Authenticated Middleware %s\n", token)

		if _, found := sign.Sessions[token]; found {
			// We found the token in our map
			//log.Printf("Authenticated user %s\n", user)
			// Pass down the request to the next middleware (or final handler)

		} else {
			failed := true
			//http.Error(w, "Forbidden", http.StatusForbidden)
			tfile := "./tokens/" + token
			f, err3 := os.Open(tfile) //创建文件
			if err3 == nil {
				defer f.Close()
				//str, err3 := f.ReadString() //f.WriteString(string(wbuf)) //写入文件(字节数组)

				fileinfo, err := f.Stat()
				if err == nil {

					fileSize := fileinfo.Size()
					buffer := make([]byte, fileSize)

					_, err := f.Read(buffer)
					if err == nil {
						ui := &sign.UserInfo{}
						err3 := json.Unmarshal(buffer, &ui)
						if err3 == nil {
							sign.Sessions[token] = ui
							failed = false
							//next.ServeHTTP(w, r)

						}
					}
				}
			}
			if failed {
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
		}

		if f, ok := actions[action]; ok {

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

func f_authout(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_auth", *v)
	token := r.Header.Get("mster-token")
	res := make(map[string]interface{})
	delete(sign.Sessions, token)
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

type roomitem struct {
	Img_src string `json:"img_src"`
	Img_num string `json:"img_num"`
	Title   string `json:"title"`
}

func f_roomlist(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_roomlist", *v)
	ris := []roomitem{
		roomitem{"", "45", "roomitem01"},
		roomitem{"", "48", "roomitem02"},
		roomitem{"", "49", "roomitem03"},
	}
	rmsg, err := json.Marshal(ris)
	if err == nil {
		w.Write(rmsg)
	}

}
