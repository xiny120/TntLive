package sign

import(
	"cfg"
	"database/sql"
	"encoding/json"
	_ "github.com/go-sql-driver/mysql"	
)

const{
	StaSignupOk  = iota
	StaSignupNameDuplicate
	StaSignupSqlServerError
}

const{
	StaSigninOk = iota
	StaSigninNameError
	StaSigninPwdError
	StaSigninSqlServerError
}

func SignIn(un string, pwd string) int {
	db, err := sql.Open("mysql", cfg.Cfg["tidb"])
	if err != nil {
		
	}
	defer db.Close()
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

func SignUp(un string, pwd string, cellphone string, email string){
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