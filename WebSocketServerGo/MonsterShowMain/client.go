// Copyright 2013 The Gorilla WebSocket Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"bufio"
	_ "bytes"
	"cfg"
	"fmt"
	"io"
	"os"

	//"database/sql"
	"encoding/json"
	"log"
	"net/http"
	"strconv"
	"time"
	sign "ucenter"

	uuid "github.com/satori/go.uuid"

	"github.com/gorilla/mux"
	"github.com/gorilla/websocket"
	_ "github.com/mattn/go-adodb"
)

const (
	// Time allowed to write a message to the peer.
	writeWait = 10 * time.Second

	// Time allowed to read the next pong message from the peer.
	pongWait = 60 * time.Second

	// Send pings to peer with this period. Must be less than pongWait.
	pingPeriod = (pongWait * 9) / 10

	// Maximum message size allowed from peer.
	maxMessageSize = 512
)

var (
	newline = []byte{'\n'}
	space   = []byte{' '}
	hubs    [999]*Hub
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
	// 新增如下代码,解决跨域问题,即403错误
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

// Client is a middleman between the websocket connection and the hub.
type Client struct {
	hub       *Hub
	conn      *websocket.Conn
	send      chan []byte
	SessionID string //未登录之前，记录socket连接时生成的uuid,如果客户端免登录进来的，客户端上报后置换为客户端保存的uuid
}

func (c *Client) readPump() {
	defer func() {
		if info := recover(); info != nil {
			log.Println("client ", c.SessionID, "触发了宕机,终止读任务", info)
		} else {
			//log.Println("client ", c.SessionId, "读任务完成,终止读任务")
		}
		c.hub.unregister <- c
		c.conn.Close()
	}()
	c.conn.SetReadLimit(maxMessageSize)
	c.conn.SetReadDeadline(time.Now().Add(pongWait))
	c.conn.SetPongHandler(func(string) error { c.conn.SetReadDeadline(time.Now().Add(pongWait)); return nil })
	for {
		_, message, err := c.conn.ReadMessage()

		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				//log.Println("用户：", c.SessionId, "error:", err)
			}
			break
		}
		//if mt == 1 {
		m := make(map[string]interface{})
		r := make(map[string]interface{})
		json.Unmarshal(message, &m)

		switch m["t"] {
		case "sign up": // 注册
			account := m["account"].(string)
			password := m["password"].(string)
			email := m["email"].(string)
			cellphone := m["cellphone"].(string)
			r["t"] = "sign up"
			r["status"] = 1
			r["info"] = (account + password + email + cellphone)

			rmsg, err := json.Marshal(r)
			if err == nil {
				c.send <- rmsg
			}

		case "sign in": // 登录
			account := m["account"].(string)
			password := m["password"].(string)
			tt, _ := sign.In(account, password)
			tt.SessionID = c.SessionID
			sign.SessionsSet(c.SessionID, tt)
			r["t"] = "sign in"
			r["userinfo"] = tt
			rmsg, err := json.Marshal(r)
			if err == nil {
				c.send <- rmsg
			}
			log.Println("用户：", account, tt.UserID)

		case "sign out": // 登出
		case "checkin": //免密登录
			r["t"] = "checkin"
			r["status"] = 1
			r["msg"] = "用户密钥过期，请重新登录！"
			//un := ""
			if f, ok := sign.SessionsGet(m["sessionid"].(string)); ok {
				v1 := m["userinfo"].(map[string]interface{})
				v2 := v1["Token"].(string)
				//un = f.UserName

				if f.Token == v2 {
					r["status"] = 0
					r["msg"] = "免密登录成功！"
				}
			}
			rmsg, err := json.Marshal(r)
			if err == nil {
				c.send <- rmsg

			}
			if r["status"] != 0 {
				c.hub.unregister <- c //c.conn.WriteMessage(websocket.CloseMessage, []byte{})
			}
			//log.Println("用户：", un, m["sessionid"], "checkin 免密登录", r["status"], r["msg"])
		case "toall":
			c.hub.broadcast <- message

		}
		//}
	}
}

func (c *Client) writePump() {
	ticker := time.NewTicker(pingPeriod)
	defer func() {
		if info := recover(); info != nil {
			log.Println("client ", c.SessionID, "写任务触发宕机，终止执行", info)
		} else {
			//log.Println("client ", c.SessionId, "写任务成功完成，终止执行")
		}
		ticker.Stop()
		c.conn.Close()
	}()
	for {
		select {
		case message, ok := <-c.send:
			c.conn.SetWriteDeadline(time.Now().Add(writeWait))
			if !ok {
				c.conn.WriteMessage(websocket.CloseMessage, []byte{})
				//log.Println("用户：", c.SessionId, "关闭连接..")
				return
			}

			w, err := c.conn.NextWriter(websocket.TextMessage)
			if err != nil {
				log.Println("用户：", c.SessionID, "NextWriter create 失败..", err)
				return
			}
			w.Write(message)
			//log.Println("writePump", string(message))

			n := len(c.send)
			for i := 0; i < n; i++ {
				w.Write(newline)
				w.Write(<-c.send)
			}

			if err := w.Close(); err != nil {
				log.Println("用户：", c.SessionID, "NextWriter close 失败..", err)
				return
			}
		case <-ticker.C:
			c.conn.SetWriteDeadline(time.Now().Add(writeWait))
			if err := c.conn.WriteMessage(websocket.PingMessage, nil); err != nil {
				log.Println("用户：", c.SessionID, "websocket.PingMessage 失败..", err)
				return
			}
		}
	}
}

// websocket连接请求处理handlefunc
func serveWs(hub *Hub, w http.ResponseWriter, r *http.Request, conn *websocket.Conn) {
	ok, _ := uuid.NewV4()
	client := &Client{hub: hub, conn: conn, send: make(chan []byte, 256), SessionID: ok.String()}
	//log.Println("newClient SessionId", client.SessionId)
	client.hub.register <- client
	// 开启client的数据读写任务。
	go client.writePump()
	go client.readPump()

}

func serveWebSocket(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println(err)
	} else {
		vars := mux.Vars(r)
		id, err := strconv.Atoi(vars["id"])
		if err == nil {
			if id > 0 && id < 1000 {
				if hubs[id] != nil {
					//log.Println("serveTest - ", r.URL, " - ", vars["id"], id, hubs[id])
					serveWs(hubs[id], w, r, conn)
				}
			}
		} else {
			http.Error(w, "NotFound", http.StatusNotFound)
		}
	}
}

func initHubs() {
	// 初始化hubs,每个hubs代表一个websocket聊天室，每个go main支持 999个聊天室。

	for i := 0; i < len(hubs); i++ {
		hubs[i] = newHub()
		go hubs[i].run()
	}
}

func initDB() {
	fi, err0 := os.Open("conf/mssql.dat")
	if err0 != nil {
		fmt.Printf("Error: %s\n", err0)
		return
	}
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
	//fmt.Println(conf)
	var isdebug = false

	//连接字符串
	//connString := fmt.Sprintf("server=%s;port%d;database=%s;user id=%s;password=%s", server, port, database, user, password)
	connString := fmt.Sprintf("Provider=SQLOLEDB;Data Source=%s;Initial Catalog=%s;user id=%s;password=%s", conf[0], conf[2], conf[3], conf[4])

	if isdebug {
		fmt.Println(connString)
	}

	cfg.Cfg["tidb"] = "pic98:" + conf[5] + "@tcp(106.14.145.51:4000)/Pic98"
	cfg.Cfg["mssql"] = connString
	cfg.Cfg["wx_appid"] = conf[6]
	cfg.Cfg["wx_secret"] = conf[7]
}
