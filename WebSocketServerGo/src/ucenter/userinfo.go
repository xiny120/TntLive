package sign

// UserInfo  ok
type UserInfo struct {
	UserName  string
	UserID    int64
	UserGUID  string
	Token     string
	Info      string
	SessionID string
	AvatarURL string `json:"avatarUrl"`
}
