package Handler

import (
	"Pic98/Cfg"
	"Pic98/Member"
	"database/sql"
	"fmt"
	_ "image"
	_ "image/color"
	_ "image/draw"
	"image/jpeg"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strings"

	_ "github.com/go-sql-driver/mysql"

	"github.com/satori/go.uuid"
)

func Image(w http.ResponseWriter, r *http.Request) {
	param := strings.Split(r.RequestURI, "/")
	if len(param) >= 3 {
		filePath := "wwwroot/Image/" + param[2]
		filePath, _ = url.QueryUnescape(filePath)
		if pe, _ := PathExists(filePath); pe == true {
			http.ServeFile(w, r, filePath)
			return
		}
		http.ServeFile(w, r, "wwwroot/Image/siwa.jpg")
	}
}

func Image_Banner(w http.ResponseWriter, r *http.Request) {
	param := strings.Split(r.RequestURI, "/")
	if len(param) >= 3 {
		filePath := "wwwroot/Image/Banner"
		for _, val := range param[3:] {
			if val == ".." {
				continue
			}
			filePath = filePath + "/" + val

		}
		//log.Println(filePath)
		if pe, _ := PathExists(filePath); pe == true {
			http.ServeFile(w, r, filePath)
			return
		}
		http.ServeFile(w, r, "wwwroot/Image/siwa.jpg")
	}
}

func Image_Free(w http.ResponseWriter, r *http.Request) {
	requri := r.RequestURI
	split0 := strings.Split(requri, "?")
	param := strings.Split(split0[0], "/")
	if len(param) >= 3 {
		filePath := "wwwroot/Image/free"
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

func Image_Vip(w http.ResponseWriter, r *http.Request) {
	requri := r.RequestURI
	split0 := strings.Split(requri, "?")
	param := strings.Split(split0[0], "/")
	if len(param) >= 3 {

		ui, err0 := Member.LoadUserinfo(r)
		filePath := "wwwroot/thumbnail/Image/Vip"
		if err0 == nil {
			if ui.Userguid != "" {
				filePath = "wwwroot/Image/Vip"
			}
		} else {

			//
		}
		url0 := "/thumbnail/Image/Vip/"
		for _, val := range param[3:] {
			if val == ".." {
				continue
			}
			filePath = filePath + "/" + val
			url0 = url0 + val

		}

		if err0 != nil {
			w.Header().Set("Location", url0)
			http.Redirect(w, r, url0, http.StatusTemporaryRedirect)
			return
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

func Image_Update(w http.ResponseWriter, r *http.Request) {
	filePath := getCurrentDirectory() + "/wwwroot/Image/Vip/"
	filePath = "wwwroot/Image/Vip"
	files, _ := GetAllFiles(filePath)
	//log.Println(files)
	for _, file := range files {

		aguid, _ := uuid.NewV4()

		f1, errjpg := os.Open(file)
		if errjpg != nil {
			panic(errjpg)
		}
		defer f1.Close()

		m1, errm := jpeg.Decode(f1)
		if errm != nil {
			panic(errm)
		}
		bounds := m1.Bounds()

		db, err := sql.Open("mysql", Cfg.Cfg["tidb"])
		if err != nil {
			//log.Fatal(err)
		}
		defer db.Close()
		file = strings.Replace(file, "wwwroot/", "", -1)
		log.Println(file)
		stmt, _ := db.Prepare("INSERT INTO picinfo(aguid,picurl,width,height) VALUES (?,?,?,?)")
		log.Println(stmt)
		defer stmt.Close()
		_, erri := stmt.Exec(aguid, "/"+file, bounds.Dx(), bounds.Dy())
		if erri != nil {
			fmt.Printf("insert data error: %v\n", err)
		}

	}

}

//获取指定目录下的所有文件,包含子目录下的文件
func GetAllFiles(dirPth string) (files []string, err error) {
	var dirs []string
	dir, err := ioutil.ReadDir(dirPth)
	if err != nil {
		return nil, err
	}

	PthSep := string(os.PathSeparator)
	//suffix = strings.ToUpper(suffix) //忽略后缀匹配的大小写

	for _, fi := range dir {
		if fi.IsDir() { // 目录, 递归遍历
			dirs = append(dirs, dirPth+PthSep+fi.Name())
			GetAllFiles(dirPth + PthSep + fi.Name())
		} else {
			// 过滤指定格式
			//ok := strings.HasSuffix(fi.Name(), ".go")
			//if ok {
			files = append(files, dirPth+PthSep+fi.Name())
			//}
		}
	}

	// 读取子目录下文件
	for _, table := range dirs {
		temp, _ := GetAllFiles(table)
		for _, temp1 := range temp {
			files = append(files, temp1)
		}
	}

	return files, nil
}

func getCurrentDirectory() string {
	dir, err := filepath.Abs(filepath.Dir(os.Args[0]))
	if err != nil {
		//log.Fatal(err)
	}
	return strings.Replace(dir, "\\", "/", -1)
}

// 获取表数据
func Get1(db *sql.DB) {
	rows, err := db.Query("select * from user;")
	if err != nil {
		//log.Fatal(err)
	}
	defer rows.Close()
	cloumns, err := rows.Columns()
	if err != nil {
		//log.Fatal(err)
	}
	// for rows.Next() {
	//  err := rows.Scan(&cloumns[0], &cloumns[1], &cloumns[2])
	//  if err != nil {
	//      log.Fatal(err)
	//  }
	//  fmt.Println(cloumns[0], cloumns[1], cloumns[2])
	// }
	values := make([]sql.RawBytes, len(cloumns))
	scanArgs := make([]interface{}, len(values))
	for i := range values {
		scanArgs[i] = &values[i]
	}
	for rows.Next() {
		err = rows.Scan(scanArgs...)
		if err != nil {
			//log.Fatal(err)
		}
		var value string
		for i, col := range values {
			if col == nil {
				value = "NULL"
			} else {
				value = string(col)
			}
			fmt.Println(cloumns[i], ": ", value)
		}
		fmt.Println("------------------")
	}
	if err = rows.Err(); err != nil {
		//log.Fatal(err)
	}
}
