package handler

import (
	"Pic98/dbo"
	"Pic98/member"
	"encoding/json"
	"html/template"
	"io"
	"net/http"
	"os"
	"strings"
	"time"
)

var (
	actionsForm = map[string](func(*member.Userinfo, http.ResponseWriter, *http.Request, *map[string]interface{})){
		"Original": fOriginal,
	}
)

// FormAction ok
func FormAction(w http.ResponseWriter, r *http.Request) {
	data := PageData
	data.Title = "FormAction接口错误信息"
	data.Data = "FormAction接口错误信息:URL解析错误"
	param := strings.Split(r.URL.Path, "/")
	if len(param) >= 3 {
		if err := r.ParseMultipartForm(32 << 20); err == nil {
			var v1 map[string]interface{}
			data.Data = "FormAction接口错误信息:Action未注册"
			if f, ok := actionsForm[param[2]]; ok {
				data.Data = "FormAction接口错误信息:用户未认证"
				if ui, err1 := member.LoadUserinfo(r); err1 == nil {
					f(&ui, w, r, &v1)
					return
				}
			}
		} else {
			data.Data = "FormAction接口错误信息:" + err.Error()
		}
	}
	t, err := template.ParseFiles(
		"wwwroot/tpl/formaction.html",
		"wwwroot/tpl/public/header.html",
		"wwwroot/tpl/public/nav.html",
		"wwwroot/tpl/public/footer.html")
	if err != nil {
	}

	err = t.Execute(w, data)
	if err != nil {

	}
}

func fOriginal(ui *member.Userinfo, w http.ResponseWriter, r *http.Request, v *map[string]interface{}) {
	res := make(map[string]interface{})
	res["t"] = "Original"
	res["status"] = 1
	res["msg"] = ""
	idname := []string{}
	idcode := []string{}
	fs := make(map[string]string, 3)
	if r.MultipartForm != nil {
		idname = r.MultipartForm.Value["id-name"]
		idcode = r.MultipartForm.Value["id-code"]
		for key, value := range r.MultipartForm.File {
			files := value
			len := len(files)
			for i := 0; i < len; i++ {
				file, err := files[i].Open()
				defer file.Close()
				if err != nil {
					res["msg"] = err.Error()
				} else {
					path := "wwwroot/static/upload/original/" + time.Now().Format("2006-01")
					os.Mkdir(path, os.ModePerm)
					idfile := path + "/" + ui.Userguid + "_" + files[i].Filename
					cur, err := os.Create(idfile)
					defer cur.Close()
					if err != nil {
						res["msg"] = err.Error()
					} else {
						io.Copy(cur, file)
						fs[key] = idfile
					}
				}
			}
		}
	}

	if len(fs) >= 3 {
		res["msg"] = "不能重复提交申请！"
		if rows, err := dbo.Select("select userid from original where userid=? and status = 0", ui.UserID); err == nil {
			if !rows.Next() {
				mjson, _ := json.Marshal(fs)
				mString := string(mjson)
				now := time.Now()
				str := `
		INSERT INTO original
		(userid,idname,idcode,photo,createdate,status,passid,passdate)
		VALUES (?,?,?,?,?,0,0,?)
		`
				if _, err := dbo.Insert(str, ui.UserID, idname[0], idcode[0], mString, now, now); err == nil {
					res["status"] = 0
				} else {
					res["msg"] = err.Error()
				}
			}
		}
	}
	rmsg, err := json.Marshal(res)
	if err == nil {
		w.Write(rmsg)
	}
}
