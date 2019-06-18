package webapi100

import (
	"cfg"
	"toolset"

	//"database/sql"
	"encoding/json"
	"log"
	"net/http"
	"os"
	"time"
	sign "ucenter"
	//"github.com/gorilla/mux"
)

var (
	actionsPrivate = map[string](func(*sign.UserInfo, http.ResponseWriter, *http.Request, *map[string]interface{})){
		"modipassword": fModipassword,
		"authout":      fAuthout,
		//"medialist":    f_medialist,
		"pushroomlist": fPushroomlist,
		"caniplay":     fCaniplay,
	}
)

// Private ok
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
	log.Println("r.Body ", r.Body)
	err := json.NewDecoder(r.Body).Decode(&v)
	if err != nil {
		w.Write([]byte("\"status\":1,\"msg\":\"提交的json参数解析失败\""))
	} else {
		v1 := v.(map[string]interface{})
		action := v1["action"].(string)
		token := r.Header.Get("mster-token")
		log.Printf("Api Private Action:%s Token:%s\n", action, token)

		if ui, found := sign.SessionsGet(token); found {
			if f, ok := actionsPrivate[action]; ok {
				f(ui, w, r, &v1)
			} else {
				http.Error(w, "NotFound", http.StatusNotFound)
			}
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
	}
}

type pushroomitem struct {
	ID      string //`json:"id"`
	Title   string //`json:"title"`
	Intro   string //`json:"intro"`
	Icon    string //`json:"icon"`
	PushURI string //`json:"pushuri"`
	VHost   string //`json:"vhost"`
	VApp    string //`json:"vapp"`
	VStream string //`json:"vstream"`
}

func fPushroomlist(ui *sign.UserInfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	userid := ui.UserID
	res := make(map[string]interface{})
	res["t"] = "roomlist"
	res["status"] = 0
	res["msg"] = ""
	ris := [](map[string]interface{}){}
	db, err0 := cfg.OpenDb() //sql.Open("adodb", cfg.Cfg["mssql"])
	if err0 != nil {
		log.Println("ServeSrs sql open error")
	} else {
		defer db.Close()
		stmt1, err0 := db.Prepare(`
			SELECT  r.roomid, r.title,r.stitle, r.intro, r.icon,r.vhost,r.vport,r.vapp,r.vstream
			FROM publisher p,roomlist r
			where p.roomid = r.roomid and p.userid = ? and r.deleted = 0
						`)
		if err0 != nil {
			log.Println("ServeSrs sql db.Prepare error", err0)
		} else {
			defer stmt1.Close()
			rows, err := stmt1.Query(userid)
			//log.Println(rows)
			if err != nil {
				log.Println("ServeSrs sql stmt.Query error", err.Error())
			} else {
				ris, err = toolset.Rows2Map(rows)
			}
		}
	}

	res["roomlist"] = ris
	res["msg"] = ""
	rmsg, err := json.Marshal(res)
	log.Println(res)
	if err == nil {
		w.Write(rmsg)
	}
}

type caniplay struct {
	RoomID       string
	CreateDate   string
	FilePath     string
	FileName     string
	NickName     string
	Encryptioned int32
	EncKey       int32
}

func fCaniplay(ui *sign.UserInfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_caniplay", *v)
	id := "0"
	res := make(map[string]interface{})
	res["t"] = "caniplay"
	res["status"] = 1
	res["msg"] = "no,you can not！"

	if val, found := (*v)["id"]; found {
		id = val.(string)
		db, err0 := cfg.OpenDb() //sql.Open("adodb", cfg.Cfg["mssql"])
		if err0 != nil {
			log.Println("ServeSrs sql open error")
		} else {
			defer db.Close()
			stmt1, err0 := db.Prepare(`
				SELECT roomid,
				    createdate,
				    filepath,
				    filename,
				    nickname,
				    encrypted,
				    enckey
				FROM hislist
				where id = ?
						`)
			if err0 != nil {
				log.Println("ServeSrs sql db.Prepare error")
			} else {
				defer stmt1.Close()
				rows, err := stmt1.Query(id)
				if err != nil {
					log.Println("ServeSrs sql stmt.Query error", err.Error())
				} else {
					cip, err01 := toolset.Rows2Map(rows)

					if err01 == nil && toolset.MapGetInt("roomid", &cip[0], 0) == 999 {
						stmt02, err02 := db.Prepare(`SELECT[StartTime] ,[StopTime]
							FROM [hds12204021_db].[dbo].[dv_vcd_charge_2014]
							where UserId=?`)
						if err02 != nil {

						} else {
							defer stmt02.Close()
							row03, err03 := stmt02.Query(ui.UserID)
							if err03 != nil {

							} else {
								culs03, _ := rows.Columns()
								count03 := len(culs03)
								vals03 := make([]interface{}, count03)
								for row03.Next() {
									row03.Scan(&vals03[0], &vals03[1])
									log.Println("Start:", vals03[0], " Stop:", vals03[1])
									now03 := time.Now()
									if vals03[0].(time.Time).Before(now03) && vals03[1].(time.Time).After(now03) {
										res["data"] = cip
										res["status"] = 0
										res["msg"] = "yes, you can!"
									}
									break
								}
							}
						}

					} else {
						res["data"] = cip
						res["status"] = 0
						res["msg"] = "yes, you can!"
					}

				}
			}
		}

	}

	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

func fAuthout(ui *sign.UserInfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
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

func fModipassword(ui *sign.UserInfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	account := toolset.MapGetString("account", v, "")     //(*v)["account"].(string)
	password := toolset.MapGetString("password", v, "")   //(*v)["password"].(string)
	password1 := toolset.MapGetString("password1", v, "") //(*v)["password1"].(string)
	msg, status := sign.ModiPassword(account, password, password1)
	res["t"] = "modipassword"
	res["status"] = status
	res["msg"] = msg
	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}
