// Copyright 2013 The Gorilla WebSocket Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"encoding/json"
	_ "fmt"
	//"log"
)

// Hub maintains the set of active clients and broadcasts messages to the
// clients.
type Hub struct {
	// Registered clients.
	clients map[*Client]bool

	// Inbound messages from the clients.
	broadcast chan []byte

	// Register requests from the clients.
	register chan *Client

	// Unregister requests from clients.
	unregister chan *Client
}

func newHub() *Hub {
	return &Hub{
		broadcast:  make(chan []byte),
		register:   make(chan *Client),
		unregister: make(chan *Client),
		clients:    make(map[*Client]bool),
	}
}

type ui struct {
	UserName  string ``
	UserGUID  string ``
	AvatarURL string `json:"avatarUrl"`
}

type welmsg struct {
	T        string `json:"t"`
	Msg      string `json:"msg"`
	UserInfo ui     `json:"userInfo"`
}

func (h *Hub) run() {
	for {
		select {
		case client := <-h.register:
			h.clients[client] = true
			data := struct {
				T         string `json:"t"`
				Sessionid string `json:"sessionid"`
			}{
				T:         "sessionid",
				Sessionid: client.SessionID,
			}
			msg, _ := json.Marshal(data)
			client.send <- msg
			welmsg0 := welmsg{
				T:   "toall",
				Msg: "银河系第三区交通委提醒您：航道千万条，安全第一条；放飞不规范，亲人两行泪。",
			}
			welmsg0.UserInfo.UserName = "系统小喇叭"

			msg, _ = json.Marshal(welmsg0)
			//log.Println(welmsg0)
			client.send <- msg

		case client := <-h.unregister:
			//log.Println("websocket gone ")
			if _, ok := h.clients[client]; ok {
				delete(h.clients, client)
				close(client.send)
			}

		case message := <-h.broadcast:
			for client := range h.clients {
				select {
				case client.send <- message:
				default:
					close(client.send)
					delete(h.clients, client)
				}
			}
		}
	}
}
