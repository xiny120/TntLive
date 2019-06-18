package srsauth

import (
	"encoding/json"
	"errors"
	"io"
	"os"
	"path/filepath"
	"time"

	"MonsterShowMain/cfg"
	//"database/sql"
	"log"
	"net/http"

	sign "MonsterShowMain/ucenter"
	"strings"
)

// ServeSrs ok
func ServeSrs(w http.ResponseWriter, r *http.Request) {
	if origin := r.Header.Get("Origin"); origin != "" {
		w.Header().Set("Access-Control-Allow-Origin", "*") //允许访问所有域
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, HEAD, PUT, DELETE")
		w.Header().Add("Access-Control-Allow-Headers", "Cache-Control, X-Proxy-Authorization, X-Requested-With, Content-Type") //header的类型
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
		ClientID int64  `json:"client_id"`
		Cwd      string `json:"cwd"`
		File     string `json:"file"`
	}
	err := json.NewDecoder(r.Body).Decode(&data)
	//log.Println(data)
	retstr := "1"
	if err != nil {
		w.Write([]byte(retstr))
	}
	switch data.Action {
	case "on_connect":
		retstr = "0"
	case "on_publish":
		log.Println(data)
		if pars, err := uri2map(data.Param); err == nil {
			ui, _ := sign.SessionsGet(pars["s"])
			if ui.Token == pars["t"] {
				retstr = "0"
			}
		}
	case "on_unpublish":
		retstr = "0"
	case "on_close":
		retstr = "0"
	case "on_play":
		if pars, err := uri2map(data.Param); err == nil {
			if sid, found := pars["s"]; found {
				if ui, err0 := sign.SessionsGet(sid); err0 && sid != "" {
					if tk, err1 := pars["t"]; err1 && ui.Token == tk {
						retstr = "0"
					}
				}
			}
		}
	case "on_stop":
		retstr = "0"
	case "on_dvr":
		pars, err := uri2map(data.Param)
		if err != nil {
			break
		}
		ui, found := sign.SessionsGet(pars["s"])
		if !found {
			break
		}
		if ui.Token != pars["t"] {
			break
		}
		retstr = "0"
		RoomID := 9999
		//if RoomID0, ok := pars["r"]; !ok {
		//	RoomID = 9997
		//} else {
		//	RoomID, _ = strconv.Atoi(RoomID0)
		//}
		db, err0 := cfg.OpenDb()
		if err0 != nil {
			log.Println("ServeSrs sql open error")
			break
		}
		defer db.Close()
		// get roomid by app and stream
		stmtrd, errrd := db.Prepare(`
			select roomid from roomlist where vapp=? and vstream=?
		`)
		if errrd != nil {
			log.Println("errrd", errrd)
			break
		}
		defer stmtrd.Close()
		rowrd, errrowrd := stmtrd.Query(data.App, data.Stream)
		if errrowrd != nil {
			log.Println("errrowrd", errrowrd)
			break
		}
		if !rowrd.Next() {
			log.Println("rowrd.Next()")
			break
		}
		var val interface{}
		rowrd.Scan(&val)
		RoomID = int(val.(int64))

		OOtmp := strings.Replace(data.File, "/media/share/", "E:/mzgp/share/", 1)
		enckey := encoderfile(OOtmp)
		FilePath := strings.Replace(data.File, "/media/share/", "", 1)
		FileName := filepath.Base(FilePath)
		NickName := FileName

		stmt1, err0 := db.Prepare(`
			INSERT INTO hislist
			(roomid,createdate,PublisherId,FilePath,FileName,NickName,encrypted,EncKey)
			VALUES(?,?,?,?,?,?,?,?)
		`)
		if err0 != nil {
			log.Println("ServeSrs sql db.Prepare error", err0)
		} else {
			defer stmt1.Close()

			_, err := stmt1.Exec(RoomID, time.Now().Format("2006-01-02 15:04:05"),
				ui.UserID, FilePath, FileName, NickName, 1, enckey)

			//log.Println(r1)
			if err != nil {
				log.Println("ServeSrs sql stmt.Query error", err.Error())
			} else {

			}
		}
		log.Println("用户：", pars["s"], "roomid:", RoomID, "/", data.Stream, data.File, " ", retstr)
	}
	//log.Println("SRS ACTION: %s Status:%s", data.Action, retstr)
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
	var s byte
	s = (a1[5])
	if s == 0 {
		s = 1
	}
	file, err := os.OpenFile(filepath, os.O_RDWR, 0600)
	if err != nil {
		log.Println(err)
	}
	defer file.Close()
	data := make([]byte, 1024)
	count, err := file.Read(data)
	if err == nil {
		if count == len(data) {
			for i := 0; i < 1024; i++ {
				data[i] = data[i] ^ s
			}
			file.Seek(0, os.SEEK_SET)
			file.Write(data[512:1024])
			file.Write(data[0:512])
		}
	}
	return s
}

// Copy ok
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
