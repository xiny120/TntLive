package main

type structabc struct {
	name string
	val  string
}

func structabcf(a structabc) {
	a.name = "abc"
	a.val = "def"
}

func structabcf1(a *structabc) {
	a.name = "abc1"
	a.val = "def1"
}

func main() {
	a := structabc{"a", "b"}
	structabcf(a)
	structabcf1(&a)

}
