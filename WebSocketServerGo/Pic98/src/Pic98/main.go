// main.go
package main

import (
	"Pic98/admin"
	"Pic98/api"
	"Pic98/cfg"
	"Pic98/dbo"
	"Pic98/handler"
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

var HttpMux = http.NewServeMux()

func main() {
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
	HttpMux.Handle("/baidu", rh)
	regidx := http.HandlerFunc(handler.Index)
	HttpMux.Handle("/", regidx)
	reghotidol := http.HandlerFunc(handler.Index_Hotidol)
	HttpMux.Handle("/Index/Hotidol", reghotidol)
	regnewidol := http.HandlerFunc(handler.Index_Newidol)
	HttpMux.Handle("/Index/Newidol", regnewidol)
	regList := http.HandlerFunc(handler.List)
	HttpMux.Handle("/List/", regList)
	regDetail := http.HandlerFunc(handler.Detail)
	HttpMux.Handle("/Detail/", regDetail)
	rega := http.HandlerFunc(handler.Account)
	HttpMux.Handle("/Account/", rega)
	regcmd := http.HandlerFunc(handler.AccountCmd)
	HttpMux.Handle("/Account/Cmd", regcmd)
	regh := http.HandlerFunc(handler.Account_Register)
	HttpMux.Handle("/Account/Register/", regh)
	regid := http.HandlerFunc(handler.Account_Register_Cmd)
	HttpMux.Handle("/Account/Register/Cmd", regid)
	regLogin := http.HandlerFunc(handler.Account_Login)
	HttpMux.Handle("/Account/Login/", regLogin)
	regPost := http.HandlerFunc(handler.Account_Post)
	HttpMux.Handle("/Account/Post/", regPost)
	//regOriginal := http.HandlerFunc(Handler.AccountOriginal)
	//HttpMux.Handle("/Account/Original/", regOriginal)
	regPostParam := http.HandlerFunc(handler.Account_Post_Param)
	HttpMux.Handle("/Account/Post/Param", regPostParam)
	regPostParamIdol := http.HandlerFunc(handler.Account_Post_Param_Idol)
	HttpMux.Handle("/Account/Post/Param/Idol/", regPostParamIdol)
	regi := http.HandlerFunc(handler.Image)
	HttpMux.Handle("/Image/", regi)
	regiv := http.HandlerFunc(handler.Image_Vip)
	HttpMux.Handle("/Image/Vip/", regiv)
	regif := http.HandlerFunc(handler.Image_Free)
	HttpMux.Handle("/Image/free/", regif)
	regibanner := http.HandlerFunc(handler.Image_Banner)
	HttpMux.Handle("/Image/Banner/", regibanner)
	regiu := http.HandlerFunc(handler.Image_Update)
	HttpMux.Handle("/Image/Update", regiu)

	makeThumbnail := http.HandlerFunc(handler.MakeThumbnail)
	HttpMux.Handle("/MakeThumbnail/", makeThumbnail)

	thumbnail := http.HandlerFunc(handler.Thumbnail)
	HttpMux.Handle("/thumbnail/", thumbnail)

	php_ueditor_controller := http.HandlerFunc(handler.Php_ueditor_controller)
	HttpMux.Handle("/php_ueditor_controller/", php_ueditor_controller)

	topicHandler := http.HandlerFunc(handler.Topic)
	HttpMux.Handle("/topic/", topicHandler)
	formActionHandler := http.HandlerFunc(handler.FormAction)
	HttpMux.Handle("/FormAction/", formActionHandler)
	log.Println("请用浏览器打开 http://127.0.0.1:3160 ...")
	HttpMux.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("wwwroot/static"))))

	//sr := r.PathPrefix("/admin")
	adminh := http.HandlerFunc(admin.ServeAdmin)
	HttpMux.Handle("/admin/", adminh)

	apiPrivate := http.HandlerFunc(api.Private)
	HttpMux.Handle("/api/private/", apiPrivate)

	err := http.ListenAndServe(":3160", HttpMux)
	log.Println(err)

}
