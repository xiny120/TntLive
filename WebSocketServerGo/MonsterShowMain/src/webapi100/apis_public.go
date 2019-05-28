package webapi100

import (
	"cfg"
	"database/sql"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"time"
	"ucenter"

	"github.com/gorilla/mux"
)

var (
	actions = map[string](func(http.ResponseWriter, *http.Request, *map[string]interface{})){
		"auth":         f_auth,
		"roomlist":     f_roomlist,
		"medialist":    f_medialist,
		"medialistnew": f_medialistnew,
		"wxjsinit":     f_wxjsinit,
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
	res := make(map[string]interface{})
	res["msg"] = "unknow"
	res["status"] = 0
	var v interface{}
	err := json.NewDecoder(r.Body).Decode(&v)
	log.Println("serveTest - ", r.URL, " - ", v)
	if err != nil {
		//w.Write([]byte("{\"status\":1,\"msg\":\"提交的json参数解析失败\"}"))
		res["msg"] = "提交的json参数解析失败"
		res["status"] = 1
		rmsg, err := json.Marshal(res)
		if err == nil {
			w.Write(rmsg)
		}
	} else {
		v1 := v.(map[string]interface{})
		action := v1["action"].(string)

		if f, ok := actions[action]; ok {

			f(w, r, &v1)
		} else {
			//w.Write([]byte("{\"status\":1,\"msg\":\"no api\"}"))
			//http.Error(w, "NotFound", http.StatusNotFound)
			res["msg"] = "no api"
			res["status"] = 1
			rmsg, err := json.Marshal(res)
			if err == nil {
				w.Write(rmsg)
			}
		}
	}
}

type wxjsinit struct {
	AppId     string //`json:"appId"`
	Timestamp string //`json:"timestamp"`
	NonceStr  string //`json:"nonceStr"`
	Signature string //`json:"signature"`
}

func f_wxjsinit(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_wxjsinit", *v)
	res := make(map[string]interface{})
	res["t"] = "wxjsinit"
	res["status"] = 0
	//at := GetAccessToken()
	//tkt := GetWeiXinJsapi_Ticket()
	//nostr := CreatenNonce_str()
	//ts := CreatenTimestamp()

	res["msg"] = "注销成功！"
	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

func f_auth(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	//log.Println("f_auth", *v)
	res := make(map[string]interface{})
	account := (*v)["account"].(string)
	password := (*v)["password"].(string)
	tt := sign.SignIn(account, password)
	sign.SessionsSet(tt.SessionId, tt)
	wbuf, werr := json.Marshal(tt)
	if werr == nil {
		tfile := "./tokens/" + tt.SessionId
		f, err3 := os.Create(tfile) //创建文件
		log.Println("f_auth", tfile, err3)
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
	ris := []roomitem{}
	result := ""
	page := int((*v)["page"].(float64)) - 1
	per_page := int((*v)["per_page"].(float64))
	db, err0 := sql.Open("adodb", cfg.Cfg["mssql"])
	if err0 != nil {
		result = "f_roomlist sql open error"
	} else {
		defer db.Close()
		sqlstr := fmt.Sprintf("SELECT TOP (%d) [Id], [Title], [Intro], [Icon], [Corver], [Background], [Follow], [Online],  [Type], [PullUri], [OrderIdx] FROM  [Web2019_roomlist] where [orderidx] >= %d and bDeleted=0 order by [orderidx] asc", per_page, page*per_page)
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

type mediaitem struct {
	Id           string //`json:"id"`
	RoomName     string //`json:"roomname"`
	CreateDate   string //`json:"createdate"`
	PublisherId  string //`json:"PublisherId"`
	FileName     string //`json:"filename"`
	NickName     string //`json:"title"`
	FilePath     string //`json:"filepath"`
	FileSize     int64  //`json:"follow"`
	Followed     int64  //`json:"onlines"`
	Readed       int64  //`json:"intro"`
	Encryptioned int32  //`json:"encryptioned"`
}

func f_medialist(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	//log.Println("f_medialist", *v)
	roomid := "{96518478-BE8D-4EEE-9FEC-69D472CED4DC}"
	orderby := " asc "
	pageid := 1
	CreateDate := "3030-12-31"
	if val, found := (*v)["roomid"]; found {
		roomid = val.(string)
	}
	if val, found := (*v)["orderby"]; found {
		orderby = val.(string)
	}
	if val, found := (*v)["pageid"]; found {
		pageid = int(val.(float64))
	}
	if val, found := (*v)["CreateDate"]; found {
		CreateDate = val.(string)
	}

	pagesize := "20"
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
		/*
			sqlstr := `
				select top ` + pagesize + ` *
				from (select a.[Id], a.[CreateDate],a.[PublisherId],a.[NickName],a.[FileSize],a.[Followed],
				a.[Readed],a.[FilePath], b.[STitle],row_number() over(order by a.` + orderby + ` ) as rownumber,
				a.[Encryptioned],a.[FileName]
				from [hds12204021_db].[dbo].[Web2019_historylist] a, [hds12204021_db].[dbo].[Web2019_roomlist] b
				where a.roomid = '` + roomid + `' and a.Deleted = 0 and a.roomid=b.id and a.[CreateDate] < CONVERT(datetime,'` + CreateDate + `')) temp_row
				where rownumber>((?-1)*?)
			`
		*/
		sqlstr := `
			select top ` + pagesize + `
			a.[Id], a.[CreateDate],a.[PublisherId],a.[NickName],a.[FileSize],a.[Followed],
			a.[Readed],a.[FilePath], b.[STitle],a.[CreateDate],a.[Encryptioned],a.[FileName]
			from [hds12204021_db].[dbo].[Web2019_historylist] a, [hds12204021_db].[dbo].[Web2019_roomlist] b
			where a.roomid = '` + roomid + `' and a.Deleted = 0 and a.roomid=b.id and a.[CreateDate] < CONVERT(datetime,'` + CreateDate + `')
			order by a.` + orderby + `
		`

		stmt1, err0 := db.Prepare(sqlstr)
		if err0 != nil {
			log.Println("ServeSrs sql db.Prepare error" + err0.Error())
		} else {
			defer stmt1.Close()
			rows, err := stmt1.Query() //pageid, pagesize)
			if err != nil {
				log.Println("ServeSrs sql stmt.Query error", err.Error(), pageid, pagesize, roomid)
			} else {
				culs, _ := rows.Columns()
				count := len(culs)
				vals := make([]interface{}, count)
				for rows.Next() {
					ri := mediaitem{}
					rows.Scan(&vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &vals[6], &vals[7], &vals[8], &vals[9], &vals[10], &vals[11])
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
					if vals[7] != nil {
						ri.FilePath = (vals[7].(string))
					}
					if vals[8] != nil {
						ri.RoomName = (vals[8].(string))
					}
					if vals[10] != nil {
						ri.Encryptioned = int32(vals[10].(int64))
					}
					if vals[11] != nil {
						ri.FileName = (vals[11].(string))
					}
					mis = append(mis, ri)
				}
			}
		}
	}

	res["medialist"] = mis
	res["msg"] = ""
	rmsg, err := json.Marshal(res)
	//log.Println(rmsg)
	if err == nil {
		w.Write(rmsg)
	}
}

func f_medialistnew(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	//log.Println("f_medialist", *v)
	roomid := "{96518478-BE8D-4EEE-9FEC-69D472CED4DC}"
	orderby := "CreateDate"
	pageid := 1
	CreateDate := ""
	if val, found := (*v)["roomid"]; found {
		roomid = val.(string)
	}
	if val, found := (*v)["orderby"]; found {
		orderby = val.(string)
	}
	if val, found := (*v)["pageid"]; found {
		pageid = int(val.(float64))
	}
	if val, found := (*v)["CreateDate"]; found {
		CreateDate = val.(string)
	}

	pagesize := "20"
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
		sqlstr := `
			select top ` + pagesize + ` *
			from (select a.[Id], a.[CreateDate],a.[PublisherId],a.[NickName],a.[FileSize],a.[Followed],
			a.[Readed],a.[FilePath], b.[STitle],row_number() over(order by a.` + orderby + ` ) as rownumber,
			a.[Encryptioned],a.[FileName]
			from [hds12204021_db].[dbo].[Web2019_historylist] a, [hds12204021_db].[dbo].[Web2019_roomlist] b
			where a.roomid = '` + roomid + `' and a.Deleted = 0 and a.roomid=b.id and a.[CreateDate] > CONVERT(datetime,'` + CreateDate + `')) temp_row
			where rownumber>((?-1)*?)
		`
		log.Println(sqlstr)
		stmt1, err0 := db.Prepare(sqlstr)
		if err0 != nil {
			log.Println("ServeSrs sql db.Prepare error" + err0.Error())
		} else {
			defer stmt1.Close()
			rows, err := stmt1.Query(pageid, pagesize)
			if err != nil {
				log.Println("ServeSrs sql stmt.Query error", err.Error(), pageid, pagesize, roomid)
			} else {
				culs, _ := rows.Columns()
				count := len(culs)
				vals := make([]interface{}, count)
				for rows.Next() {
					ri := mediaitem{}
					rows.Scan(&vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &vals[6], &vals[7], &vals[8], &vals[9], &vals[10], &vals[11])
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
					if vals[7] != nil {
						ri.FilePath = (vals[7].(string))
					}
					if vals[8] != nil {
						ri.RoomName = (vals[8].(string))
					}
					if vals[10] != nil {
						ri.Encryptioned = int32(vals[10].(int64))
					}
					if vals[11] != nil {
						ri.FileName = (vals[11].(string))
					}
					mis = append(mis, ri)
				}
			}
		}
	}

	res["medialist"] = mis
	res["msg"] = ""
	rmsg, err := json.Marshal(res)
	//log.Println(rmsg)
	if err == nil {
		w.Write(rmsg)
	}
}
