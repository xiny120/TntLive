package sign

import (
	"cfg"
	"database/sql"

	//"encoding/json"
	"fmt"
	"log"

	//"strings"

	//"github.com/satori/go.uuid"

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

func SignIn(un string, pwd string) int {

	// 连接数据库
	db, err0 := sql.Open("adodb", cfg.Cfg["mssql"])
	if err0 != nil {
		fmt.Println("sql open:", err0)
		return 0
	}
	defer db.Close()

	// 执行SQL语句
	//rows, err := db.Query("select username,userid from dv_user where username='" + un + "'")
	stmt, err0 := db.Prepare(`SELECT [UserID],[UserName] FROM [Dv_User] where [UserName] = ? and [UserPassword] = ?`)
	if err0 != nil {
		log.Println(err0)
	}
	defer stmt.Close()
	rows, err := stmt.Query(un, pwd)
	if err != nil {
		fmt.Println("query: ", err)
		return 0
	}
	for rows.Next() {
		var name string
		var number int
		rows.Scan(&number, &name)
		fmt.Printf("Name: %s \t Number: %d\n", name, number)
	}

	/*

		db, err := sql.Open("mssql", cfg.Cfg["mssql"])
		var userid int32
		var username string
		var userpassword string
		var userguid string
		if err != nil {
			log.Fatal("Open connection failed:", err.Error())
		}
		err = db.Ping()
		if err != nil {
			log.Fatal("connection failed:", err.Error())
		}
		log.Println(err.Error())
		defer db.Close()
		stmt, err0 := db.Prepare(`SELECT [UserID],[UserName],[UserPassword],[userguid] FROM [Dv_User] where [UserName] = "xinyuan"`)
		if err0 != nil {
			log.Println(err0)
		}
		defer stmt.Close()
		rows, err := stmt.Query(un)
		if err == nil {
			defer rows.Close()
			if rows.Next() {
				rows.Scan(&userid, &username, &userpassword, &userguid)
				ok, _ := uuid.NewV4()
				log.Println(ok)
			}
		}

		ss, _ := json.Marshal(db)
		log.Println(ss)
	*/

	return 0
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
