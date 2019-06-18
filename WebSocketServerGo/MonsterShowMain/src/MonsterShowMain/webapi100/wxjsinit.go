package webapi100

import (
	"MonsterShowMain/cfg"
	"bufio"
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
	lasttime    time.Time
	accessToken string
	expiresIn   int
	ticket      string
)

// GetAccessToken ok
func GetAccessToken() string {
	tfile := "./ws_js/" + "access_tokens"
	if accessToken == "" {

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
				accessToken = conf[1]
				expiresIn, _ = strconv.Atoi(conf[2])
			}
		}
	}

	tnow := time.Now()

	if tnow.Sub(lasttime).Seconds() >= 7200 {

		tokenURL := fmt.Sprintf("https://api.weixin.qq.com/cgi-bin/token?grant_type=%s&appid=%s&secret=%s",
			"client_credential", cfg.Cfg["wx_appid"], cfg.Cfg["wx_secret"])
		log.Println(tokenURL)
		resp, err := http.Post(tokenURL, "", strings.NewReader(""))
		if err == nil {
			defer resp.Body.Close()
			var v1 interface{}
			err := json.NewDecoder(resp.Body).Decode(&v1)
			if err == nil {
				v := v1.(map[string]interface{})
				if val, found := v["access_token"]; found {
					accessToken = val.(string)
					lasttime = time.Now()
				}
				if val, found := v["expires_in"]; found {
					expiresIn = int(val.(float64))
				}
				fi, err0 := os.Create(tfile)
				if err0 == nil {
					defer fi.Close()
					bw := bufio.NewWriter(fi)
					bw.WriteString(lasttime.Format("2006-01-02 15:04:05") + "\n")
					bw.WriteString(accessToken + "\n")
					bw.WriteString(strconv.Itoa(expiresIn) + "\n")
					bw.Flush()
				}
			}
		}
	}
	return accessToken
}

// GetWeiXinJsapiTicket ok
func GetWeiXinJsapiTicket() string {
	tokenURL := fmt.Sprintf("https://api.weixin.qq.com/cgi-bin/ticket/getticket?access_token=%s&type=%s",
		accessToken, "jsapi")
	log.Println(tokenURL)
	resp, err := http.Post(tokenURL, "", strings.NewReader(""))
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

// CreatenNonceStr ok
func CreatenNonceStr() string {
	return "43tl899ujljdfafds"
}

// CreatenTimestamp ok
func CreatenTimestamp() int {
	return time.Now().Nanosecond()
}
