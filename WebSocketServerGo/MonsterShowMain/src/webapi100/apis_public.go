package webapi100

import (
	"cfg"
	"database/sql"
	"encoding/json"
	"fmt"
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

func Public(w http.ResponseWriter, r *http.Request) {
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

type roomitem struct {
	Img_src    string `json:"img_src"`
	Img_num    string `json:"img_num"`
	Title      string `json:"title"`
	Follow     int64  `json:"follow"`
	Onlines    int64  `json:"onlines"`
	Intro      string `json:"intro"`
	PullUri    string `json:"pulluri"`
	Id         string `json:"id"`
	Icon       string `json:"icon"`
	Corver     string `json:"corver"`
	Background string `json:"background"`
	Type       int64  `json:"type"`
	OrderIdx   int64  `json:"orderidx"`
}

func f_roomlist(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_roomlist", *v)
	res := make(map[string]interface{})
	res["t"] = "f_roomlist"
	res["status"] = 1
	res["msg"] = "未知错误！"
	ris := []roomitem{
		//roomitem{"http://gwgz.com/images_man/d1_180.jpg", "45", "唐能通盘中直播房间", "800", "600"},
		//roomitem{"http://gwgz.com/images_man/d2_180.jpg", "48", "唐能通100天训练", "890", "300"},
		//roomitem{"http://gwgz.com/images_man/d3_180.jpg", "49", "唐能通光盘课课件", "900", "200"},
		//roomitem{"http://gwgz.com/images_man/d3_180.jpg", "49", "唐能通短线是银商城", "8000", "6000"},
	}
	result := ""
	page := int((*v)["page"].(float64)) - 1
	per_page := int((*v)["per_page"].(float64))
	db, err0 := sql.Open("adodb", cfg.Cfg["mssql"])
	if err0 != nil {
		result = "f_roomlist sql open error"
	} else {
		defer db.Close()
		sqlstr := fmt.Sprintf("SELECT TOP (%d) [Id], [Title], [Intro], [Icon], [Corver], [Background], [Follow], [Online],  [Type], [PullUri], [OrderIdx] FROM  [Web2019_roomlist] where [orderidx] >= %d order by [orderidx] asc", per_page, page*per_page)
		log.Println(sqlstr)
		stmt, err0 := db.Prepare(sqlstr)
		if err0 != nil {
			log.Println(err0)
			result = "f_roomlist sql db.Prepare error"
		} else {
			defer stmt.Close()
			rows, err := stmt.Query()
			if err != nil {
				result = "f_roomlist sql stmt.Query error"
			} else {
				result = "用户名或密码错误！"
				culs, _ := rows.Columns()
				count := len(culs)
				vals := make([]interface{}, count)
				//valuePtrs := make([]interface{}, count)
				//for i := 0; i < count; i++ {
				//	valuePtrs[i] = &values[i]
				//}
				for rows.Next() {
					ri := roomitem{}
					//rows.Scan(&ri.Id, &ri.Title, &ri.Intro, &ri.Icon, &ri.Corver, &ri.Background, &ri.Follow, &ri.Onlines, &ri.Type, &ri.PullUri, &ri.OrderIdx)
					rows.Scan(&vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &vals[6], &vals[7], &vals[8], &vals[9], &vals[10])
					if vals[0] != nil {
						ri.Id = vals[0].(string)
					}
					if vals[1] != nil {
						ri.Title = vals[1].(string)
					}
					if vals[2] != nil {
						ri.Intro = vals[2].(string)
					}
					if vals[3] != nil {
						ri.Icon = vals[3].(string)
					}
					if vals[4] != nil {
						ri.Corver = vals[4].(string)
					}
					if vals[5] != nil {
						ri.Background = vals[5].(string)
					}
					if vals[6] != nil {
						ri.Follow = (vals[6].(int64))
					}
					if vals[7] != nil {
						ri.Onlines = (vals[7].(int64))
					}
					if vals[8] != nil {
						ri.Type = (vals[8].(int64))
					}
					if vals[9] != nil {
						ri.PullUri = vals[9].(string)
					}
					if vals[10] != nil {
						ri.OrderIdx = vals[10].(int64)
					}
					result = "登录成功"

					ris = append(ris, ri)
					//break
				}
			}
		}

	}
	res["roomlist"] = ris
	res["msg"] = result
	rmsg, err := json.Marshal(ris)
	if err == nil {
		w.Write(rmsg)
	}

}
