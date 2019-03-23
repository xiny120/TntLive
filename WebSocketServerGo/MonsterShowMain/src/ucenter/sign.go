package sign

import (
	"cfg"
	"database/sql"

	//"encoding/json"
	"fmt"
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
	Sessions = make(map[string]*UserInfo)
)

func SignIn(un string, pwd string) *UserInfo {
	ok, _ := uuid.NewV4()
	//log.Println(ok)
	// 连接数据库
	ui := &UserInfo{UserName: "anonymous", UserId: 0, Token: ok.String(), UserUuid: "00000000-0000-0000-0000-000000000000", Info: "", AvatarUrl: "../../static/logo.png"}
	db, err0 := sql.Open("adodb", cfg.Cfg["mssql"])
	if err0 != nil {
		fmt.Println("sql open:", err0)
		ui.Info = "SignIn sql open error"

	} else {
		defer db.Close()

		// 执行SQL语句
		//rows, err := db.Query("select username,userid from dv_user where username='" + un + "'")
		stmt, err0 := db.Prepare(`SELECT [UserID],[UserName],[userguid] FROM [Dv_User] where [UserName] = ? and [UserPassword] = ?`)
		if err0 != nil {
			log.Println(err0)
			ui.Info = "SignIn sql db.Prepare error"
		} else {
			defer stmt.Close()
			rows, err := stmt.Query(un, pwd)
			if err != nil {
				fmt.Println("query: ", err)
				ui.Info = "SignIn sql stmt.Query error"
			} else {
				for rows.Next() {
					var name string
					var number int
					rows.Scan(&ui.UserId, &ui.UserName, &ui.UserUuid)
					fmt.Printf("Name: %s \t Number: %d\n", name, number)
				}
			}
		}

	}
	return ui
}

func SignUp(un string, pwd string, cellphone string, email string) {
	/*
		db, err := sql.Open("mysql", Cfg.Cfg["tidb"])
		if err != nil {
			log.Println(err)
		}
		defer db.Close()
		tagguids := strings.Split(tagguid, ",")

		for _, val := range tagguids {
			if val == "" {
				continue
			}
			aguid, _ := uuid.NewV4()
			stmt, _ := db.Prepare("INSERT INTO tags2pic(aguid,tagguid,picguid) VALUES (?,?,?)")
			//log.Println(stmt)
			defer stmt.Close()
			_, erri := stmt.Exec(aguid, val, picguid)
			if erri != nil {
				fmt.Printf("insert data error: %v\n", err)
			}
		}
	*/

}
