package webapi100

import (
	sign "MonsterShowMain/ucenter"
	"encoding/json"
	"log"
	"net/http"

	//"os"

	"github.com/gorilla/mux"
)

// Oauth ok
func Oauth(w http.ResponseWriter, r *http.Request) {
	w.Write([]byte("Gorilla! oauth\n"))
}

// AuthenticationMiddleware Define our struct
type AuthenticationMiddleware struct {
	tokenUsers map[string]string
}

// Populate Initialize it somewhere
func (amw *AuthenticationMiddleware) Populate() {
	amw.tokenUsers = make(map[string]string)
	amw.tokenUsers["00000000"] = "user0"
	amw.tokenUsers["aaaaaaaa"] = "userA"
	amw.tokenUsers["05f717e5"] = "randomUser"
	amw.tokenUsers["deadbeef"] = "user0"
}

// Middleware 中间件。进行统一的权限认证校验。
func (amw *AuthenticationMiddleware) Middleware(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {

		if origin := r.Header.Get("Origin"); origin != "" {
			w.Header().Set("Access-Control-Allow-Origin", "*") //允许访问所有域
			w.Header().Set("Access-Control-Allow-Methods", "POST")
			w.Header().Add("Access-Control-Allow-Headers", "Content-Type,Origin,mster-token") //header的类型
			w.Header().Set("Content-Type", "application/json")                                //返回数据格式是json
		}

		if r.Method == "OPTIONS" {
			return
		}

		vars := mux.Vars(r)
		if vars["action"] == "auth" {
			next.ServeHTTP(w, r)
		} else {

			token := r.Header.Get("mster-token")
			log.Printf("Authenticated Middleware %s - %s\n", vars["action"], token)

			if _, found := sign.SessionsGet(token); found {
				next.ServeHTTP(w, r)
			} else {
				res := make(map[string]interface{})
				res["t"] = "not sign in"
				res["status"] = 1
				res["msg"] = "请登录后操作"
				rmsg, err := json.Marshal(res)
				if err == nil {
					w.Write(rmsg)
				}
			}
		}
	})
}
