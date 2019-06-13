package toolset

import (
	"database/sql"
)

func MapGetString(key string, v *map[string]interface{}, defv string) string {
	ret := defv
	if val, found := (*v)[key]; found {
		ret = val.(string)
	}
	return ret
}

func MapGetInt(key string, v *map[string]interface{}, defv int) int {
	ret := defv
	if val, found := (*v)[key]; found {
		switch val.(type) {
		case int64:
			ret = int(val.(int64))
		case float64:
			ret = int(val.(float64))

		}
	}
	return ret
}

// Rows2Map ok
func Rows2Map(rows *sql.Rows) ([](map[string]interface{}), error) {
	ris := [](map[string]interface{}){}
	culs, _ := rows.Columns()
	count := len(culs)
	vals := make([]interface{}, count)
	scanArgs := make([]interface{}, count)
	for i := range vals {
		scanArgs[i] = &vals[i]
	}

	for rows.Next() {
		rows.Scan(scanArgs...)
		entry := make(map[string]interface{})
		for i, col := range culs {
			v := vals[i]
			b, ok := v.([]byte)
			if ok {
				entry[col] = string(b)
			} else {
				entry[col] = v
			}
		}
		ris = append(ris, entry)
	}
	return ris, nil
}

// RowMap2RowMap ok
func RowMap2RowMap(map0 *[](map[string]interface{}), map1 *[](map[string]interface{})) ([](map[string]interface{}), error) {
	toDel := []int{}
	for _, v0 := range *map0 {
		for k1, v1 := range *map1 {
			if v0["userid"] == v1["userid"] {
				toDel = append(toDel, k1)
			}
		}
	}
	for k1, v1 := range *map1 {
		have := false
		for _, v2 := range toDel {
			if v2 == k1 {
				have = true
			}
		}
		if !have {
			*map0 = append((*map0), v1)
		}
	}

	return *map0, nil
}
