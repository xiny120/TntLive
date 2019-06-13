package dbo

import "database/sql"

// ArticleReviewSearchPass ok
func ArticleReviewSearchPass(aguid string, status int) error {
	db, err := sql.Open(driverName, dataSourceName)
	if err != nil {
		return err
	}
	defer db.Close()
	tx, err := db.Begin()
	if err != nil {
		return err
	}
	table0 := "topic"
	if status == 1 {
		table0 = "topic_reject"
	}
	stmt, err := tx.Prepare(`
	INSERT INTO ` + table0 + `(aguid,
	coverimg,
	createtime,
	likesum,
	likeweek,
	likemonth,
	userguid,
	idolguid,
	title,
	intro,
	content,
	tags) 
	SELECT aguid,
	coverimg,
	createtime,
	likesum,
	likeweek,
	likemonth,
	userguid,
	idolguid,
	title,
	intro,
	content,
	tags FROM topic_review 
	where aguid = ?
	`)
	if err != nil {
		tx.Rollback()
		return err
	}
	defer stmt.Close()
	_, err = stmt.Exec(aguid)
	if err != nil {
		tx.Rollback()
		return err
	}

	stmt, err = tx.Prepare(`
	delete
	FROM topic_review 
	where aguid = ?
		`)
	if err != nil {
		tx.Rollback()
		return err
	}
	defer stmt.Close()
	_, err = stmt.Exec(aguid)
	if err != nil {
		tx.Rollback()
		return err
	}
	tx.Commit()
	return nil
}

// OriginalReviewAllow ok
func OriginalReviewAllow(id string, status int) error {
	db, err := sql.Open(driverName, dataSourceName)
	if err != nil {
		return err
	}
	defer db.Close()
	tx, err := db.Begin()
	if err != nil {
		return err
	}
	stmt, err := tx.Prepare(`
	UPDATE original
	SET	status = ?
	WHERE id= ?;
	`)
	if err != nil {
		tx.Rollback()
		return err
	}
	defer stmt.Close()
	_, err = stmt.Exec(status, id)
	if err != nil {
		tx.Rollback()
		return err
	}
	writer := 0
	if status == 1 {
		writer = 1
	}
	stmt, err = tx.Prepare(`
		Update userinfo
		Set writer=?
		where userid in (select userid from original where id=?)
		`)
	if err != nil {
		tx.Rollback()
		return err
	}
	defer stmt.Close()
	_, err = stmt.Exec(writer, id)
	if err != nil {
		tx.Rollback()
		return err
	}
	tx.Commit()
	return nil
}
