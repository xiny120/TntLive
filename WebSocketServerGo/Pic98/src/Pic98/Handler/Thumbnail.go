package Handler

import (
	"html/template"
	"image/jpeg"
	_ "log"
	"net/http"
	"net/url"
	"os"
	"path"
	"path/filepath"
	"strings"

	"github.com/nfnt/resize"
)

func Thumbnail(w http.ResponseWriter, r *http.Request) {
	requri := r.RequestURI
	split0 := strings.Split(requri, "?")
	param := strings.Split(split0[0], "/")
	if len(param) >= 3 {
		filePath := "wwwroot/thumbnail/Image"
		for _, val := range param[3:] {
			if val == ".." {
				continue
			}
			filePath = filePath + "/" + val

		}

		filePath, _ = url.QueryUnescape(filePath)
		//log.Println(filePath)
		if pe, _ := FileExists(filePath); pe == true {
			http.ServeFile(w, r, filePath)
			return
		}
		http.ServeFile(w, r, "wwwroot/Image/siwa.jpg")
	}
}

func MakeThumbnail(w http.ResponseWriter, r *http.Request) {
	//fmt.Fprintf(w, "%s", "register now!")
	t, err := template.ParseFiles(
		"wwwroot/tpl/Message.html",
		"wwwroot/tpl/public/header.html",
		"wwwroot/tpl/public/nav.html",
		"wwwroot/tpl/public/footer.html")
	if err != nil {
		//log.Fatal(err)
	}

	path := "wwwroot/Image/Vip/"
	pathstr := ""

	err = filepath.Walk(path, func(path string, f os.FileInfo, err error) error {
		if f == nil {
			return err
		}
		if f.IsDir() {
			return nil
		}
		println(path)

		file, err := os.Open(path)
		if err != nil {
			//log.Fatal(err)
		}

		img, err := jpeg.Decode(file)
		if err != nil {
			//log.Fatal(err)
		}
		file.Close()

		// resize to width 1000 using Lanczos resampling
		// and preserve aspect ratio
		m := resize.Resize(160, 0, img, resize.Lanczos3)
		pathstrd := strings.Replace(path, "wwwroot", "wwwroot/thumbnail/", 1)
		createFile(pathstrd)

		out, err := os.Create(pathstrd)
		if err != nil {
			//log.Fatal(err)
		}
		defer out.Close()

		// write new image to file
		jpeg.Encode(out, m, nil)

		pathstr += " " + path
		return nil
	})
	if err != nil {
		//log.Fatal(err)
	}

	data := struct {
		Title   string
		Message string
	}{
		Title:   "注册新用户",
		Message: pathstr,
	}

	err = t.Execute(w, data)
	if err != nil {
		//log.Fatal(err)
	}
}

//调用os.MkdirAll递归创建文件夹
func createFile(filePath string) error {
	filePath = path.Dir(filepath.ToSlash(filePath))
	if !isExist(filePath) {
		err := os.MkdirAll(filePath, os.ModePerm)
		return err
	}
	return nil
}

// 判断所给路径文件/文件夹是否存在(返回true是存在)
func isExist(path string) bool {
	_, err := os.Stat(path) //os.Stat获取文件信息
	if err != nil {
		if os.IsExist(err) {
			return true
		}
		return false
	}
	return true
}
