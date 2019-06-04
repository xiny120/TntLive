package Handler

import (
	_ "encoding/base64"
	_ "encoding/json"
	"fmt"
	_ "image/jpeg"
	"io"
	"io/ioutil"
	_ "log"
	"math/rand"
	"net/http"
	"os"
	_ "path"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/disintegration/imaging"
	_ "github.com/nfnt/resize"
)

func Php_ueditor_controller(w http.ResponseWriter, r *http.Request) {

	var result string
	result = "{\"state\":\"SUCCESS\"}"
	err := r.ParseForm()
	if err != nil {
		result = "{\"state\":\"fail\",\"msg\":\"WebApi Account/Register/Cmd ParseForm失败\"}"
	} else {
		op := r.FormValue("action")
		switch op {
		case "config": //这里是conf/config.json
			file, err := os.Open("wwwroot/static/3rd/UMeditor/utf8-php/php/config.json")
			if err != nil {
				//log.Fatal(err)
				//os.Exit(1)
			}
			defer file.Close()
			fd, err := ioutil.ReadAll(file)
			src := string(fd)
			re, _ := regexp.Compile("\\/\\*[\\S\\s]+?\\*\\/") //参考php的$CONFIG = json_decode(preg_replace("/\/\*[\s\S]+?\*\//", "", file_get_contents("config.json")), true);
			//将php中的正则移植到go中，需要将/ \/\*[\s\S]+?\*\/  /去掉前后的/，然后将\改成2个\\
			//参考//去除所有尖括号内的HTML代码，并换成换行符
			// re, _ = regexp.Compile("\\<[\\S\\s]+?\\>")
			// src = re.ReplaceAllString(src, "\n")
			//当把<和>换成/*和*\时，斜杠/和*之间加双斜杠\\才行。
			src = re.ReplaceAllString(src, "")
			////tt := []byte(src)
			////var r0 interface{}
			////json.Unmarshal(tt, &r0) //这个byte要解码
			//c.Data["json"] = r
			//c.ServeJson()
			result = src

			//下面这段是测试用的
			// b := []byte(`{
			//             "imageActionName": "uploadimage",
			//             "imageFieldName": "upfile",
			//             "imageMaxSize": 2048000,
			//             "imageAllowFiles": [".png", ".jpg", ".jpeg", ".gif", ".bmp"],
			//             "imageCompressEnable": true,
			//             "imageCompressBorder": 1600,
			//             "imageInsertAlign": "none",
			//             "imageUrlPrefix": "",
			//             "imagePathFormat": "/static/upload/{yyyy}{mm}{dd}/{time}{rand:6}"
			//       }`)
			// var r interface{}
			// json.Unmarshal(b, &r)
			// c.Data["json"] = r
			// c.ServeJson()

		case "uploadimage", "uploadfile", "uploadvideo":
			path0 := "./wwwroot/Image/Vip/" + time.Now().Format("2006/01/")
			pathstrd0 := strings.Replace(path0, "wwwroot", "wwwroot/thumbnail/", 1)
			err := os.MkdirAll(pathstrd0, 0777) //..代表本当前exe文件目录的上级，.表示当前目录，没有.表示盘的根目录
			err = os.MkdirAll(path0, 0777)      //..代表本当前exe文件目录的上级，.表示当前目录，没有.表示盘的根目录
			if err != nil {
				//beego.Error(err)
			}
			//保存上传的图片
			//获取上传的文件，直接可以获取表单名称对应的文件名，不用另外提取
			file, h, err := r.FormFile("upfile")
			if err != nil {
				fmt.Println(err)
				return
			}
			defer file.Close()
			path1 := path0 + time.Now().Format("02150405") + strconv.Itoa(rand.Intn(100)) + "_" + h.Filename
			//fmt.Println(path1)
			f, err := os.Create(path1)
			if err != nil {
				//beego.Error(err)
			}
			defer f.Close()
			io.Copy(f, file)
			//err = c.SaveToFile("upfile", path1) //.Join("attachment", attachment)) //存文件    WaterMark(path)    //给文件加水印
			if err != nil {
				//beego.Error(err)
			}

			if op == "uploadimage" {
				src, err := imaging.Open(path1)
				if err != nil {
					//log.Fatalf("failed to open image: %v", err)
				}
				// Resize the cropped image to width = 200px preserving the aspect ratio.
				dest := imaging.Resize(src, 160, 0, imaging.Lanczos)
				pathstrd := strings.Replace(path1, "wwwroot", "wwwroot/thumbnail/", 1)
				// Save the resulting image as JPEG.
				err = imaging.Save(dest, pathstrd)
				if err != nil {
					//log.Fatalf("failed to save image: %v", err)
				}

			}

			result = "{\"state\": \"SUCCESS\", \"url\": \"" + strings.Replace(path1, "./wwwroot", "", 1) + "\", \"title\": \"" + h.Filename + "\", \"original\": \"" + h.Filename + "\"}"
			//c.ServeJson()
			// 		{
			//     "state": "SUCCESS",
			//     "url": "upload/demo.jpg",
			//     "title": "demo.jpg",
			//     "original": "demo.jpg"
			// }
			/*
				case "uploadscrawl":
					number := c.Input().Get("number")

					name := c.Input().Get("name")
					err := os.MkdirAll(".\\attachment\\"+number+name, 0777) //..代表本当前exe文件目录的上级，.表示当前目录，没有.表示盘的根目录
					if err != nil {
						beego.Error(err)
					}
					path1 := ".\\attachment\\" + number + name + "\\"
					//保存上传的图片
					//upfile为base64格式文件，转成图片保存
					ww := c.Input().Get("upfile")
					ddd, _ := base64.StdEncoding.DecodeString(ww)           //成图片文件并把文件写入到buffer
					newname := strconv.FormatInt(time.Now().Unix(), 10)     // + "_" + filename
					err = ioutil.WriteFile(path1+newname+".jpg", ddd, 0666) //buffer输出到jpg文件中（不做处理，直接写到文件）
					if err != nil {
						beego.Error(err)
					}
					c.Data["json"] = map[string]interface{}{
						"state":    "SUCCESS",
						"url":      "/attachment/" + number + name + "/" + newname + ".jpg",
						"title":    newname + ".jpg",
						"original": newname + ".jpg",
					}
					c.ServeJson()
				case "listimage":
					list := []List{
						{"/static/upload/1.jpg"},
						{"/static/upload/2.jpg"},
					}
					listimage := Listimage{"SUCCESS", list, 1, 21}
					c.Data["json"] = listimage
					c.ServeJson()
					// 需要支持callback参数,返回jsonp格式
					// {
					//     "state": "SUCCESS",
					//     "list": [{
					//         "url": "upload/1.jpg"
					//     }, {
					//         "url": "upload/2.jpg"
					//     }, ],
					//     "start": 20,
					//     "total": 100
					// }
				case "catchimage":
					list := []ListCatch{
						{"/static/upload/1.jpg", "https://pic2.zhimg.com/7c4a389acaa008a6d1fe5a0083c86975_b.png", "SUCCESS"},
						{"/static/upload/2.jpg", "https://pic2.zhimg.com/7c4a389acaa008a6d1fe5a0083c86975_b.png", "SUCCESS"},
					}
					catchimage := Catchimage{"SUCCESS", list}
					c.Data["json"] = catchimage
					c.ServeJson()

					file, header, err := c.GetFile("source") // r.FormFile("upfile")
					beego.Info(header.Filename)
					if err != nil {
						panic(err)
					}
					defer file.Close()
					filename := strings.Replace(uuid.NewUUID().String(), "-", "", -1) + path.Ext(header.Filename)
					err = os.MkdirAll(path.Join("static", "upload"), 0775)
					if err != nil {
						panic(err)
					}
					outFile, err := os.Create(path.Join("static", "upload", filename))
					if err != nil {
						panic(err)
					}
					defer outFile.Close()
					io.Copy(outFile, file)
					// 需要支持callback参数,返回jsonp格式
					// list项的state属性和最外面的state格式一致
					// {
					//     "state": "SUCCESS",
					//     "list": [{
					//         "url": "upload/1.jpg",
					//         "source": "http://b.com/2.jpg",
					//         "state": "SUCCESS"
					//     }, {
					//         "url": "upload/2.jpg",
					//         "source": "http://b.com/2.jpg",
					//         "state": "SUCCESS"
					//     }, ]
					// }
			*/
		}

	}
	//log.Println(result)
	w.Write([]byte(result))
}
