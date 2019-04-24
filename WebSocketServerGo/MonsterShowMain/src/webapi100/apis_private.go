package webapi100

import (
	"cfg"
	"database/sql"
	"encoding/json"
	"log"
	"net/http"
	"os"
	"time"
	"ucenter"
	//"github.com/gorilla/mux"
)

var (
	actions_private = map[string](func(http.ResponseWriter, *http.Request, *map[string]interface{})){
		"modipassword": f_modipassword,
		"authout":      f_authout,
		"medialist":    f_medialist,
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

type mediaitem struct {
	Id          string //`json:"id"`
	CreateDate  string //`json:"createdate"`
	PublisherId string //`json:"PublisherId"`
	NickName    string //`json:"title"`
	FileSize    int64  //`json:"follow"`
	Followed    int64  //`json:"onlines"`
	Readed      int64  //`json:"intro"`
}

func f_medialist(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_medialist", *v)
	roomid := (*v)["roomid"].(string)
	res := make(map[string]interface{})
	res["t"] = "medialist"
	res["status"] = 0
	res["msg"] = ""
	mis := []mediaitem{}

	db, err0 := sql.Open("adodb", cfg.Cfg["mssql"])
	if err0 != nil {
		log.Println("ServeSrs sql open error")
	} else {
		defer db.Close()
		stmt1, err0 := db.Prepare(`
			select [Id], [CreateDate],[PublisherId],[NickName],[FileSize],[Followed],[Readed]
			from [hds12204021_db].[dbo].[Web2019_historylist]
			where roomid=? and Deleted = 0 order by CreateDate desc
						`)
		if err0 != nil {
			log.Println("ServeSrs sql db.Prepare error")
		} else {
			defer stmt1.Close()
			rows, err := stmt1.Query(roomid)
			log.Println(rows)
			if err != nil {
				log.Println("ServeSrs sql stmt.Query error", err.Error())
			} else {
				culs, _ := rows.Columns()
				count := len(culs)
				vals := make([]interface{}, count)
				for rows.Next() {
					ri := mediaitem{}
					//rows.Scan(&ri.Id, &ri.Title, &ri.Intro, &ri.Icon, &ri.Corver, &ri.Background, &ri.Follow, &ri.Onlines, &ri.Type, &ri.PullUri, &ri.OrderIdx)
					rows.Scan(&vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &vals[6]) //, &vals[7], &vals[8], &vals[9], &vals[10])
					if vals[0] != nil {
						ri.Id = vals[0].(string)
					}
					if vals[1] != nil {
						ri.CreateDate = (vals[1].(time.Time)).Format(("2006-01-02 15:04:05"))
					}
					if vals[2] != nil {
						ri.PublisherId = vals[2].(string)
					}
					if vals[3] != nil {
						ri.NickName = vals[3].(string)
					}
					if vals[4] != nil {
						ri.FileSize = vals[4].(int64)
					}
					if vals[5] != nil {
						ri.Followed = vals[5].(int64)
					}
					if vals[6] != nil {
						ri.Readed = (vals[6].(int64))
					}
					mis = append(mis, ri)
				}
			}
		}
	}

	res["medialist"] = mis
	res["msg"] = ""
	rmsg, err := json.Marshal(res)
	log.Println(rmsg)
	if err == nil {
		w.Write(rmsg)
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
