package dbo

import (
	"Pic98/cfg"
	"database/sql"
)

var (
	driverName     = "mysql"
	dataSourceName = cfg.Cfg["tidb"]
)

// Init ok
func Init() {
	driverName = "mysql"
	dataSourceName = cfg.Cfg["tidb"]
}

// Select ok
func Select(prepareStr string, args ...interface{}) (*sql.Rows, error) {
	db, err := sql.Open(driverName, dataSourceName)
	if err != nil {
		return nil, err
	}
	defer db.Close()
	stmt, err := db.Prepare(prepareStr)
	if err != nil {
		return nil, err
	}
	defer stmt.Close()
	rows, err := stmt.Query(args...)
	if err != nil {
		return nil, err
	}
	return rows, nil
}

// Insert ok
func Insert(prepareStr string, args ...interface{}) (sql.Result, error) {
	db, err := sql.Open(driverName, dataSourceName)
	if err != nil {
		return nil, err
	}
	defer db.Close()
	stmt, err := db.Prepare(prepareStr)
	if err != nil {
		return nil, err
	}
	defer stmt.Close()
	rows, err := stmt.Exec(args...)
	if err != nil {
		return nil, err
	}
	return rows, nil
}

// InsertS ok
func InsertS(prepareStr string, args ...interface{}) sql.Result {
	if db, err := sql.Open(driverName, dataSourceName); err == nil {
		defer db.Close()
		if stmt, err := db.Prepare(prepareStr); err == nil {
			defer stmt.Close()
			if rows, err := stmt.Exec(args); err == nil {
				return rows
			}
		}
	}
	return nil
}
