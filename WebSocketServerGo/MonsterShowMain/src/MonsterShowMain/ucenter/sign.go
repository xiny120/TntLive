package sign

import (
	"MonsterShowMain/cfg"
	//"database/sql"
	"os"

	"encoding/json"
	//_ "fmt"
	"log"

	//"strings"

	uuid "github.com/satori/go.uuid"

	//_ "github.com/denisenkom/go-mssqldb"
	_ "github.com/mattn/go-adodb"
)

const (
	// StaSignupOk   means shit
	StaSignupOk = iota
	// StaSignupNameDuplicate means shit
	StaSignupNameDuplicate
	// StaSignupSQLServerError means shit toooooooooooo
	StaSignupSQLServerError
)

const (
	// StaSigninOk ok
	StaSigninOk = iota
	// StaSigninNameError ok
	StaSigninNameError
	// StaSigninPwdError ok
	StaSigninPwdError
	// StaSigninSQLServerError ok
	StaSigninSQLServerError
)

var (
	sessions = make(map[string]*UserInfo)
)

// SessionsDel ok
func SessionsDel(token string) {
	delete(sessions, token)
}

// SessionsSet ok
func SessionsSet(token string, ui *UserInfo) {
	sessions[token] = ui
}

// SessionsGet ok
func SessionsGet(token string) (*UserInfo, bool) {
	ui := &UserInfo{}
	ret := false
	func() {
		if ui, ret = sessions[token]; ret {
			return
		}
		tfile := "./tokens/" + token
		f, err3 := os.Open(tfile) //创建文件
		if err3 != nil {
			return
		}
		defer f.Close()
		fileinfo, err := f.Stat()
		if err != nil {
			return
		}
		fileSize := fileinfo.Size()
		buffer := make([]byte, fileSize)
		_, err = f.Read(buffer)
		if err != nil {
			return
		}
		err3 = json.Unmarshal(buffer, &ui)
		if err3 == nil {
			sessions[token] = ui
			ret = true
		}
	}()

	return ui, ret
}

// In ok
func In(un string, pwd string) (*UserInfo, bool) {
	ret := false
	ok, _ := uuid.NewV4()
	sid, _ := uuid.NewV4()
	ui := &UserInfo{UserName: "anonymous", UserID: 0, Token: ok.String(), UserGUID: "00000000-0000-0000-0000-000000000000", Info: "", AvatarURL: "../../static/logo.png", SessionID: sid.String()}
	ui.Info = func() string {
		db, err0 := cfg.OpenDb() //sql.Open("adodb", cfg.Cfg["mssql"])
		if err0 != nil {
			return "SignIn sql open error"
		}
		defer db.Close()
		stmt, err0 := db.Prepare(`
			SELECT b.userid,a.userid as username,a.userguid
			FROM useridentify a, userinfo b
			where a.userguid = b.userguid and a.userid=? and b.password=?	
		`)
		if err0 != nil {
			return "SignIn sql db.Prepare error"
		}
		defer stmt.Close()
		rows, err := stmt.Query(un, pwd)
		if err != nil {
			return "SignIn sql stmt.Query error"
		}
		ui.Info = "用户名或密码错误！"
		for rows.Next() {
			rows.Scan(&ui.UserID, &ui.UserName, &ui.UserGUID)
			ui.Info = "登录成功"
			ret = true
			break
		}
		return ui.Info
	}()
	return ui, ret
}

// Up ok
func Up(un string, pwd string, cellphone string, email string) {

}

// ModiPassword ok
func ModiPassword(un string, pwd string, pwdnew string) (string, int) {
	iresult := 1
	ui := &UserInfo{}
	ok := false
	result := func() string {
		if ui, ok = In(un, pwd); !ok {
			return "当前密码错误！"
		}
		db, err0 := cfg.OpenDb()
		if err0 != nil {
			return "modipassword sql open error"
		}
		defer db.Close()
		stmt1, err0 := db.Prepare(`Update userinfo set password=? where userguid = ?`)
		if err0 != nil {
			log.Println(err0)
			return "modipassword sql db.Prepare error"
		}
		defer stmt1.Close()
		r1, err := stmt1.Exec(pwdnew, ui.UserGUID)
		log.Println(r1)
		if err != nil {
			return "modipassword sql stmt.Query error"
		}
		iresult = 0
		return "用户名密码修改成功"
	}()
	return result, iresult
}
