package webapi100

import (
	"bufio"
	"cfg"
	"encoding/json"
	"fmt"
	"io"

	//	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"
)

var (
	lasttime     time.Time
	access_token string
	expires_in   int
	ticket       string
)

func GetAccessToken() string {
	tfile := "./ws_js/" + "access_tokens"
	if access_token == "" {

		fi, err0 := os.Open(tfile)
		if err0 == nil {
			defer fi.Close()
			br := bufio.NewReader(fi)
			var conf []string
			for {
				a, _, c := br.ReadLine()
				if c == io.EOF {
					break
				}
				conf = append(conf, string(a))
			}
			if len(conf) == 3 {
				lasttime, _ = time.Parse("2006-01-02 15:04:05", conf[0])
				access_token = conf[1]
				expires_in, _ = strconv.Atoi(conf[2])
			}
		}
	}

	tnow := time.Now()

	if tnow.Sub(lasttime).Seconds() >= 7200 {

		tokenUrl := fmt.Sprintf("https://api.weixin.qq.com/cgi-bin/token?grant_type=%s&appid=%s&secret=%s",
			"client_credential", cfg.Cfg["appid"], cfg.Cfg["secret"])
		log.Println(tokenUrl)
		resp, err := http.Post(tokenUrl, "", strings.NewReader(""))
		if err == nil {
			defer resp.Body.Close()
			var v1 interface{}
			err := json.NewDecoder(resp.Body).Decode(&v1)
			if err == nil {
				v := v1.(map[string]interface{})
				if val, found := v["access_token"]; found {
					access_token = val.(string)
					lasttime = time.Now()
				}
				if val, found := v["expires_in"]; found {
					expires_in = int(val.(float64))
				}
				fi, err0 := os.Create(tfile)
				if err0 == nil {
					defer fi.Close()
					bw := bufio.NewWriter(fi)
					bw.WriteString(lasttime.Format("2006-01-02 15:04:05") + "\n")
					bw.WriteString(access_token + "\n")
					bw.WriteString(strconv.Itoa(expires_in) + "\n")
					bw.Flush()
				}
			}
		}
	}
	return access_token
}

func GetWeiXinJsapi_Ticket() string {
	tokenUrl := fmt.Sprintf("https://api.weixin.qq.com/cgi-bin/ticket/getticket?access_token=%s&type=%s",
		access_token, "jsapi")
	log.Println(tokenUrl)
	resp, err := http.Post(tokenUrl, "", strings.NewReader(""))
	if err == nil {
		defer resp.Body.Close()
		var v1 interface{}
		err := json.NewDecoder(resp.Body).Decode(&v1)
		if err == nil {
			log.Println(v1)
			v := v1.(map[string]interface{})
			if val, found := v["ticket"]; found {
				ticket = val.(string)
			}

		}
	}
	return ticket
}

func CreatenNonce_str() string {
	return "43tl899ujljdfafds"
}

func CreatenTimestamp() int {
	return time.Now().Nanosecond()
}
