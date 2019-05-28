package srs_auth

import (
	"encoding/json"
	"errors"
	"io"
	"os"
	"path/filepath"
	"time"

	"cfg"
	"database/sql"
	"log"
	"net/http"

	//"strconv"
	"strings"
	"ucenter"
)

func ServeSrs(w http.ResponseWriter, r *http.Request) {
	if origin := r.Header.Get("Origin"); origin != "" {
		w.Header().Set("Access-Control-Allow-Origin", "*") //允许访问所有域
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, HEAD, PUT, DELETE")
		w.Header().Add("Access-Control-Allow-Headers", "Cache-Control, X-Proxy-Authorization, X-Requested-With, Content-Type") //header的类型
		//		w.Header().Set("Content-Type", "application/json")                                //返回数据格式是json
	}

	if r.Method == "OPTIONS" {
		return
	}
	var data struct {
		Action   string `json:"action"`
		App      string `json:"app"`
		Hello    string `json:"hello"`
		Stream   string `json:"stream"`
		Param    string `json:"param"`
		ClientId int64  `json:"client_id"`
		Cwd      string `json:"cwd"`
		File     string `json:"file"`
	}
	err := json.NewDecoder(r.Body).Decode(&data)
	retstr := "1"
	if err == nil {
		switch data.Action {
		case "on_connect":
			retstr = "0"

		case "on_publish":
			pars, err := uri2map(data.Param)
			if err == nil {
				ui, _ := sign.SessionsGet(pars["sessionid"])
				if ui.Token == pars["token"] {
					retstr = "0"
				}
			}
		case "on_unpublish":
			retstr = "0"
		case "on_close":
			retstr = "0"
		case "on_play":
			pars, err := uri2map(data.Param)
			if err == nil {
				ui, _ := sign.SessionsGet(pars["sessionid"])
				if ui.Token == pars["token"] {
					retstr = "0"
				}
			}
			//log.Println("用户：", pars["sessionid"], retstr)
		case "on_stop":
			retstr = "0"
		case "on_dvr":
			pars, err := uri2map(data.Param)
			if err == nil {
				ui, _ := sign.SessionsGet(pars["sessionid"])
				if ui.Token == pars["token"] {
					retstr = "0"
					RoomId := ""
					if RoomId0, ok := pars["roomid"]; !ok {
						RoomId = "{32efce68-c637-4f1c-b915-2a7c5efd7b15}"
					} else {
						RoomId = RoomId0
					}
					db, err0 := sql.Open("adodb", cfg.Cfg["mssql"])
					if err0 != nil {
						log.Println("ServeSrs sql open error")
					} else {
						defer db.Close()
						OOtmp := strings.Replace(data.File, "/media/share/", "E:/mzgp/share/", 1)
						//log.Println(OOtmp)
						enckey := encoderfile(OOtmp)
						BasePath := cfg.SrsDvrBasepath
						FilePath := strings.Replace(data.File, "/media/share/", "", 1)
						FileName := filepath.Base(FilePath)
						NickName := FileName
						FullPath := BasePath + FilePath
						FileInfo, _ := os.Stat(FullPath)
						FileSize := FileInfo.Size()

						stmt1, err0 := db.Prepare(`
							INSERT INTO [hds12204021_db].[dbo].[Web2019_historylist]
							           ([RoomId]
							           ,[CreateDate]
							           ,[App]
							           ,[Stream]
							           ,[PublisherId]
							           ,[FilePath]
							           ,[FileName]
							           ,[NickName]
							           ,[FileSize]
										,[EncKey]
							           )
							     VALUES
							           (?
							           ,?
							           ,?
							           ,?
							           ,?
							           ,?
							           ,?
							           ,?
							           ,?
										,?
							           )						
						`)
						if err0 != nil {
							log.Println("ServeSrs sql db.Prepare error")
						} else {
							defer stmt1.Close()

							r1, err := stmt1.Exec(RoomId, time.Now().Format("2006-01-02 15:04:05"),
								data.App, data.Stream, ui.UserUuid, FilePath, FileName, NickName, FileSize, enckey)

							log.Println(r1)
							if err != nil {
								log.Println("ServeSrs sql stmt.Query error", err.Error())
							} else {

							}
						}
					}
				}
				log.Println("用户：", pars["sessionid"], "on_dvr", data.App, "/", data.Stream, data.File, " ", retstr)
			}
		}
	}
	w.Write([]byte(retstr))
}

func uri2map(uri string) (map[string]string, error) {
	m := make(map[string]string)
	if len(uri) < 1 {
		return m, errors.New("uri is none")
	}
	if uri[0:1] == "?" {
		uri = uri[1:]
	}

	pars := strings.Split(uri, "&")
	for _, par := range pars {
		parkv := strings.Split(par, "=")
		m[parkv[0]] = parkv[1]
	}
	//log.Println(m)
	return m, nil
}

func encoderfile(filepath string) byte {
	Copy(filepath, filepath+".bak")
	fi, _ := os.Stat(filepath)
	a1 := []byte(fi.Name())
	var s byte = (a1[5])
	if s == 0 {
		s = 1
	}
	file, err := os.OpenFile(filepath, os.O_RDWR, 0600)
	log.Println(filepath, err)
	if err != nil {
		log.Println(err)
	}
	defer file.Close()
	data := make([]byte, 1024)
	count, err := file.Read(data)
	log.Println(count, err)
	if err == nil {
		if count == len(data) {
			for i := 0; i < 1024; i++ {
				data[i] = data[i] ^ s
			}
			file.Seek(0, os.SEEK_SET)
			file.Write(data[512:1024])
			log.Println(data[512:1024])
			file.Write(data[0:512])
			log.Println(data[0:512])
		}
	}
	return s
}

func Copy(src, dst string) (int64, error) {
	sourceFileStat, err := os.Stat(src)
	if err != nil {
		return 0, err
	}

	if !sourceFileStat.Mode().IsRegular() {
		return 0, err
	}

	source, err := os.Open(src)
	if err != nil {
		return 0, err
	}
	defer source.Close()

	destination, err := os.Create(dst)
	if err != nil {
		return 0, err
	}

	defer destination.Close()
	nBytes, err := io.Copy(destination, source)
	return nBytes, err
}
