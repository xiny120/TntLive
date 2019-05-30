package webapi100

import (
	"cfg"
	"encoding/json"

	//"fmt"
	"log"
	"net/http"
	"os"
	"time"
	"toolset"
	sign "ucenter"

	uuid "github.com/satori/go.uuid"
)

var (
	actions = map[string](func(http.ResponseWriter, *http.Request, *map[string]interface{})){
		"auth":         fAuth,
		"reg":          fReg,
		"roomlist":     fRoomlist,
		"medialist":    fMedialist,
		"medialistnew": fMedialistnew,
		"wxjsinit":     fWxjsinit,
	}
)

// Public ok
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
	AppID     string //`json:"appId"`
	Timestamp string //`json:"timestamp"`
	NonceStr  string //`json:"nonceStr"`
	Signature string //`json:"signature"`
}

func fWxjsinit(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
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

func fReg(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "sign in"
	res["status"] = 1
	res["msg"] = "用户名密码不能为空！"
	account := toolset.MapGetString("account", v, "")
	password := toolset.MapGetString("password", v, "")
	email := toolset.MapGetString("email", v, "")
	cellphone := toolset.MapGetString("cellphone", v, "")

	res["msg"] = func() string {
		if account == "" || password == "" {
			return "用户名密码不能为空！"
		}
		db, err0 := cfg.OpenDb()
		if err0 != nil {
			return "f_reg sql open error"
		}
		defer db.Close()
		stmt, err0 := db.Prepare(`
			SELECT userguid 
			FROM useridentify
			where userid=?			
		`)
		if err0 != nil {
			return "f_reg sql db.Prepare error"
		}
		defer stmt.Close()
		rows, err := stmt.Query(account)
		if err != nil {
			return "f_reg sql stmt.Query error" + account + password + email + cellphone
		}
		if rows.Next() {
			return "用户名已经被占用！"
		}

		sid, _ := uuid.NewV4()

		stmti, err1 := db.Prepare(`
			INSERT INTO useridentify
			(userguid, userid, createtime, LastSignin, LastIp)
			VALUES (?,?,?,?,?)
		`)
		if err1 != nil {
			return "f_reg sql db.Prepare error"
		}
		defer stmti.Close()
		_, err2 := stmti.Exec(sid, account, time.Now(), time.Now(), r.RemoteAddr)
		if err2 != nil {
			return "f_reg sql stmti.Exec error"
		}

		stmtu, err3 := db.Prepare(`
			INSERT INTO userinfo
			(userguid, email,cellphone, password, create_time, age, sex, nick_name, follow, idol, writer)
			VALUES (?,?,?,?,?,?,?,?,?,?,?)
		`)
		defer stmtu.Close()
		_, err3 = stmtu.Exec(sid, email, cellphone, password, time.Now(), 0, 0, account, 0, 0, 0)
		if err3 != nil {
			stmti.Exec("delete from useridentify where userid=?", account)
			return "f_reg sql stmti.Exec error"
		}
		res["status"] = 0
		return "注册成功！"
	}()
	//res["msg"] = result
	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

func fAuth(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	account := toolset.MapGetString("account", v, "")   //(*v)["account"].(string)
	password := toolset.MapGetString("password", v, "") //(*v)["password"].(string)
	tt, _ := sign.In(account, password)
	sign.SessionsSet(tt.SessionID, tt)
	wbuf, werr := json.Marshal(tt)
	if werr == nil {
		tfile := "./tokens/" + tt.SessionID
		f, err3 := os.Create(tfile) //创建文件
		log.Println("f_auth", tfile, err3)
		if err3 == nil {
			defer f.Close()
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

func fRoomlist(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	log.Println("f_roomlist", *v)
	res := make(map[string]interface{})
	res["t"] = "f_roomlist"
	res["status"] = 1
	res["msg"] = "未知错误！"
	ris := [](map[string]interface{}){}

	res["msg"] = func() string {
		db, err0 := cfg.OpenDb()
		if err0 != nil {
			return "f_roomlist sql open error"
		}
		defer db.Close()
		stmt, err0 := db.Prepare(`
			SELECT roomid, title, intro, icon, corver, background, follow, online,
			    type, ownerguid, orderidx, vhost,vport, vapp, vstream, deleted ,stitle, createdate
			FROM roomlist
			where deleted = 0
			order by createdate desc
		`)
		if err0 != nil {
			return "f_roomlist sql db.Prepare error"
		}
		defer stmt.Close()
		rows, err := stmt.Query()
		if err != nil {
			return "f_roomlist sql stmt.Query error"
		}
		ris, err = toolset.Rows2Map(rows)
		return "登录成功"
	}()

	res["roomlist"] = ris

	rmsg, err := json.Marshal(ris)
	if err == nil {
		w.Write(rmsg)
	}

}

type mediaitem struct {
	ID           string //`json:"id"`
	RoomName     string //`json:"roomname"`
	CreateDate   string //`json:"createdate"`
	PublisherID  string //`json:"PublisherId"`
	FileName     string //`json:"filename"`
	NickName     string //`json:"title"`
	FilePath     string //`json:"filepath"`
	FileSize     int64  //`json:"follow"`
	Followed     int64  //`json:"onlines"`
	Readed       int64  //`json:"intro"`
	Encryptioned int32  //`json:"encryptioned"`
}

func fMedialist(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	roomid := toolset.MapGetString("roomid", v, "{96518478-BE8D-4EEE-9FEC-69D472CED4DC}")
	pageid := toolset.MapGetInt("pageid", v, 0)
	CreateDate := toolset.MapGetString("CreateDate", v, "3030-12-31")
	pagesize := 20
	res := make(map[string]interface{})
	res["t"] = "medialist"
	res["status"] = 0
	res["msg"] = ""
	mis := []mediaitem{}
	db, err0 := cfg.OpenDb()
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
			sqlstr := `
				select top ` + pagesize + `
				a.[Id], a.[CreateDate],a.[PublisherId],a.[NickName],a.[FileSize],a.[Followed],
				a.[Readed],a.[FilePath], b.[STitle],a.[CreateDate],a.[Encryptioned],a.[FileName]
				from [hds12204021_db].[dbo].[Web2019_historylist] a, [hds12204021_db].[dbo].[Web2019_roomlist] b
				where a.roomid = '` + roomid + `' and a.Deleted = 0 and a.roomid=b.id and a.[CreateDate] < CONVERT(datetime,'` + CreateDate + `')
				order by a.` + orderby + `
				`
		*/
		sqlstr := `
			select top 20
			a.[Id], a.[CreateDate],a.[PublisherId],a.[NickName],a.[FileSize],a.[Followed],
			a.[Readed],a.[FilePath], b.[STitle],a.[CreateDate],a.[Encryptioned],a.[FileName]
			from [hds12204021_db].[dbo].[Web2019_historylist] a, [hds12204021_db].[dbo].[Web2019_roomlist] b
			where a.roomid = ? and a.Deleted = 0 and a.roomid=b.id and a.[CreateDate] < CONVERT(datetime,'` + CreateDate + `')
			order by a.CreateDate desc
		`
		log.Println(sqlstr)
		stmt1, err0 := db.Prepare(sqlstr)
		if err0 != nil {
			log.Println("ServeSrs sql db.Prepare error" + err0.Error())
		} else {
			defer stmt1.Close()
			rows, err := stmt1.Query(roomid) //, CreateDate) //, "  ")
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
						ri.ID = vals[0].(string)
					}
					if vals[1] != nil {
						ri.CreateDate = (vals[1].(time.Time)).Format(("2006-01-02 15:04:05.000"))
					}
					if vals[2] != nil {
						ri.PublisherID = vals[2].(string)
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

func fMedialistnew(w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	//log.Println("f_medialist", *v)
	roomid := toolset.MapGetString("roomid", v, "{96518478-BE8D-4EEE-9FEC-69D472CED4DC}")
	//orderby := "CreateDate"
	pageid := toolset.MapGetInt("pageid", v, 1)
	CreateDate := toolset.MapGetString("CreateDate", v, "3030-12-31")
	pagesize := 20
	res := make(map[string]interface{})
	res["t"] = "medialist"
	res["status"] = 0
	res["msg"] = ""
	mis := []mediaitem{}
	log.Println("medialistnew CreateDate", CreateDate)
	db, err0 := cfg.OpenDb() //sql.Open("adodb", cfg.Cfg["mssql"])
	if err0 != nil {
		log.Println("ServeSrs sql open error")
	} else {
		defer db.Close()
		sqlstr := `
			select 
			a.[Id], a.[CreateDate],a.[PublisherId],a.[NickName],a.[FileSize],a.[Followed],
			a.[Readed],a.[FilePath], b.[STitle],a.[CreateDate],a.[Encryptioned],a.[FileName]
			from [hds12204021_db].[dbo].[Web2019_historylist] a, [hds12204021_db].[dbo].[Web2019_roomlist] b
			where a.roomid = ? and a.Deleted = 0 and a.roomid=b.id and a.[CreateDate] > CONVERT(datetime,'` + CreateDate + `')
			order by a.CreateDate desc
		`
		log.Println(sqlstr)
		stmt1, err0 := db.Prepare(sqlstr)
		if err0 != nil {
			log.Println("ServeSrs sql db.Prepare error" + err0.Error())
		} else {
			defer stmt1.Close()
			rows, err := stmt1.Query(roomid) //, CreateDate)
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
						ri.ID = vals[0].(string)
					}
					if vals[1] != nil {
						ri.CreateDate = (vals[1].(time.Time)).Format(("2006-01-02 15:04:05.000"))
					}
					if vals[2] != nil {
						ri.PublisherID = vals[2].(string)
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
