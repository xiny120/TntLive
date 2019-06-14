package main

import (
	"encoding/json"
	"fmt"
)

func main() {
	var v map[string]interface{}
	jsonstr := `{"id":13,"name":"胖胖","weight":216.5,"dd":"123"}`
	json.Unmarshal([]byte(jsonstr), &v)
	for k, v1 := range v {
		fmt.Print(k, " = ")
		switch v1.(type) {
		case int:
			fmt.Println(v1, "is an int value.")
		case string:
			fmt.Println(v1, "is a string value.")
		case int64:
			fmt.Println(v1, "is an int64 value.")
		case float64:
			fmt.Println(v1, "is an float64 value.")
		default:
			fmt.Println(v1, "is an unknown type.")
		}
	}
}
