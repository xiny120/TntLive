// Copyright 2013 The Gorilla WebSocket Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
// foobared!@13MA9ERi

package main

type ClientList map[*Client]bool

// Hub maintains the set of active clients and broadcasts messages to the
// clients.
type Hub struct {
	// Registered clients.
	clients map[int]ClientList

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
		clients:    make(map[int]ClientList),
	}
}

func (h *Hub) run() {
	roomid := 0
	for {
		select {
		case client := <-h.register:
			if _, ok := h.clients[roomid]; ok {

			} else {
				clist := make(ClientList)
				h.clients[roomid] = clist
			}
			(h.clients[roomid])[client] = true
		case client := <-h.unregister:
			if _, ok := (h.clients[roomid])[client]; ok {
				delete(h.clients[roomid], client)
				close(client.send)
			}
		case message := <-h.broadcast:

			for client := range h.clients[roomid] {
				select {
				case client.send <- message:
				default:
					close(client.send)
					delete(h.clients[roomid], client)
				}
			}
		}
	}
}
