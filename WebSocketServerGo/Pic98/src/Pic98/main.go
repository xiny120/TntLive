// main.go
package main

import (
	"Pic98/admin"
	"Pic98/api"
	"Pic98/cfg"
	"Pic98/dbo"
	"Pic98/handler"
	"encoding/json"
	"fmt"
	"log"
	"math/rand"
	"net/http"
	_ "net/http/pprof"
	"os"
	"path/filepath"
	_ "runtime"
	_ "strconv"
	"time"
)

func main() {
	var HTTPMux = http.NewServeMux()
	var v map[string]interface{}
	jsonstr := `{"id":13,"name":"胖胖","weight":216.5,"dd":"123"}`
	json.Unmarshal([]byte(jsonstr), &v)
	for k, v1 := range v {
		fmt.Print(k, " = ")
		switch v1.(type) {
		case int:
			fmt.Println(v1, "is an int value.")
		case string:
			fmt.Println(v1, "is a string value.")
		case int64:
			fmt.Println(v1, "is an int64 value.")
		case float64:
			fmt.Println(v1, "is an float64 value.")
		default:
			fmt.Println(v1, "is an unknown type.")
		}
	}

	//log.Println(v["id"], v["name"], v["weight"])
	go func() {
		//mux := http.NewServeMux()
		//mux.HandleFunc("/go", func(w http.ResponseWriter, r *http.Request) {
		//	num := strconv.FormatInt(int64(runtime.NumGoroutine()), 10)
		//	w.Write([]byte(num))
		//})
		//http.ListenAndServe(":6061", mux)
		http.ListenAndServe(":6060", nil)
	}()

	dir, err0 := filepath.Abs(filepath.Dir(os.Args[0]))
	if err0 != nil {
		log.Fatal(err0)
	}
	fmt.Println(dir)

	rand.Seed(time.Now().Unix())
	cfg.Cfg["tidb"] = "pic98:vck123456@tcp(106.14.145.51:4000)/Pic98"
	dbo.Init()

	rh := http.RedirectHandler("http://www.baidu.com", 307)
	HTTPMux.Handle("/baidu", rh)
	regidx := http.HandlerFunc(handler.Index)
	HTTPMux.Handle("/", regidx)
	reghotidol := http.HandlerFunc(handler.Index_Hotidol)
	HTTPMux.Handle("/Index/Hotidol", reghotidol)
	regnewidol := http.HandlerFunc(handler.Index_Newidol)
	HTTPMux.Handle("/Index/Newidol", regnewidol)
	regList := http.HandlerFunc(handler.List)
	HTTPMux.Handle("/List/", regList)
	regDetail := http.HandlerFunc(handler.Detail)
	HTTPMux.Handle("/Detail/", regDetail)
	rega := http.HandlerFunc(handler.Account)
	HTTPMux.Handle("/Account/", rega)
	regcmd := http.HandlerFunc(handler.AccountCmd)
	HTTPMux.Handle("/Account/Cmd", regcmd)
	regh := http.HandlerFunc(handler.Account_Register)
	HTTPMux.Handle("/Account/Register/", regh)
	regid := http.HandlerFunc(handler.Account_Register_Cmd)
	HTTPMux.Handle("/Account/Register/Cmd", regid)
	regLogin := http.HandlerFunc(handler.Account_Login)
	HTTPMux.Handle("/Account/Login/", regLogin)
	regPost := http.HandlerFunc(handler.Account_Post)
	HTTPMux.Handle("/Account/Post/", regPost)
	//regOriginal := http.HandlerFunc(Handler.AccountOriginal)
	//HttpMux.Handle("/Account/Original/", regOriginal)
	regPostParam := http.HandlerFunc(handler.Account_Post_Param)
	HTTPMux.Handle("/Account/Post/Param", regPostParam)
	regPostParamIdol := http.HandlerFunc(handler.Account_Post_Param_Idol)
	HTTPMux.Handle("/Account/Post/Param/Idol/", regPostParamIdol)
	regi := http.HandlerFunc(handler.Image)
	HTTPMux.Handle("/Image/", regi)
	regiv := http.HandlerFunc(handler.Image_Vip)
	HTTPMux.Handle("/Image/Vip/", regiv)
	regif := http.HandlerFunc(handler.Image_Free)
	HTTPMux.Handle("/Image/free/", regif)
	regibanner := http.HandlerFunc(handler.Image_Banner)
	HTTPMux.Handle("/Image/Banner/", regibanner)
	regiu := http.HandlerFunc(handler.Image_Update)
	HTTPMux.Handle("/Image/Update", regiu)

	makeThumbnail := http.HandlerFunc(handler.MakeThumbnail)
	HTTPMux.Handle("/MakeThumbnail/", makeThumbnail)

	thumbnail := http.HandlerFunc(handler.Thumbnail)
	HTTPMux.Handle("/thumbnail/", thumbnail)

	phpUeditorController := http.HandlerFunc(handler.PhpUeditorController)
	HTTPMux.Handle("/php_ueditor_controller/", phpUeditorController)

	topicHandler := http.HandlerFunc(handler.Topic)
	HTTPMux.Handle("/topic/", topicHandler)
	formActionHandler := http.HandlerFunc(handler.FormAction)
	HTTPMux.Handle("/FormAction/", formActionHandler)
	log.Println("请用浏览器打开 http://127.0.0.1:3160 ...")
	HTTPMux.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("wwwroot/static"))))

	//sr := r.PathPrefix("/admin")
	adminh := http.HandlerFunc(admin.ServeAdmin)
	HTTPMux.Handle("/admin/", adminh)

	apiPrivate := http.HandlerFunc(api.Private)
	HTTPMux.Handle("/api/private/", apiPrivate)
	apiAdmin := http.HandlerFunc(api.Admin)
	HTTPMux.Handle("/api/admin/", apiAdmin)

	err := http.ListenAndServe(":3160", HTTPMux)
	log.Println(err)

}
