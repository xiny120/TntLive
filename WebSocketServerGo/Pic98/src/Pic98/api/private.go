package api

import (
	"Pic98/dbo"
	"Pic98/member"

	//"Pic98/toolset"
	"encoding/json"
	"log"
	"net/http"

	"github.com/xiny120/gotoolset"
)

var (
	actionsPrivateAPI = map[string](func(*member.Userinfo, http.ResponseWriter, *http.Request, *map[string]interface{})){
		"original-review-list":              fOriginalReviewList,
		"original-review-allow":             fOriginalReviewAllow,
		"original-search-list":              fOriginalSearchList,
		"original-search-originalCancel":    fOriginalSearchOriginalCancel,
		"article-search-list":               fArticleSearchList,
		"article-review-search-list":        fArticleReviewSearchList,
		"article-review-search-articlepass": fArticleReviewSearchPass,
	}
)

// fArticleReviewSearchArticlePass ok
func fArticleReviewSearchPass(ui *member.Userinfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "article-review-search-articlepass"
	res["status"] = 1
	res["msg"] = "未知错误"

	func() {
		aguid := ddmtoolset.MapGetString("aguid", v, "") //(*v)["data"].([]interface{})
		status := ddmtoolset.MapGetInt("status", v, -1)
		if status == -1 {
			res["msg"] = "status 解析错误"
			return
		}
		dbo.ArticleReviewSearchPass(aguid, status)
		res["data"] = ""
		res["status"] = 0
		res["msg"] = "原创作者取消成功！"
	}()

	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

// fArticleReviewSearchList ok
func fArticleReviewSearchList(ui *member.Userinfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "article-review-search-list"
	res["status"] = 1
	res["msg"] = "未知错误"

	func() {
		rows, err := dbo.Select(`
		SELECT a.aguid, a.coverimg, a.createtime, a.likesum, a.userguid, a.title, a.intro, a.tags,a.content,
		b.nick_name  FROM topic_review a, userinfo b where a.userguid = b.userguid
		order by createtime asc
		limit 0,1
		`)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		data, err := ddmtoolset.Rows2Map(rows)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		res["data"] = data
		res["status"] = 0
		res["msg"] = "success"
	}()

	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

// fArticleSearchList ok
func fArticleSearchList(ui *member.Userinfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "fArticleSearchList"
	res["status"] = 1
	res["msg"] = "未知错误"

	func() {
		atable := map[int]string{0: "topic", 1: "topic_review", 2: "topic_reject"}

		pageidx := ddmtoolset.MapGetInt("pageidx", v, 1) - 1
		articleType := ddmtoolset.MapGetInt("articleType", v, 0)
		pageSize := 50
		startIdx := pageidx * pageSize
		if startIdx < 0 {
			startIdx = 0
		}
		rows, err := dbo.Select(`
		SELECT a.aguid, a.coverimg, a.createtime, a.likesum, a.userguid, a.title, a.intro, a.tags,
		b.nick_name  FROM `+atable[articleType]+` a, userinfo b where a.userguid = b.userguid and a.userguid=?
		order by createtime desc
		limit ?,?
		`, ui.Userguid, startIdx, pageSize)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		data, err := ddmtoolset.Rows2Map(rows)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		res["data"] = data

		rows, err = dbo.Select(`SELECT count(*) as count FROM `+atable[articleType]+` where userguid=?`, ui.Userguid)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		data0, err := ddmtoolset.Rows2Map(rows)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		res["total"] = data0
		res["pagesize"] = pageSize
		res["status"] = 0
		res["msg"] = "success"
	}()

	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

func fOriginalSearchOriginalCancel(ui *member.Userinfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "original-review-allow"
	res["status"] = 1
	res["msg"] = "未知错误"

	func() {
		arr := (*v)["data"].([]interface{})
		for _, val := range arr {
			switch val.(type) {
			case string:
				res["status"] = 0
				id := val.(string)
				dbo.OriginalReviewAllow(id, 2)
				res["data"] = ""
				res["status"] = 0
				res["msg"] = "原创作者取消成功！"
			default:
				res["msg"] = "参数错误"
			}
		}
	}()

	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

func fOriginalSearchList(ui *member.Userinfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "original-search-list"
	res["status"] = 1
	res["msg"] = "未知错误"

	func() {
		idname := (*v)["idname"].(string)
		idcode := (*v)["idcode"].(string)
		userid := (*v)["userid"].(string)
		nickname := (*v)["nick_name"].(string)
		pageidx := int((*v)["pageidx"].(float64))
		pageSize := 50
		startIdx := pageidx * pageSize
		data := [](map[string]interface{}){}
		if idname == "" && idcode == "" && userid == "" && nickname == "" {
			rows, err := dbo.Select(`
			SELECT a.id,
			b.nick_name,
			a.idname,
			a.idcode,
			a.photo,
			a.createdate,
			a.userid
			FROM original a,userinfo b
			where a.userid = b.userid and a.status = 1 
			
			order by a.createdate asc
			LIMIT ?,?
			`, startIdx, pageSize)
			if err != nil {
				res["msg"] = err.Error()
				return
			}
			data0, err := ddmtoolset.Rows2Map(rows)
			if err != nil {
				res["msg"] = err.Error()
				return
			}

			data, _ = ddmtoolset.RowMap2RowMap(&data, &data0)

		}

		if nickname != "" {
			rows, err := dbo.Select(`
			SELECT a.id,
			b.nick_name,
			a.idname,
			a.idcode,
			a.photo,
			a.createdate,
			a.userid
			FROM original a,userinfo b
			where a.userid = b.userid and a.status = 1 and b.nick_name like ? 
			
			order by a.createdate asc
			LIMIT ?,?
			`, "%"+nickname+"%", startIdx, pageSize)
			if err != nil {
				res["msg"] = err.Error()
				return
			}
			data0, err := ddmtoolset.Rows2Map(rows)
			if err != nil {
				res["msg"] = err.Error()
				return
			}

			data, _ = ddmtoolset.RowMap2RowMap(&data, &data0)

		}
		if idname != "" {
			rows, err := dbo.Select(`
			SELECT a.id,
			b.nick_name,
			a.idname,
			a.idcode,
			a.photo,
			a.createdate,
			a.userid
			FROM original a,userinfo b
			where a.userid = b.userid and a.status = 1 and a.idname like ? 
			
			order by a.createdate asc
			LIMIT ?,?
			`, "%"+idname+"%", startIdx, pageSize)
			if err != nil {
				res["msg"] = err.Error()
				return
			}
			data0, err := ddmtoolset.Rows2Map(rows)
			if err != nil {
				res["msg"] = err.Error()
				return
			}

			data, _ = ddmtoolset.RowMap2RowMap(&data, &data0)

		}

		if idcode != "" {
			rows, err := dbo.Select(`
			SELECT a.id,
			b.nick_name,
			a.idname,
			a.idcode,
			a.photo,
			a.createdate,
			a.userid
			FROM original a,userinfo b
			where a.userid = b.userid and a.status = 1 and a.idcode like ? 
			
			order by a.createdate asc
			LIMIT ?,?
			`, "%"+idcode+"%", startIdx, pageSize)
			if err != nil {
				res["msg"] = err.Error()
				return
			}
			data0, err := ddmtoolset.Rows2Map(rows)
			if err != nil {
				res["msg"] = err.Error()
				return
			}

			data, _ = ddmtoolset.RowMap2RowMap(&data, &data0)
		}

		if userid != "" {
			rows, err := dbo.Select(`
			SELECT a.id,
			b.nick_name,
			a.idname,
			a.idcode,
			a.photo,
			a.createdate,
			a.userid
			FROM original a,userinfo b
			where a.userid = b.userid and a.status = 1 and a.userid = ? 
			
			order by a.createdate asc
			LIMIT ?,?
			`, userid, startIdx, pageSize)
			if err != nil {
				res["msg"] = err.Error()
				return
			}
			data0, err := ddmtoolset.Rows2Map(rows)
			if err != nil {
				res["msg"] = err.Error()
				return
			}

			data, _ = ddmtoolset.RowMap2RowMap(&data, &data0)
		}

		rows, err := dbo.Select(`SELECT count(*) as count FROM original where status = 1`)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		data0, err := ddmtoolset.Rows2Map(rows)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		res["data"] = data
		res["total"] = data0
		res["pagesize"] = pageSize
		res["status"] = 0
		res["msg"] = "success"
	}()

	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

func fOriginalReviewAllow(ui *member.Userinfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "original-review-allow"
	res["status"] = 1
	res["msg"] = "未知错误"

	func() {
		arr := (*v)["data"].([]interface{})
		for _, val := range arr {
			switch val.(type) {
			case string:
				res["status"] = 0
				id := val.(string)
				status := (*v)["status"]
				dbo.OriginalReviewAllow(id, int(status.(float64)))
				res["data"] = ""
				res["status"] = 0
				res["msg"] = "原创作者申请审核成功！"
			default:
				res["msg"] = "参数错误"
			}
		}
	}()

	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

func fOriginalReviewList(ui *member.Userinfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "original-review-list"
	res["status"] = 1
	res["msg"] = "未知错误"

	func() {
		rows, err := dbo.Select(`
		SELECT a.id,
		b.nick_name,
		a.idname,
		a.idcode,
		a.photo,
		a.createdate
		FROM original a,userinfo b
		where a.userid = b.userid and a.status = 0
		
		order by a.createdate asc
		LIMIT 0,50
		`)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		data, err := ddmtoolset.Rows2Map(rows)
		if err != nil {
			res["msg"] = err.Error()
			return
		}
		res["data"] = data
		res["status"] = 0
		res["msg"] = "success"
	}()

	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}

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
	err := json.NewDecoder(r.Body).Decode(&v)
	if err != nil {
		res := make(map[string]interface{})
		res["t"] = "original-review-list"
		res["status"] = 1
		res["msg"] = "提交的json参数解析失败"
		rmsg, err := json.Marshal(res)
		if err == nil {
			w.Write(rmsg)
		}
		return
	}
	v1 := v.(map[string]interface{})
	if ui, err1 := member.LoadUserinfo(r); err1 == nil {
		action := ddmtoolset.MapGetString("action", &v1, "")
		if f, ok := actionsPrivateAPI[action]; ok {
			f(&ui, w, r, &v1)
			return
		}
		log.Println("action not found:", action)
		return
	}
	http.Error(w, "StatusUnauthorized", http.StatusUnauthorized)
}
