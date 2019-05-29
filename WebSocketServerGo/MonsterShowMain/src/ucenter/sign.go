package sign

import (
	"cfg"
	//"database/sql"
	"os"

	"encoding/json"
	_ "fmt"
	"log"

	//"strings"

	"github.com/satori/go.uuid"

	//_ "github.com/denisenkom/go-mssqldb"
	_ "github.com/mattn/go-adodb"
)

const (
	StaSignupOk = iota
	StaSignupNameDuplicate
	StaSignupSqlServerError
)

const (
	StaSigninOk = iota
	StaSigninNameError
	StaSigninPwdError
	StaSigninSqlServerError
)

var (
	sessions = make(map[string]*UserInfo)
)

func SessionsDel(token string) {
	delete(sessions, token)
}
func SessionsSet(token string, ui *UserInfo) {
	sessions[token] = ui
}

func SessionsGet(token string) (*UserInfo, bool) {
	ui := &UserInfo{}
	ret := false
	found := false

	func() {
		if ui, found = sessions[token]; found {
			ret = true
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

func SignIn(un string, pwd string) *UserInfo {
	ok, _ := uuid.NewV4()
	sid, _ := uuid.NewV4()
	ui := &UserInfo{UserName: "anonymous", UserId: 0, Token: ok.String(), UserUuid: "00000000-0000-0000-0000-000000000000", Info: "", AvatarUrl: "../../static/logo.png", SessionId: sid.String()}
	ui.Info = func() string {
		db, err0 := cfg.OpenDb() //sql.Open("adodb", cfg.Cfg["mssql"])
		if err0 != nil {
			return "SignIn sql open error"
		}
		defer db.Close()
		stmt, err0 := db.Prepare(`SELECT [UserID],[UserName],[userguid] FROM [Dv_User] where [UserName] = ? and [UserPassword] = ?`)
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
			rows.Scan(&ui.UserId, &ui.UserName, &ui.UserUuid)
			ui.Info = "登录成功"
			break
		}
		return ui.Info
	}()
	return ui
}

func SignUp(un string, pwd string, cellphone string, email string) {

}

func ModiPassword(un string, pwd string, pwdnew string) (string, int) {
	iresult := 1
	result := func() string {
		db, err0 := cfg.OpenDb() //sql.Open("adodb", cfg.Cfg["mssql"])
		if err0 != nil {
			return "modipassword sql open error"
		}
		defer db.Close()
		stmt, err0 := db.Prepare(`SELECT [UserID],[UserName],[userguid] FROM [Dv_User] where [UserName] = ? and [UserPassword] = ?`)
		if err0 != nil {
			return "modipassword sql db.Prepare error"
		}
		defer stmt.Close()
		rows, err := stmt.Query(un, pwd)
		if err != nil {
			return "modipassword sql stmt.Query error"
		}

		if !rows.Next() {
			return "当前密码错误！"
		}
		stmt1, err0 := db.Prepare(`Update [Dv_User] set UserPassword=? where [UserName] = ?`)
		if err0 != nil {
			return "modipassword sql db.Prepare error"
		}
		defer stmt1.Close()
		r1, err := stmt1.Exec(pwdnew, un)
		log.Println(r1)
		if err != nil {
			return "modipassword sql stmt.Query error"
		}
		iresult = 0
		return "用户名密码修改成功"
	}()
	return result, iresult
}
