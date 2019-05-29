package toolset

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
		ret = int(val.(float64))
	}
	return ret
}
