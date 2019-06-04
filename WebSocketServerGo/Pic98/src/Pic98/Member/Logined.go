package Member

import (
	"Pic98/Cfg"
	"database/sql"
	"encoding/json"
	"log"
	"net/http"

	_ "github.com/go-sql-driver/mysql"
	"github.com/satori/go.uuid"
)

type Userinfo struct {
	Online_key        string
	Userguid          string
	Un                string
	Idol              int
	Writer            int
	Distributor_level int
	Distributor_ref0  int
	Distributor_ref1  int
}

var (

	//sessions := map[string]userinfo{}
	Sessions = make(map[string]Userinfo)
)

// NewV1 returns UUID based on current timestamp and MAC address.
func Login(un string, pwd string) (Userinfo, error) {
	ui := Userinfo{Online_key: "", Userguid: "", Un: "", Idol: 0, Writer: 0}
	db, err := sql.Open("mysql", Cfg.Cfg["tidb"])
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()
	//strsql := fmt.Sprintf("SELECT userguid FROM useridentify where userid='%s'", name)
	stmt, err0 := db.Prepare(`SELECT a.userguid,b.idol,b.writer FROM Pic98.useridentify a, Pic98.userinfo b 
	where a.userid=? and a.userguid = b.userguid and b.password = ?`)
	if err0 != nil {
		log.Println(err0)
	}
	//log.Println(stmt)
	defer stmt.Close()
	rows, err := stmt.Query(un, pwd)
	//log.Println(rows)
	//log.Println(err)
	if err == nil {
		defer rows.Close()
		if rows.Next() {
			rows.Scan(&ui.Userguid, &ui.Idol, &ui.Writer)
			ok, _ := uuid.NewV4()
			ui.Online_key = ok.String()
			ui.Un = un
			Sessions[ui.Online_key] = ui
			SaveUserinfo(ui)
		}
	}
	return ui, nil
}

func SaveUserinfo(ui Userinfo) {
	db, err := sql.Open("mysql", Cfg.Cfg["tidb"])
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()
	stmt, _ := db.Prepare(`delete from sessions where un=?`)
	defer stmt.Close()
	rows, err := stmt.Exec(ui.Un)
	if err != nil {
		log.Println(rows)
	}
	ui0, _ := json.Marshal(ui)
	stmt0, _ := db.Prepare(`insert into sessions (aguid,online_key,un,uidata) values(?,?,?,?)`)
	defer stmt0.Close()
	ok, _ := uuid.NewV4()
	ret, err0 := stmt0.Exec(ok.String(), ui.Online_key, ui.Un, string(ui0))
	log.Println(ret)
	if err0 != nil {
		log.Println(err0)
	}

}

func LoadUserinfo(r *http.Request) (Userinfo, error) {
	ui := Userinfo{Online_key: "", Userguid: "", Un: "", Idol: 0, Writer: 0}

	cookie, err0 := r.Cookie("token")
	if err0 == nil {
		online_key := cookie.Value
		db, err := sql.Open("mysql", Cfg.Cfg["tidb"])
		if err != nil {
			log.Println(err)
		} else {
			defer db.Close()
			stmt, _ := db.Prepare(`select uidata from sessions where online_key =?`)
			defer stmt.Close()
			rows, err := stmt.Query(online_key)
			if err != nil {
				log.Println(rows)
			} else {
				defer rows.Close()
				if rows.Next() {
					var uidata string
					rows.Scan(&uidata)
					err := json.Unmarshal([]byte(uidata), &ui)

					//解析失败会报错，如json字符串格式不对，缺"号，缺}等。
					if err != nil {
						log.Println(err)
					}
				}

			}
		}
	}
	return ui, nil

}
