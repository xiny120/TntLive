package sign

type UserInfo struct {
	UserName  string
	UserId    int64
	UserUuid  string
	Token     string
	Info      string
	SessionId string
	AvatarUrl string `json:"avatarUrl"`
}
