package webapi100

import (
	"log"
	"net/http"

	"github.com/gorilla/mux"
)

func Oauth(w http.ResponseWriter, r *http.Request) {
	w.Write([]byte("Gorilla! oauth\n"))
}

// Define our struct
type AuthenticationMiddleware struct {
	tokenUsers map[string]string
}

// Initialize it somewhere
func (amw *AuthenticationMiddleware) Populate() {
	amw.tokenUsers = make(map[string]string)
	amw.tokenUsers["00000000"] = "user0"
	amw.tokenUsers["aaaaaaaa"] = "userA"
	amw.tokenUsers["05f717e5"] = "randomUser"
	amw.tokenUsers["deadbeef"] = "user0"
}

// 中间件。进行统一的权限认证校验。
func (amw *AuthenticationMiddleware) Middleware(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		vars := mux.Vars(r)
		if vars["action"] == "auth" {
			next.ServeHTTP(w, r)
		} else {
			token := r.Header.Get("X-Session-Token")

			//log.Printf("Authenticated Middleware %s - %s\n", r.URL, vars["action"])

			if user, found := amw.tokenUsers[token]; found {
				// We found the token in our map
				log.Printf("Authenticated user %s\n", user)
				// Pass down the request to the next middleware (or final handler)
				next.ServeHTTP(w, r)
			} else {
				// Write an error and stop the handler chain
				http.Error(w, "Forbidden", http.StatusForbidden)
			}
		}
	})
}
